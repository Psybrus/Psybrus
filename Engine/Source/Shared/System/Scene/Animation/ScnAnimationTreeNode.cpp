/**************************************************************************
*
* File:		ScnAnimationTreeNode.cpp
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Animation/ScnAnimationTreeNode.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeNode::ScnAnimationTreeNode()
{
	pWorkingPose_ = NULL;
	pReferencePose_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationTreeNode::~ScnAnimationTreeNode()
{
	//
	delete pWorkingPose_;
	pWorkingPose_ = NULL;
	pReferencePose_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnAnimationTreeNode::initialise( ScnAnimationPose* pReferencePose )
{
	BcAssert( pReferencePose_ == NULL );
	BcAssert( pWorkingPose_ == NULL );

	pReferencePose_ = pReferencePose;
	pWorkingPose_ = new ScnAnimationPose( *pReferencePose );
}

//////////////////////////////////////////////////////////////////////////
// initialise
const ScnAnimationPose* ScnAnimationTreeNode::getWorkingPose() const
{
	return pWorkingPose_;
}
