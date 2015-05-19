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
		new ReField( "Children_", &ScnAnimationTreeBlendNode::Children_, bcRFF_IMPORTER ),
		new ReField( "BlendType_", &ScnAnimationTreeBlendNode::BlendType_, bcRFF_IMPORTER ),
		new ReField( "BlendValue_", &ScnAnimationTreeBlendNode::BlendValue_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnAnimationTreeBlendNode, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeBlendNode::ScnAnimationTreeBlendNode()
{
	Children_[ 0 ] = nullptr;
	Children_[ 1 ] = nullptr;
	BlendValue_ = 0.0f;
	BlendType_ = scnATBT_LERP;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationTreeBlendNode::~ScnAnimationTreeBlendNode()
{
	delete Children_[ 0 ];
	delete Children_[ 1 ];
	Children_[ 0 ] = nullptr;
	Children_[ 1 ] = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// setChildNode
//virtual
void ScnAnimationTreeBlendNode::initialise( 
	ScnAnimationPose* pReferencePose,
	ScnAnimationNodeFileData* pNodeFileData )
{
	ScnAnimationTreeNode::initialise( pReferencePose, pNodeFileData );
}

//////////////////////////////////////////////////////////////////////////
// setChildNode
//virtual
void ScnAnimationTreeBlendNode::setChildNode( BcU32 Idx, ScnAnimationTreeNode* pNode )
{
	BcAssert( Idx < 2 );
	Children_[ Idx ] = pNode;
}

//////////////////////////////////////////////////////////////////////////
// getChildNode
//virtual
ScnAnimationTreeNode* ScnAnimationTreeBlendNode::getChildNode( BcU32 Idx )
{
	BcAssert( Idx < 2 );
	return Children_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getNoofChildNodes
//virtual
BcU32 ScnAnimationTreeBlendNode::getNoofChildNodes() const
{
	return 2;
}

//////////////////////////////////////////////////////////////////////////
// decode
//virtual
void ScnAnimationTreeBlendNode::decode()
{
	Children_[ 0 ]->decode();
	Children_[ 1 ]->decode();
}

//////////////////////////////////////////////////////////////////////////
// pose
//virtual
void ScnAnimationTreeBlendNode::pose()
{
	Children_[ 0 ]->pose();
	Children_[ 1 ]->pose();

	// Only blend if we need to do any work.
	switch( BlendType_ )
	{
	case scnATBT_LERP:
		if( BlendValue_ > 0.0f && BlendValue_ < 1.0f )
		{
			pWorkingPose_->blend( Children_[ 0 ]->getWorkingPose(), Children_[ 1 ]->getWorkingPose(), BlendValue_ );
		}
		break;
	case scnATBT_ADD:
		pWorkingPose_->add( *pReferencePose_, Children_[ 0 ]->getWorkingPose(), Children_[ 1 ]->getWorkingPose(), BlendValue_ );
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// advance
//virtual
void ScnAnimationTreeBlendNode::advance( BcF32 Tick )
{
	Children_[ 0 ]->advance( Tick );
	Children_[ 1 ]->advance( Tick );
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
			return Children_[ 0 ]->getWorkingPose();
		}
		else
		{
			return Children_[ 1 ]->getWorkingPose();
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
