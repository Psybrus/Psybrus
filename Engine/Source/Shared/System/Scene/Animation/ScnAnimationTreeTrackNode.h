/**************************************************************************
*
* File:		ScnAnimationTreeTrackNode.h
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATIONTREETRACKNODE_H__
#define __SCNANIMATIONTREETRACKNODE_H__

#include "System/Scene/Animation/ScnAnimationTreeNode.h"
#include "System/Scene/Animation/ScnAnimation.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTreeTrackNode
class ScnAnimationTreeTrackNode:
	public ScnAnimationTreeNode
{
public:
	REFLECTION_DECLARE_DERIVED( ScnAnimationTreeTrackNode, ScnAnimationTreeNode );

public:
	ScnAnimationTreeTrackNode();
	virtual ~ScnAnimationTreeTrackNode();

	virtual void initialise( ScnAnimationPose* pReferencePose );
	virtual void setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode );
	virtual ScnAnimationTreeNode* getChildNode( BcU32 Idx );
	virtual BcU32 getNoofChildNodes() const;
	virtual void preUpdate( BcF32 Tick );
	virtual void update( BcF32 Tick );
	virtual void postUpdate( BcF32 Tick );

	void setSpeed( BcF32 Speed );
	void queueAnimation( ScnAnimation* pAnimation );

private:
	void decodeFrames();
	void interpolatePose();

private:
	typedef std::vector< ScnAnimation* > TAnimationQueue;

	ScnAnimationPose* pPoseA_;
	ScnAnimationPose* pPoseB_;
		
	BcU32 CurrPoseIndex_;

	BcF32 Speed_;
	BcF32 Time_;

	TAnimationQueue AnimationQueue_;
	const ScnAnimationPoseFileData* pPoseFileDataA_;
	const ScnAnimationPoseFileData* pPoseFileDataB_;
};

#endif
