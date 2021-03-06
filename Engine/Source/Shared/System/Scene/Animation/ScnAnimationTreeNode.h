/**************************************************************************
*
* File:		ScnAnimationTreeNode.h
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNANIMATIONTREENODE_H__
#define __SCNANIMATIONTREENODE_H__

#include "System/Scene/Animation/ScnAnimation.h"
#include "System/Scene/Animation/ScnAnimationPose.h"

//////////////////////////////////////////////////////////////////////////
// ScnAnimationTreeNode
class ScnAnimationTreeNode:
	public ReObject
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnAnimationTreeNode, ReObject );

public:
	ScnAnimationTreeNode();
	ScnAnimationTreeNode( ReNoInit );
	virtual ~ScnAnimationTreeNode();
	
public:
	/**
	 * Initialise.
	 */
	virtual void initialise( 
		ScnAnimationPose* pReferencePose, 
		ScnAnimationNodeFileData* pNodeFileData );

	/**
	 * Set child node.
	 */
	virtual void setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode ) = 0;

	/**
	 * Get child node.
	 */
	virtual ScnAnimationTreeNode* getChildNode( BcU32 Idx ) = 0;

	/**
	 * Get number of nodes.
	 */
	virtual BcU32 getNoofChildNodes() const = 0;

	/**
	 * Decode anything that we need to pose.
	 */
	virtual void decode() = 0;
	
	/**
	 * Pose. Depends on decoding step.
	 */
	virtual void pose() = 0;

	/**
	 * Advance animation.
	 */
	virtual void advance( BcF32 Tick ) = 0;

	/**
	 * Get working pose.
	 */
	virtual const ScnAnimationPose& getWorkingPose() const;

protected:
	ScnAnimationNodeFileData* pNodeFileData_;
	ScnAnimationPose* pReferencePose_;
	ScnAnimationPose* pWorkingPose_;
};

#endif
