/**************************************************************************
*
* File:		ScnAnimation.h
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATION_H__
#define __SCNANIMATION_H__

#include "System/Content/CsResource.h"

#include "System/Scene/Animation/ScnAnimationFileData.h"
#include "System/Scene/Animation/ScnAnimationPose.h"

//////////////////////////////////////////////////////////////////////////
// ScnModelRef
typedef ReObjectRef< class ScnAnimation > ScnAnimationRef;

//////////////////////////////////////////////////////////////////////////
// ScnAnimation
class ScnAnimation:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnAnimation, CsResource );

	ScnAnimation();
	virtual ~ScnAnimation();
	
	const ScnAnimationNodeFileData* getNodeByIndex( BcU32 Idx ) const;
	BcU32 getNodeIndexByName( BcName Name ) const;
	BcU32 findPoseIndexAtTime( BcF32 Time ) const ;
	const ScnAnimationPoseFileData* findPoseAtIndex( BcU32 Idx ) const ;
	const BcU8* findKeyDataStartForPose( const ScnAnimationPoseFileData* pPose ) const;	
	BcF32 getLength() const;

	void decodePoseAtIndex( 
		BcU32 Idx, 
		ScnAnimationPose* pOutputPose,
		ScnAnimationNodeFileData* TargetNodesFileData ) const;

private:
	template< typename _Ty >
	void decodePoseAtIndexTyped( 
		BcU32 Idx, 
		ScnAnimationPose* pOutputPose,
		ScnAnimationNodeFileData* TargetNodesFileData ) const;

private:
	void fileReady();
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );

private:
	ScnAnimationHeader Header_;
	ScnAnimationNodeFileData* pNodeData_;
	const ScnAnimationPoseFileData* pPoseData_;
	const BcU8* pKeyData_;
};

#endif
