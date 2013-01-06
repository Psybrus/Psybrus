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
class ScnAnimationTreeNode
{
public:
	BCREFLECTION_DECLARE_BASE( ScnAnimationTreeNode );

public:
	ScnAnimationTreeNode( const BcName& Name );
	virtual ~ScnAnimationTreeNode();
	
public: // Overrides to implement interface.

	virtual void initialise( ScnAnimationPose* pReferencePose );

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
	 * Pre-update
	 */
	virtual void preUpdate( BcF32 Tick ) = 0;
	
	/**
	 * update
	 */
	virtual void update( BcF32 Tick ) = 0;

	/**
	 * Post-update
	 */
	virtual void postUpdate( BcF32 Tick ) = 0;

	/**
	 * Get working pose.
	 */
	const ScnAnimationPose* getWorkingPose() const;

protected:
	BcName					Name_;
	ScnAnimationPose*		pReferencePose_;
	ScnAnimationPose*		pWorkingPose_;
};

#endif
