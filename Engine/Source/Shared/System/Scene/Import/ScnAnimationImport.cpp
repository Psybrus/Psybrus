/**************************************************************************
*
* File:		ScnAnimationImport.cpp
* Author:	Neil Richardson
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnAnimationImport.h"

#include "Base/BcMath.h"

#include <memory>

#if PSY_IMPORT_PIPELINE

#include "assimp/config.h"
#include "assimp/cimport.h"
#include "assimp/anim.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

namespace
{
	/**
	 * Assimp logging function.
	 */
	void AssimpLogStream( const char* Message, char* User )
	{
		BcPrintf( "ASSIMP: %s", Message );
	}

	/**
	 * Get key node from an array of node key anims.
	 */
	template< typename _KEY_TYPE, typename _VALUE_TYPE >
	BcBool GetKeyNodeAnim( 
		_KEY_TYPE* NodeKeys,
		BcU32 NumNodeKeys,
		BcF32 Time,
		BcBool Interpolate,
		_VALUE_TYPE& Key )
	{
		// Setup indices.
		BcU32 MinIdx = 0;
		BcU32 MidIdx = 0;
		BcU32 MaxIdx = BcClamp( NumNodeKeys - ( Interpolate ? 2 : 1 ), 0, NumNodeKeys - 1 );
	
		// Binary search.
		BcBool KeepSearching = BcTrue;
		do
		{
			MidIdx = ( MinIdx + MaxIdx ) / 2;
			const _KEY_TYPE& KeyA = NodeKeys[ MidIdx ];
			const _KEY_TYPE& KeyB = NodeKeys[ BcMin( MidIdx + 1, NumNodeKeys - 1 ) ];

			if( Time >= KeyA.mTime )
			{
				if( Time <= KeyB.mTime )
				{
					KeepSearching = BcFalse;
				}
				else
				{
					KeepSearching = MinIdx != MidIdx;
					MinIdx = MidIdx;
				}
			}
			else
			{
				KeepSearching = MaxIdx != MidIdx;
				MaxIdx = MidIdx;
			}
		}
		while( KeepSearching );

		// Should have a valid key by this point, we need to calculate the output key now.
		if( Interpolate && ( MidIdx + 1 ) < NumNodeKeys )
		{
			const _KEY_TYPE& KeyA = NodeKeys[ MidIdx ];
			const _KEY_TYPE& KeyB = NodeKeys[ MidIdx + 1 ];
			BcAssert( KeyA.mTime <= KeyB.mTime );
			BcF32 StepSize = static_cast< BcF32 >( KeyB.mTime ) - static_cast< BcF32 >( KeyA.mTime );
			BcF32 LerpAmount = ( Time - static_cast< BcF32 >( KeyA.mTime ) ) / StepSize;
			
			Assimp::Interpolator< _KEY_TYPE >()( Key, KeyA, KeyB, LerpAmount );
		}
		else
		{
			const _KEY_TYPE& KeyA = NodeKeys[ MidIdx ];
			Key = KeyA.mValue;
		}
	
		return BcTrue;
	}
}

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnAnimationImport )
	
void ScnAnimationImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnAnimationImport::Source_, bcRFF_IMPORTER ),
		new ReField( "FrameRate_", &ScnAnimationImport::FrameRate_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnAnimationImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationImport::ScnAnimationImport():
	Source_(),
	FrameRate_( 24.0f ),
	HeaderStream_(),
	NodeStream_(),
	PoseStream_(),
	KeyStream_(),
	Scene_( nullptr ),
	AnimatedNodes_()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationImport::ScnAnimationImport( ReNoInit ):
	Source_(),
	FrameRate_( 24.0f ),
	HeaderStream_(),
	NodeStream_(),
	PoseStream_(),
	KeyStream_(),
	Scene_( nullptr ),
	AnimatedNodes_()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationImport::~ScnAnimationImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnAnimationImport::import( const Json::Value& )
{
#if PSY_IMPORT_PIPELINE
	if( Source_.empty() )
	{
		BcPrintf( "ERROR: Missing 'source' field.\n" );
		return BcFalse;
	}

	CsResourceImporter::addDependency( Source_.c_str() );

	auto PropertyStore = aiCreatePropertyStore();

	aiLogStream AssimpLogger =
	{
		AssimpLogStream, (char*)this
	};
	aiAttachLogStream( &AssimpLogger );

	Scene_ = aiImportFileExWithProperties( 
		Source_.c_str(), 
		0,
		nullptr, 
		PropertyStore );

	aiReleasePropertyStore( PropertyStore );

	if( Scene_ != nullptr )
	{
		// Build animated nodes list. Need this to calculate relative transforms later.
		recursiveParseAnimatedNodes( Scene_->mRootNode, BcErrorCode );

		// Pack down animation into useful internal format.
		BcAssert( Scene_->mNumAnimations == 1 );
		for( BcU32 AnimationIdx = 0; AnimationIdx < 1; ++AnimationIdx )
		{
			auto* Animation = Scene_->mAnimations[ AnimationIdx ];

			BcF32 Rate = 1.0f;
			BcU32 Duration = static_cast< BcU32 >( Animation->mDuration / Rate );

			// Setup data streams.
			ScnAnimationHeader Header;
			Header.NoofNodes_ = Animation->mNumChannels;
			Header.NoofPoses_ = Duration;
			Header.Flags_ = scnAF_DEFAULT;
			Header.Packing_ = scnAP_R16S16T16; // TODO: Make this configurable when we factor out into another class.
			HeaderStream_ << Header;

			// Animation node file data.
			ScnAnimationNodeFileData NodeFileData;
			for( BcU32 NodeIdx = 0; NodeIdx < Animation->mNumChannels; ++NodeIdx )
			{
				auto* Channel = Animation->mChannels[ NodeIdx ];
				NodeFileData.Name_ = CsResourceImporter::addString( Channel->mNodeName.C_Str() );
				NodeStream_ << NodeFileData;
			}

			// Calculate output pose.
			for( BcF32 Time = 0.0f; Time <= Animation->mDuration; Time += Rate )
			{
				ScnAnimationPoseFileData Pose;
				Pose.Time_ = Time / FrameRate_;
				Pose.KeyDataOffset_ = KeyStream_.dataSize();

				// Iterate over all node channels to generate keys.
				for( BcU32 ChannelIdx = 0; ChannelIdx < Animation->mNumChannels; ++ChannelIdx )
				{
					auto* Channel = Animation->mChannels[ ChannelIdx ];
					auto& AnimatedNode = findAnimatedNode( Channel->mNodeName.C_Str() );

					aiVector3D OutPositionKey;
					aiVector3D OutScaleKey;
					aiQuaternion OutRotationKey;

					// Extract position.
					GetKeyNodeAnim( 
						Channel->mPositionKeys, 
						Channel->mNumPositionKeys, 
						Time, 
						BcTrue, 
						OutPositionKey );
	
					// Extract scale.
					GetKeyNodeAnim( 
						Channel->mScalingKeys, 
						Channel->mNumScalingKeys, 
						Time, 
						BcTrue, 
						OutScaleKey );

					// Extract rotation.
					GetKeyNodeAnim( 
						Channel->mRotationKeys, 
						Channel->mNumRotationKeys, 
						Time, 
						BcTrue, 
						OutRotationKey );

					// Combine key into transform.
					ScnAnimationTransform Transform;
					Transform.R_ = MaQuat( OutRotationKey.x, OutRotationKey.y, OutRotationKey.z, OutRotationKey.w );
					Transform.S_ = MaVec3d( OutScaleKey.x, OutScaleKey.y, OutScaleKey.z );
					Transform.T_ = MaVec3d( OutPositionKey.x, OutPositionKey.y, OutPositionKey.z );
				
					// Store as local matrix.
					Transform.toMatrix( AnimatedNode.LocalTransform_ );
				}

				// Calculate local node matrices relative to their parents.
				for( auto& AnimatedNode : AnimatedNodes_ )
				{
					if( AnimatedNode.ParentIdx_ != BcErrorCode )
					{
						auto& ParentAnimatedNode( AnimatedNodes_[ AnimatedNode.ParentIdx_ ] );
						MaMat4d ParentLocal = ParentAnimatedNode.LocalTransform_;
						AnimatedNode.WorldTransform_ = ParentLocal * AnimatedNode.LocalTransform_;
					}
					else
					{
						AnimatedNode.WorldTransform_ = AnimatedNode.LocalTransform_;
					}
				}

				// Write out pose keys.
				ScnAnimationTransformKey_R16S16T16 OutKey;
				for( BcU32 ChannelIdx = 0; ChannelIdx < Animation->mNumChannels; ++ChannelIdx )
				{
					auto* Channel = Animation->mChannels[ ChannelIdx ];
					const auto& AnimatedNode = findAnimatedNode( Channel->mNodeName.C_Str() );

					// Extract individual transform elements.
					ScnAnimationTransform Transform;
					Transform.fromMatrix( AnimatedNode.LocalTransform_ );

					// Pack into output key.
					OutKey.pack( 
						Transform.R_,
						Transform.S_,
						Transform.T_ );

					KeyStream_ << OutKey;
				}
			
				// Final size + CRC.
				Pose.KeyDataSize_ = KeyStream_.dataSize() - Pose.KeyDataOffset_;
				Pose.CRC_ = BcHash::GenerateCRC32( 0, KeyStream_.pData() + Pose.KeyDataOffset_, Pose.KeyDataSize_ );

				// Write out pose.
				PoseStream_ << Pose;
			}
		
			// Write out chunks.
			CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize(), 16, csPCF_IN_PLACE );
			CsResourceImporter::addChunk( BcHash( "nodes" ), NodeStream_.pData(), NodeStream_.dataSize() );
			CsResourceImporter::addChunk( BcHash( "poses" ), PoseStream_.pData(), PoseStream_.dataSize() );
			CsResourceImporter::addChunk( BcHash( "keys" ), KeyStream_.pData(), KeyStream_.dataSize() );
		}

		aiReleaseImport( Scene_ );
		Scene_ = nullptr;

		//
		return BcTrue;
	}
#endif // PSY_IMPORT_PIPELINE
	return BcFalse;	
}

