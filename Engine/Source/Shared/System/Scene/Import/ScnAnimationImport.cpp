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

#include "Import/Mdl/Mdl.h"

#include "assimp/config.h"
#include "assimp/cimport.h"
#include "assimp/anim.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

#define ENABLE_ASSIMP_IMPORTER			( 0 )

#ifdef PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationImport::ScnAnimationImport()
{
}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnAnimationImport::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	Source_ = Object[ "source" ].asString();

#if ENABLE_ASSIMP_IMPORTER
	auto PropertyStore = aiCreatePropertyStore();

	Scene_ = aiImportFileExWithProperties( 
		Source_.c_str(), 
		0,
		nullptr, 
		PropertyStore );

	aiReleasePropertyStore( PropertyStore );

	if( Scene_ != nullptr )
	{
		// Calculate stuffs.
		// Temp code at the moment to test stuff.
		for( BcU32 AnimationIdx = 0; AnimationIdx < Scene_->mNumAnimations; ++AnimationIdx )
		{
			auto* Animation = Scene_->mAnimations[ AnimationIdx ];
			for( BcU32 ChannelIdx = 0; ChannelIdx < Animation->mNumChannels; ++ChannelIdx )
			{
				auto* Channel = Animation->mChannels[ ChannelIdx ];

				BcF32 Rate = 0.5f;
				for( BcF32 Time = 0.0f; Time <= Animation->mDuration; Time += Rate )
				{
					aiVectorKey OutKey;
					if( getPositionKeyNodeAnim( Channel, Time, BcTrue, OutKey ) )
					{
						int a = 0; ++a;
					}
				}
			}
		}

		aiReleaseImport( Scene_ );
		Scene_ = nullptr;

		// Setup data streams.
		ScnAnimationHeader Header;
		Header.NoofNodes_ = 1;
		Header.NoofPoses_ = 1; // means we always have the first frame as the last for smooth looping. TODO: Optional.
		Header.Flags_ = scnAF_DEFAULT;
		Header.Packing_ = scnAP_R16S16T16; // TODO: Make this configurable when we factor out into another class.
		HeaderStream_ << Header;
		Importer.addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize(), 16, csPCF_IN_PLACE );

		//
		return BcTrue;
	}
