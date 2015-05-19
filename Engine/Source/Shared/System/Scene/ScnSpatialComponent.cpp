/**************************************************************************
*
* File:		ScnSpatialComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	13/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnSpatialComponent.h"
#include "System/Scene/ScnComponentProcessor.h"

#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnSpatialComponent );

void ScnSpatialComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pSpatialTreeNode_", &ScnSpatialComponent::pSpatialTreeNode_, bcRFF_TRANSIENT ),
	};
	
	using namespace std::placeholders;	
	ReRegisterClass< ScnSpatialComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Update",
					ScnComponentPriority::SPATIAL_UPDATE,
					std::bind( &ScnSpatialComponent::update, _1 ) ),
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSpatialComponent::ScnSpatialComponent():
	pSpatialTreeNode_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSpatialComponent::~ScnSpatialComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnSpatialComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnSpatialComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// setSpatialTreeNode
void ScnSpatialComponent::setSpatialTreeNode( ScnSpatialTreeNode* pNode )
{
	pSpatialTreeNode_ = pNode;
}

//////////////////////////////////////////////////////////////////////////
// getSpatialTreeNode
ScnSpatialTreeNode* ScnSpatialComponent::getSpatialTreeNode()
{
	return pSpatialTreeNode_;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB ScnSpatialComponent::getAABB() const
{
	BcAssertMsg( BcFalse, "ScnSpatialComponent: Not implemented a getAABB!" );
	return MaAABB();
}

//////////////////////////////////////////////////////////////////////////
// update
//static
void ScnSpatialComponent::update( const ScnComponentList& Components )
{
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnSpatialComponent >() );
		auto* SpatialComponent = static_cast< ScnSpatialComponent* >( Component.get() );
		SpatialComponent->pSpatialTreeNode_->reinsertComponent( SpatialComponent );
	}
}