//////////////////////////////////////////////////////////////////////////
// recursiveParseAnimatedNodes
void ScnAnimationImport::recursiveParseAnimatedNodes( struct aiNode* Node, size_t ParentNodeIdx )
{
#if PSY_IMPORT_PIPELINE
	AnimatedNode AnimatedNode;
	aiMatrix4x4 WorldTransform = Node->mParent != nullptr ?
		Node->mParent->mTransformation * Node->mTransformation : Node->mTransformation;

	AnimatedNode.Name_ = Node->mName.C_Str();
	AnimatedNode.LocalTransform_ = MaMat4d( Node->mTransformation[ 0 ] ).transposed();
	AnimatedNode.WorldTransform_ = MaMat4d( WorldTransform[ 0 ] ).transposed();
	AnimatedNode.ParentIdx_ = static_cast< BcU32 >( ParentNodeIdx );

	AnimatedNodes_.push_back( AnimatedNode );

	for( BcU32 ChildIdx = 0; ChildIdx < Node->mNumChildren; ++ChildIdx )
	{
		recursiveParseAnimatedNodes( Node->mChildren[ ChildIdx ], AnimatedNodes_.size() - 1 );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// findAnimatedNode
ScnAnimationImport::AnimatedNode& ScnAnimationImport::findAnimatedNode( std::string Name )
{
#if PSY_IMPORT_PIPELINE
	for( auto& AnimatedNode : AnimatedNodes_ )
	{
		if( AnimatedNode.Name_ == Name )
		{
			return AnimatedNode;
		}
	}

	// Should never hit here.
	BcBreakpoint;

	return *(ScnAnimationImport::AnimatedNode*)( nullptr );
#endif
}
