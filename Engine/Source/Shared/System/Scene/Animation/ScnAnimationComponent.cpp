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

#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnAnimationComponent );

void ScnAnimationComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Target_", &ScnAnimationComponent::Target_, bcRFF_IMPORTER ),
		new ReField( "Tree_", &ScnAnimationComponent::Tree_, bcRFF_IMPORTER ),

		new ReField( "Model_", &ScnAnimationComponent::Model_, bcRFF_TRANSIENT ),
		new ReField( "pReferencePose_", &ScnAnimationComponent::pReferencePose_ ),
	};
	
	ReRegisterClass< ScnAnimationComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( -2050 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnAnimationComponent::ScnAnimationComponent():
	Tree_( nullptr ),
	pReferencePose_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnAnimationComponent::~ScnAnimationComponent()
{
	delete Tree_;
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnAnimationComponent::destroy()
{
	// TODO: unique_ptr.
	delete Tree_;
	Tree_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual 
void ScnAnimationComponent::preUpdate( BcF32 Tick )
{
	if( Tree_ != nullptr )
	{
		Tree_->preUpdate( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual 
void ScnAnimationComponent::update( BcF32 Tick )
{
	if( Tree_ != nullptr )
	{
		Tree_->update( Tick );
	}
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual 
void ScnAnimationComponent::postUpdate( BcF32 Tick )
{
	if( Tree_ != nullptr )
	{
		Tree_->postUpdate( Tick );
	}

	applyPose();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual 
void ScnAnimationComponent::onAttach( ScnEntityWeakRef Parent )
{
	Model_ = getComponentByType< ScnModelComponent >( /*Target_ TODO*/  ); 
	BcAssertMsg( Model_ != nullptr, "Can't find target model component \"%s\"", (*Target_).c_str() );

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

	Tree_->initialise( pReferencePose_, ModelNodeFileData_.data() );
}

//////////////////////////////////////////////////////////////////////////
// applyPose
void ScnAnimationComponent::applyPose()
{
	const ScnAnimationPose& WorkingPose( Tree_->getWorkingPose() );
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
