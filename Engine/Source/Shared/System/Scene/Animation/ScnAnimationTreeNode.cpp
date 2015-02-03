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
		new ReField( "pReferencePose_", &ScnAnimationTreeNode::pNodeFileData_, bcRFF_SHALLOW_COPY ),
		new ReField( "pReferencePose_", &ScnAnimationTreeNode::pReferencePose_, bcRFF_SHALLOW_COPY ),
		new ReField( "pWorkingPose_", &ScnAnimationTreeNode::pWorkingPose_ ),
	};
		
	ReRegisterAbstractClass< ScnAnimationTreeNode, Super >( Fields );
}


//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeNode::ScnAnimationTreeNode():
	pNodeFileData_( nullptr ),
	pReferencePose_( nullptr ),
	pWorkingPose_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationTreeNode::ScnAnimationTreeNode( ReNoInit ):
	pNodeFileData_( nullptr ),
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
	pWorkingPose_ = nullptr;
	pReferencePose_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnAnimationTreeNode::initialise( 
	ScnAnimationPose* pReferencePose, 
	ScnAnimationNodeFileData* pNodeFileData )
{
	BcAssert( pReferencePose_ == nullptr );
	BcAssert( pWorkingPose_ == nullptr );

	pNodeFileData_ = pNodeFileData;
	pReferencePose_ = pReferencePose;
	pWorkingPose_ = new ScnAnimationPose( *pReferencePose );

	for( BcU32 Idx = 0; Idx < getNoofChildNodes(); ++Idx )
	{
		getChildNode( Idx )->initialise( pReferencePose, pNodeFileData );
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
