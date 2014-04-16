/**************************************************************************
*
* File:		ScnAnimationComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	05/01/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Animation/ScnAnimationComponent.h"

#include "System/Scene/Animation/ScnAnimationTreeNode.h"
#include "System/Scene/Animation/ScnAnimationTreeBlendNode.h"
#include "System/Scene/Animation/ScnAnimationTreeTrackNode.h"

#include "System/Content/CsCore.h"

#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnAnimationComponent );

//BCREFLECTION_EMPTY_REGISTER( ScnAnimationComponent );
/*
REFLECTION_DERIVED_BEGIN( ScnComponent, ScnAnimationComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
REFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual 
void ScnAnimationComponent::initialise( const Json::Value& Object )
{
	Super::initialise();

	// HACK: Setup a way to add child types in the defines.
	static BcBool HasRegisteredReflection = BcFalse;
	if( HasRegisteredReflection == BcFalse )
	{
		HasRegisteredReflection = BcTrue;
		BcBreakpoint;
		//ScnAnimationTreeNode::StaticRegisterReflection();
		//ScnAnimationTreeBlendNode::StaticRegisterReflection();
		//ScnAnimationTreeTrackNode::StaticRegisterReflection();
		//ScnAnimationPose::StaticRegisterReflection();
	}

	//
	TargetComponentName_ = Object[ "target" ].asCString();

	//
	pRootTreeNode_ = NULL;
	pReferencePose_ = NULL;

	// Initialise tree nodes.
	// TODO: Setup with reflection.
	const Json::Value& TreeValue = Object[ "tree" ];
	initialiseNode( NULL, 0, TreeValue );
}

//////////////////////////////////////////////////////////////////////////
// initialiseNode
void ScnAnimationComponent::initialiseNode( ScnAnimationTreeNode* pParentNode, BcU32 ChildIndex, const Json::Value& Object )
{
	const Json::Value& TypeValue = Object[ "type" ];
	const Json::Value& NameValue = Object[ "name" ];
	const Json::Value& ChildrenValue = Object[ "children" ];
	const ReClass* pClass = ReManager::GetClass( TypeValue.asCString() );
	ScnAnimationTreeNode* pNode = pClass->construct< ScnAnimationTreeNode >();
	pNode->setName( NameValue.asCString() );

	if( pParentNode != NULL )
	{
		pParentNode->setChildNode( ChildIndex, pNode );
	}
	else
	{
		pRootTreeNode_ = pNode;
	}

	if( ChildrenValue.type() != Json::nullValue )
	{
		for( BcU32 Idx = 0; Idx < ChildrenValue.size(); ++Idx )
		{
			const Json::Value& ChildValue = ChildrenValue[ Idx ];
			initialiseNode( pNode, Idx, ChildValue );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual 
void ScnAnimationComponent::preUpdate( BcF32 Tick )
{
	if( pRootTreeNode_ != NULL )
	{
		pRootTreeNode_->preUpdate( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual 
void ScnAnimationComponent::update( BcF32 Tick )
{
	if( pRootTreeNode_ != NULL )
	{
		pRootTreeNode_->update( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual 
void ScnAnimationComponent::postUpdate( BcF32 Tick )
{
	if( pRootTreeNode_ != NULL )
	{
		pRootTreeNode_->postUpdate( Tick );
	}

	applyPose();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual 
void ScnAnimationComponent::onAttach( ScnEntityWeakRef Parent )
{
	Model_ = getParentEntity()->getComponentByType< ScnModelComponent >( ReManager::GetClass( *TargetComponentName_ ) );
	BcAssertMsg( Model_.isValid(), "Can't find target model component \"%s\"", (*TargetComponentName_).c_str() );

	// Setup the reference pose.
	if( Model_.isValid() )
	{
		buildReferencePose();
	}

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnAnimationComponent::onDetach( ScnEntityWeakRef Parent )
{
	Model_ = NULL;
	delete pReferencePose_;
	pReferencePose_ = NULL;

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// buildReferencePose
void ScnAnimationComponent::buildReferencePose()
{
	const BcU32 NoofNodes = Model_->getNoofNodes();
	pReferencePose_ = new ScnAnimationPose( NoofNodes - 1 );

	ScnAnimationTransform Transform;
	for( BcU32 Idx = 1; Idx < NoofNodes; ++Idx )
	{
		Transform.fromMatrix( Model_->getNode( Idx ) );
		pReferencePose_->setTransform( Idx - 1, Transform );
	}

	pRootTreeNode_->initialise( pReferencePose_ );
}

//////////////////////////////////////////////////////////////////////////
// applyPose
void ScnAnimationComponent::applyPose()
{
	const ScnAnimationPose& WorkingPose( pRootTreeNode_->getWorkingPose() );
	const BcU32 NoofNodes = Model_->getNoofNodes();
	BcMat4d Matrix;
	for( BcU32 Idx = 1; Idx < NoofNodes; ++Idx )
	{
		const ScnAnimationTransform& Transform( WorkingPose.getTransform( Idx - 1 ) );
		Transform.toMatrix( Matrix );
		Model_->setNode( Idx, Matrix );
	}
}

//////////////////////////////////////////////////////////////////////////
// findNodeRecursively
ScnAnimationTreeNode* ScnAnimationComponent::findNodeRecursively( ScnAnimationTreeNode* pStartNode, const BcName& Name, const ReClass* Class )
{
	if( pStartNode->getName() == Name &&
		pStartNode->getClass()->getTypeName() == Class->getTypeName() )
	{
		return pStartNode;
	}

	BcU32 NoofNodes = pStartNode->getNoofChildNodes();
	for( BcU32 Idx = 0; Idx < NoofNodes; ++Idx )
	{
		ScnAnimationTreeNode* pNode = pStartNode->getChildNode( Idx );
		if( ( pNode = findNodeRecursively( pNode, Name, Class ) ) != NULL )
		{
			return pNode;
		}
	}

	return NULL;
}
