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
REFLECTION_DEFINE_DERIVED( ScnAnimationTreeBlendNode );

void ScnAnimationTreeBlendNode::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pNodes_",			&ScnAnimationTreeBlendNode::pNodes_ ),
		new ReField( "BlendType_",		&ScnAnimationTreeBlendNode::BlendType_ ),
		new ReField( "BlendValue_",		&ScnAnimationTreeBlendNode::BlendValue_ ),
	};
		
	ReRegisterClass< ScnAnimationTreeBlendNode, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeBlendNode::ScnAnimationTreeBlendNode()
{
	pNodes_[ 0 ] = NULL;
	pNodes_[ 1 ] = NULL;
	BlendValue_ = 0.0f;
	BlendType_ = scnATBT_LERP;
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

	// Only blend if we need to do any work.
	switch( BlendType_ )
	{
	case scnATBT_LERP:
		if( BlendValue_ > 0.0f && BlendValue_ < 1.0f )
		{
			pWorkingPose_->blend( pNodes_[ 0 ]->getWorkingPose(), pNodes_[ 1 ]->getWorkingPose(), BlendValue_ );
		}
		break;
	case scnATBT_ADD:
		pWorkingPose_->add( *pReferencePose_, pNodes_[ 0 ]->getWorkingPose(), pNodes_[ 1 ]->getWorkingPose(), BlendValue_ );
		break;
	}
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
// getWorkingPose
//virtual
const ScnAnimationPose& ScnAnimationTreeBlendNode::getWorkingPose() const
{
	BcAssert( pWorkingPose_ != NULL );

	switch( BlendType_ )
	{
	case scnATBT_LERP:
		if( BlendValue_ > 0.0f && BlendValue_ < 1.0f )
		{
			return *pWorkingPose_;
		}
		else if( BlendValue_ <= 0.0f )
		{
			return pNodes_[ 0 ]->getWorkingPose();
		}
		else
		{
			return pNodes_[ 1 ]->getWorkingPose();
		}
		break;
	case scnATBT_ADD:
		return *pWorkingPose_;
		break;

	default:
		return *pWorkingPose_;
	}
}

//////////////////////////////////////////////////////////////////////////
// setBlendType
void ScnAnimationTreeBlendNode::setBlendType( ScnAnimationTreeBlendType Type )
{
	BlendType_ = Type;
}

//////////////////////////////////////////////////////////////////////////
// getBlendType
ScnAnimationTreeBlendType ScnAnimationTreeBlendNode::getBlendType() const
{
	return BlendType_;
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