#endif // ENABLE_ASSIMP_IMPORTER

	// Fall back to old method.
	const std::string& FileName = Source_;
	MdlAnim* pAnim = MdlLoader::loadAnim( FileName.c_str() );

	if( pAnim != NULL )
	{
		// Verify all nodes have the same key count.
		BcU32 KeyCount = pAnim->pNode( 0 )->KeyList_.size();
		for( BcU32 NodeIdx = 1; NodeIdx < pAnim->nNodes(); ++NodeIdx )
		{
			MdlAnimNode* pNode = pAnim->pNode( NodeIdx );
			if( pNode->KeyList_.size() != KeyCount )
			{
				BcAssertMsg( BcFalse, "Invalid key frame count in animation node!" );
				return BcFalse;
			}
		}

		// Setup data streams.
		ScnAnimationHeader Header;
		Header.NoofNodes_ = pAnim->nNodes();
		Header.NoofPoses_ = KeyCount + 1; // means we always have the first frame as the last for smooth looping. TODO: Optional.
		Header.Flags_ = scnAF_DEFAULT;
		Header.Packing_ = scnAP_R16S16T16; // TODO: Make this configurable when we factor out into another class.
		HeaderStream_ << Header;

		// Animation node file data.
		ScnAnimationNodeFileData NodeFileData;
		for( BcU32 NodeIdx = 0; NodeIdx < pAnim->nNodes(); ++NodeIdx )
		{
			MdlAnimNode* pNode = pAnim->pNode( NodeIdx );
			NodeFileData.Name_ = Importer.addString( pNode->Name_ );
			NodeStream_ << NodeFileData;
		}

		// Build up frames of poses.
		ScnAnimationTransformKey_R16S16T16 OutKey;
		BcF32 FrameTime = 0.0f;
		BcF32 FrameRate = 1.0f / 24.0f;
		for( BcU32 KeyIdx = 0; KeyIdx < Header.NoofPoses_; ++KeyIdx )
		{
			BcU32 WrappedKeyIdx = KeyIdx % KeyCount;
			ScnAnimationPoseFileData Pose;
			Pose.Time_ = FrameTime;
			Pose.KeyDataOffset_ = KeyStream_.dataSize();

			// Advance framerate.
			FrameTime += FrameRate;

			// Iterate over nodes, and pack the key stream.
			ScnAnimationTransform Transform;
			for( BcU32 NodeIdx = 0; NodeIdx < pAnim->nNodes(); ++NodeIdx )
			{
				MdlAnimNode* pNode = pAnim->pNode( NodeIdx );
				MdlAnimKey InKey = pNode->KeyList_[ WrappedKeyIdx ];
				Transform.fromMatrix( InKey.Matrix_ );
				OutKey.pack( Transform.R_, Transform.S_, Transform.T_ );
				KeyStream_ << OutKey;
			}
			
			// Final size + CRC.
			Pose.KeyDataSize_ = KeyStream_.dataSize() - Pose.KeyDataOffset_;
			Pose.CRC_ = BcHash::GenerateCRC32( 0, KeyStream_.pData() + Pose.KeyDataOffset_, Pose.KeyDataSize_ );

			// Write out pose.
			PoseStream_ << Pose;
		}
		
		// Write out chunks.
		Importer.addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize(), 16, csPCF_IN_PLACE );
		Importer.addChunk( BcHash( "nodes" ), NodeStream_.pData(), NodeStream_.dataSize() );
		Importer.addChunk( BcHash( "poses" ), PoseStream_.pData(), PoseStream_.dataSize() );
		Importer.addChunk( BcHash( "keys" ), KeyStream_.pData(), KeyStream_.dataSize() );

		return BcTrue;
	}

	return BcFalse;	
}

//////////////////////////////////////////////////////////////////////////
// getPositionKeyNodeAnim
BcBool ScnAnimationImport::getPositionKeyNodeAnim( 
	struct aiNodeAnim* NodeAnim,
	BcF32 Time,
	BcBool Interpolate,
	struct aiVectorKey& Key )
{
	// Setup indices.
	BcU32 MinIdx = 0;
	BcU32 MidIdx = 0;
	BcU32 MaxIdx = NodeAnim->mNumPositionKeys - ( Interpolate ? 2 : 1 );
	
	// Binary search.
	BcBool KeepSearching = BcTrue;
	do
	{
		MidIdx = ( MinIdx + MaxIdx ) / 2;
		const aiVectorKey* KeyA = &NodeAnim->mPositionKeys[ MidIdx ];
		const aiVectorKey* KeyB = &NodeAnim->mPositionKeys[ MidIdx + 1 ];

		if( Time >= KeyA->mTime )
		{
			if( Time <= KeyB->mTime )
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
	if( Interpolate )
	{
		const aiVectorKey* KeyA = &NodeAnim->mPositionKeys[ MidIdx ];
		const aiVectorKey* KeyB = &NodeAnim->mPositionKeys[ MidIdx + 1 ];
		BcAssert( KeyA->mTime < KeyB->mTime );
		BcF32 StepSize = static_cast< BcF32 >( KeyB->mTime ) - static_cast< BcF32 >( KeyA->mTime );
		BcF32 LerpAmount = ( Time - static_cast< BcF32 >( KeyA->mTime ) ) / StepSize;
		Key.mTime = Time;
		Key.mValue = KeyA->mValue + ( ( KeyB->mValue - KeyA->mValue ) * LerpAmount );
	}
	else
	{
		const aiVectorKey* KeyA = &NodeAnim->mPositionKeys[ MidIdx ];
		Key = *KeyA;
	}
	
	return BcTrue;
}

#endif
