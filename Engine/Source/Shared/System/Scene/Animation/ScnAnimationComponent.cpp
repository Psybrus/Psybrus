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

void ScnAnimationComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "TargetComponentName_", &ScnAnimationComponent::TargetComponentName_ ),
		new ReField( "Model_", &ScnAnimationComponent::Model_, bcRFF_TRANSIENT ),
		new ReField( "pRootTreeNode_", &ScnAnimationComponent::pRootTreeNode_ ),
		new ReField( "pReferencePose_", &ScnAnimationComponent::pReferencePose_ ),
	};
		
	ReRegisterClass< ScnAnimationComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2050 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual 
void ScnAnimationComponent::initialise()
{
	pRootTreeNode_ = nullptr;
	pReferencePose_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual 
void ScnAnimationComponent::initialise( const Json::Value& Object )
{
	Super::initialise();

	//
	TargetComponentName_ = Object[ "target" ].asCString();

	//
	pRootTreeNode_ = nullptr;
	pReferencePose_ = nullptr;

	// Initialise tree nodes.
	// TODO: Setup with reflection.
	const Json::Value& TreeValue = Object[ "tree" ];
	initialiseNode( nullptr, 0, TreeValue );
}

//////////////////////////////////////////////////////////////////////////
// initialiseNode
void ScnAnimationComponent::initialiseNode( ScnAnimationTreeNode* pParentNode, BcU32 ChildIndex, const Json::Value& Object )
{
	const Json::Value& TypeValue = Object[ "type" ];
	const Json::Value& NameValue = Object[ "name" ];
	const Json::Value& ChildrenValue = Object[ "children" ];
	const ReClass* pClass = ReManager::GetClass( TypeValue.asCString() );
	ScnAnimationTreeNode* pNode = pClass->create< ScnAnimationTreeNode >();
	pNode->setName( NameValue.asCString() );

	if( pParentNode != nullptr )
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
// destroy
//virtual
void ScnAnimationComponent::destroy()
{
	// TODO: unique_ptr.
	delete pRootTreeNode_;
	pRootTreeNode_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual 
void ScnAnimationComponent::preUpdate( BcF32 Tick )
{
	if( pRootTreeNode_ != nullptr )
	{
		pRootTreeNode_->preUpdate( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual 
void ScnAnimationComponent::update( BcF32 Tick )
{
	if( pRootTreeNode_ != nullptr )
	{
		pRootTreeNode_->update( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual 
void ScnAnimationComponent::postUpdate( BcF32 Tick )
{
	if( pRootTreeNode_ != nullptr )
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
	Model_ = getComponentByType< ScnModelComponent >( /*TargetComponentName_ TODO*/  ); 
	BcAssertMsg( Model_ != nullptr, "Can't find target model component \"%s\"", (*TargetComponentName_).c_str() );

	// Setup the reference pose.
	if( Model_ != nullptr )
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
	Model_ = nullptr;
	delete pReferencePose_;
	pReferencePose_ = nullptr;

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// buildReferencePose
void ScnAnimationComponent::buildReferencePose()
{
	const BcU32 NoofNodes = Model_->getNoofNodes();
	const BcU32 NoofAnimNodes = NoofNodes - 1;

	// Grab node names for model we're animating.
	ModelNodeFileData_.reserve( NoofAnimNodes );
	for( BcU32 Idx = 1; Idx < NoofNodes; ++Idx )
	{
		ScnAnimationNodeFileData NodeFileData = 
		{
			Model_->findNodeNameByIndex( Idx )
		};

		ModelNodeFileData_.push_back( NodeFileData );
	}

	// Reference pose.
	pReferencePose_ = new ScnAnimationPose( NoofAnimNodes );

	ScnAnimationTransform Transform;
	for( BcU32 Idx = 1; Idx < NoofNodes; ++Idx )
	{
		Transform.fromMatrix( Model_->getNode( Idx ) );
		pReferencePose_->setTransform( Idx - 1, Transform );
	}

	pRootTreeNode_->initialise( pReferencePose_, ModelNodeFileData_.data() );
}

//////////////////////////////////////////////////////////////////////////
// applyPose
void ScnAnimationComponent::applyPose()
{
	const ScnAnimationPose& WorkingPose( pRootTreeNode_->getWorkingPose() );
	const BcU32 NoofNodes = Model_->getNoofNodes();
	MaMat4d Matrix;
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
		if( ( pNode = findNodeRecursively( pNode, Name, Class ) ) != nullptr )
		{
			return pNode;
		}
	}

	return nullptr;
}
