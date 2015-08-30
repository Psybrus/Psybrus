/**************************************************************************
*
* File:		ScnPhysicsConstraintComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsConstraintComponent.h"
#include "System/Scene/Physics/ScnPhysicsWorldComponent.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsConstraintComponent );

void ScnPhysicsConstraintComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "World_", &ScnPhysicsConstraintComponent::World_, bcRFF_SHALLOW_COPY ),
	};

	ReRegisterClass< ScnPhysicsConstraintComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsConstraintComponent::ScnPhysicsConstraintComponent():
	World_( nullptr ),
	Constraint_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsConstraintComponent::~ScnPhysicsConstraintComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnPhysicsConstraintComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Get parent world.
	World_ = getComponentAnyParentByType< ScnPhysicsWorldComponent >();
	BcAssert( World_ );

	if( Constraint_ != nullptr )
	{
		World_->addConstraint( Constraint_ );
	}

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnPhysicsConstraintComponent::onDetach( ScnEntityWeakRef Parent )
{
	if( Constraint_ != nullptr )
	{
		World_->removeConstraint( Constraint_ );
	}

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getConstraint
//virtual
class btTypedConstraint* ScnPhysicsConstraintComponent::getConstraint()
{
	return Constraint_;
}
