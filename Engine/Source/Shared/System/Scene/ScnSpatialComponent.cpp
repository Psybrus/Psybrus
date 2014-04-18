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
#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSpatialComponent );

void ScnSpatialComponent::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "pSpatialTreeNode_",			&ScnSpatialComponent::pSpatialTreeNode_ ),
	};
		
	ReRegisterClass< ScnSpatialComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnSpatialComponent::initialise()
{
	Super::initialise();

	pSpatialTreeNode_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSpatialComponent::initialise( const Json::Value& Object )
{
	initialise();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnSpatialComponent::postUpdate( BcF32 Tick )
{
	Super::update( Tick );

	// Reinsert node if we need to.
	pSpatialTreeNode_->reinsertComponent( this );
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
BcAABB ScnSpatialComponent::getAABB() const
{
	BcAssertMsg( BcFalse, "ScnSpatialComponent: Not implemented a getAABB!" );
	return BcAABB();
}
