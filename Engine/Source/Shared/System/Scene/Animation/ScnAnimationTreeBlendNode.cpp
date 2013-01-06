/**************************************************************************
*
* File:		ScnAnimationTreeBlendNode.cpp
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Animation/ScnAnimationTreeBlendNode.h"

//////////////////////////////////////////////////////////////////////////
// Reflection.
BCREFLECTION_DEFINE_DERIVED( ScnAnimationTreeBlendNode );

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeBlendNode::ScnAnimationTreeBlendNode( const BcName& Name ):
	ScnAnimationTreeNode( Name )
{
	pNodes_[ 0 ] = NULL;
	pNodes_[ 1 ] = NULL;
	BlendValue_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationTreeBlendNode::~ScnAnimationTreeBlendNode()
{
	delete pNodes_[ 0 ];
	delete pNodes_[ 1 ];
	pNodes_[ 0 ] = NULL;
	pNodes_[ 1 ] = NULL;
}

//////////////////////////////////////////////////////////////////////////
// setChildNode
//virtual
void ScnAnimationTreeBlendNode::initialise( ScnAnimationPose* pReferencePose )
{
	ScnAnimationTreeNode::initialise( pReferencePose );

	
}

//////////////////////////////////////////////////////////////////////////
// setChildNode
//virtual
void ScnAnimationTreeBlendNode::setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode )
{
	BcAssert( Idx < 2 );
	pNodes_[ Idx ] = pNode;
}

//////////////////////////////////////////////////////////////////////////
// getChildNode
//virtual
ScnAnimationTreeNode* ScnAnimationTreeBlendNode::getChildNode( BcU32 Idx )
{
	BcAssert( Idx < 2 );
	return pNodes_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getNoofChildNodes
//virtual
BcU32 ScnAnimationTreeBlendNode::getNoofChildNodes() const
{
	return 2;
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual
void ScnAnimationTreeBlendNode::preUpdate( BcF32 Tick )
{
	pNodes_[ 0 ]->preUpdate( Tick );
	pNodes_[ 1 ]->preUpdate( Tick );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnAnimationTreeBlendNode::update( BcF32 Tick )
{
	pNodes_[ 0 ]->update( Tick );
	pNodes_[ 1 ]->update( Tick );

	// Blend nodes into out working pose.
	pWorkingPose_->blend( pNodes_[ 0 ]->getWorkingPose(), pNodes_[ 1 ]->getWorkingPose(), BlendValue_ );
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual
void ScnAnimationTreeBlendNode::postUpdate( BcF32 Tick )
{
	pNodes_[ 0 ]->postUpdate( Tick );
	pNodes_[ 1 ]->postUpdate( Tick );
}

//////////////////////////////////////////////////////////////////////////
// setBlendValue
void ScnAnimationTreeBlendNode::setBlendValue( BcF32 Value )
{
	BlendValue_ = Value;
}

//////////////////////////////////////////////////////////////////////////
// getBlendValue
BcF32 ScnAnimationTreeBlendNode::getBlendValue() const
{
	return BlendValue_;
}
