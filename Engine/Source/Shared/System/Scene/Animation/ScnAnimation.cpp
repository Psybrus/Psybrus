/**************************************************************************
*
* File:		ScnAnimation.cpp
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Animation/ScnAnimation.h"

#include "System/Content/CsCore.h"

#ifdef PSY_SERVER
#include "Base/BcStream.h"
#include "Import/Mdl/Mdl.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnAnimation::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	const std::string& FileName = Object[ "source" ].asString();
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
		BcStream HeaderStream;
		BcStream NodeStream;
		BcStream PoseStream;
		BcStream KeyStream;

		ScnAnimationHeader Header;
		Header.NoofNodes_ = pAnim->nNodes();
		Header.NoofPoses_ = KeyCount + 1; // means we always have the first frame as the last for smooth looping. TODO: Optional.
		Header.Flags_ = scnAF_DEFAULT;
		Header.Packing_ = scnAP_R16S16T16; // TODO: Make this configurable when we factor out into another class.
		HeaderStream << Header;

		// Animation node file data.
		ScnAnimationNodeFileData NodeFileData;
		for( BcU32 NodeIdx = 0; NodeIdx < pAnim->nNodes(); ++NodeIdx )
		{
			MdlAnimNode* pNode = pAnim->pNode( NodeIdx );
			NodeFileData.Name_ = Importer.addString( pNode->Name_ );
			NodeStream << NodeFileData;
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
			Pose.KeyDataOffset_ = KeyStream.dataSize();

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
				KeyStream << OutKey;
			}
			
			// Final size + CRC.
			Pose.KeyDataSize_ = KeyStream.dataSize() - Pose.KeyDataOffset_;
			Pose.CRC_ = BcHash::GenerateCRC32( 0, KeyStream.pData() + Pose.KeyDataOffset_, Pose.KeyDataSize_ );

			// Write out pose.
			PoseStream << Pose;
		}
		
		// Write out chunks.
		Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize(), 16, csPCF_IN_PLACE );
		Importer.addChunk( BcHash( "nodes" ), NodeStream.pData(), NodeStream.dataSize() );
		Importer.addChunk( BcHash( "poses" ), PoseStream.pData(), PoseStream.dataSize() );
		Importer.addChunk( BcHash( "keys" ), KeyStream.pData(), KeyStream.dataSize() );

		return BcTrue;
	}

	return BcFalse;	
}

#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnAnimation );

void ScnAnimation::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Header_",			&ScnAnimation::Header_ ),
	};
		
	ReRegisterClass< ScnAnimation, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnAnimation::initialise()
{

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnAnimation::create()
{
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnAnimation::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// getNodeByIndex
const ScnAnimationNodeFileData* ScnAnimation::getNodeByIndex( BcU32 Idx ) const
{
	if( Idx < Header_.NoofNodes_ )
	{
		return &pNodeData_[ Idx ];
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// findPoseIndexAtTime
BcU32 ScnAnimation::findPoseIndexAtTime( BcF32 Time ) const 
{
	// Setup indices.
	BcU32 MinIdx = 0;
	BcU32 MidIdx = 0;
	BcU32 MaxIdx = Header_.NoofPoses_ - 2;

	// Binary search.
	BcBool KeepSearching = BcTrue;
	do
	{
		MidIdx = ( MinIdx + MaxIdx ) / 2;
		const ScnAnimationPoseFileData* pPoseA = &pPoseData_[ MidIdx ];
		const ScnAnimationPoseFileData* pPoseB = &pPoseData_[ MidIdx + 1 ];

		if( Time >= pPoseA->Time_ )
		{
			if( Time <= pPoseB->Time_ )
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

	return MidIdx;
}

//////////////////////////////////////////////////////////////////////////
// findPoseAtIndex
const ScnAnimationPoseFileData*	ScnAnimation::findPoseAtIndex( BcU32 Idx ) const 
{
	if( Idx < Header_.NoofPoses_ )
	{
		return &pPoseData_[ Idx ];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// findKeyDataStartForPose
const BcU8* ScnAnimation::findKeyDataStartForPose( const ScnAnimationPoseFileData* pPose ) const
{
	return &pKeyData_[ pPose->KeyDataOffset_ ];
}

//////////////////////////////////////////////////////////////////////////
// getLength
BcF32 ScnAnimation::getLength() const
{
	const ScnAnimationPoseFileData* pPose = &pPoseData_[ Header_.NoofPoses_ - 1 ];
	return pPose->Time_;
}

//////////////////////////////////////////////////////////////////////////
// decodePoseAtIndexTyped
template< typename _Ty >
void ScnAnimation::decodePoseAtIndexTyped( BcU32 Idx, ScnAnimationPose* pOutputPose ) const
{
	const ScnAnimationPoseFileData* pPoseFileData = findPoseAtIndex( Idx );
	if( pPoseFileData != NULL )
	{
		const BcU8* pKeyData = findKeyDataStartForPose( pPoseFileData );
		const _Ty* pTransformKeys = reinterpret_cast< const _Ty* >( pKeyData );

		ScnAnimationTransform Transform;
		for( BcU32 Idx = 0; Idx < Header_.NoofNodes_; ++Idx )
		{
			pTransformKeys[ Idx ].unpack( Transform.R_, Transform.S_, Transform.T_ );
			pOutputPose->setTransform( Idx, Transform );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// decodePoseAtIndex
void ScnAnimation::decodePoseAtIndex( BcU32 Idx, ScnAnimationPose* pOutputPose ) const
{
	switch( Header_.Packing_ )
	{
	case scnAP_R16S32T32:
		decodePoseAtIndexTyped< ScnAnimationTransformKey_R16S32T32 >( Idx, pOutputPose );
		break;
	case scnAP_R16S16T16:
		decodePoseAtIndexTyped< ScnAnimationTransformKey_R16S16T16 >( Idx, pOutputPose );
		break;
	case scnAP_R16T32:
		decodePoseAtIndexTyped< ScnAnimationTransformKey_R16T32 >( Idx, pOutputPose );
		break;
	case scnAP_R16T16:
		decodePoseAtIndexTyped< ScnAnimationTransformKey_R16T16 >( Idx, pOutputPose );
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnAnimation::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0, &Header_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnAnimation::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{		
		// Grab pose and key chunks.
		requestChunk( 1 );
		requestChunk( 2 );
		requestChunk( 3 );
	}
	else if( ChunkID == BcHash( "nodes" ) )
	{
		pNodeData_ = reinterpret_cast< ScnAnimationNodeFileData* >( pData );

		for( BcU32 Idx = 0; Idx < Header_.NoofNodes_; ++Idx )
		{
			getPackage()->markupName( pNodeData_[ Idx ].Name_ );
		}
	}
	else if( ChunkID == BcHash( "poses" ) )
	{
		pPoseData_ = reinterpret_cast< const ScnAnimationPoseFileData* >( pData ); 
	}
	else if( ChunkID == BcHash( "keys" ) )
	{
		pKeyData_ = reinterpret_cast< const BcU8* >( pData );
		
		markCreate();
	}

}
