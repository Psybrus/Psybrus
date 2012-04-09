/**************************************************************************
*
* File:		ScnComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	26/11/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnEntity.h"
#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnComponent::initialise()
{
	pHeader_ = NULL;
	pSpacialTreeNode_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnComponent::update( BcReal Tick )
{
	
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnComponent::onAttach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( ParentEntity_.isValid() == BcFalse || ParentEntity_ == Parent, "Attempting to attach component when it's already attached!" );

	ParentEntity_ = Parent;
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnComponent::onDetach( ScnEntityWeakRef Parent )
{
	BcAssertMsg( ParentEntity_.isValid() == BcTrue, "Attempting to detach component that is already detached!" );
	BcAssertMsg( ParentEntity_ == Parent, "Attempting to detach component from an entity it isn't attached to!" );

	ParentEntity_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// isAttached
BcBool ScnComponent::isAttached() const
{
	return ParentEntity_.isValid();
}

//////////////////////////////////////////////////////////////////////////
// isAttached
BcBool ScnComponent::isAttached( ScnEntityWeakRef Parent ) const
{
	return ParentEntity_ == Parent;
}

//////////////////////////////////////////////////////////////////////////
// getParentEntity
ScnEntityWeakRef ScnComponent::getParentEntity()
{
	return ParentEntity_;
}

//////////////////////////////////////////////////////////////////////////
// setSpatialTreeNode
void ScnComponent::setSpatialTreeNode( ScnSpatialTreeNode* pNode )
{
	pSpacialTreeNode_ = pNode;
}

//////////////////////////////////////////////////////////////////////////
// getSpatialTreeNode
ScnSpatialTreeNode* ScnComponent::getSpatialTreeNode()
{
	return pSpacialTreeNode_;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
const BcAABB& ScnComponent::getAABB() const
{
	static BcAABB TEMP;
	return TEMP;
}
