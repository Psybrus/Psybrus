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
BCREFLECTION_DEFINE_BASE( ScnAnimationTreeNode );
BCREFLECTION_ABSTRACT_BASE_BEGIN( ScnAnimationTreeNode )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( ScnAnimationPose,					pReferencePose_,				bcRFF_POINTER ),
	BCREFLECTION_MEMBER( ScnAnimationPose,					pWorkingPose_,					bcRFF_POINTER ),
BCREFLECTION_ABSTRACT_BASE_END();

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
// setName
void ScnAnimationTreeNode::setName( const BcName& Name )
{
	Name_ = Name;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& ScnAnimationTreeNode::getName() const
{
	return Name_;
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
