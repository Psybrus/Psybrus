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
// Reflection
REFLECTION_DEFINE_BASE( ScnAnimationTreeNode );

void ScnAnimationTreeNode::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pReferencePose_", &ScnAnimationTreeNode::pReferencePose_ ),
		new ReField( "pWorkingPose_", &ScnAnimationTreeNode::pWorkingPose_ ),
	};
		
	ReRegisterAbstractClass< ScnAnimationTreeNode, Super >( Fields );
}


//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeNode::ScnAnimationTreeNode():
	pReferencePose_( nullptr ),
	pWorkingPose_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeNode::ScnAnimationTreeNode( ReNoInit ):
	pReferencePose_( nullptr ),
	pWorkingPose_( nullptr )
{

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

	for( BcU32 Idx = 0; Idx < getNoofChildNodes(); ++Idx )
	{
		getChildNode( Idx )->initialise( pReferencePose );
	}
}

//////////////////////////////////////////////////////////////////////////
// getWorkingPose
//virtual
const ScnAnimationPose& ScnAnimationTreeNode::getWorkingPose() const
{
	BcAssert( pWorkingPose_ != NULL );
	return *pWorkingPose_;
}
