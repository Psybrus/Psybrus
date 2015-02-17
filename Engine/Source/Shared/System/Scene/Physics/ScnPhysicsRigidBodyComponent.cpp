/**************************************************************************
*
* File:		ScnPhysicsRigidBodyComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	20/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsRigidBodyComponent.h"
#include "System/Scene/Physics/ScnPhysicsWorldComponent.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnPhysicsRigidBodyComponent );

void ScnPhysicsRigidBodyComponent::StaticRegisterClass()
{
	ReRegisterClass< ScnPhysicsRigidBodyComponent, Super >();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPhysicsRigidBodyComponent::initialise()
{
	RigidBody_ = NULL;
	Mass_ = 0.0f;
	Friction_ = 0.0f;
	Restitution_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnPhysicsRigidBodyComponent::update( BcF32 Tick )
{
	// Set transform from rigid body.
	const btTransform& BulletTransform = RigidBody_->getCenterOfMassTransform();
	MaMat4d Transform;

	// Set transform using bullet's GL matrix stuff.
	BulletTransform.getOpenGLMatrix( reinterpret_cast< btScalar* >( &Transform ) );

	// TODO: Use inverse of parent transform.
	getParentEntity()->setLocalMatrix( Transform );
	
	//
	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnPhysicsRigidBodyComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	// Get parent world.
	World_ = getComponentAnyParentByType< ScnPhysicsWorldComponent >();

	// Calculate local inertia.
	btCollisionShape* CollisionShape = CollisionShape_->getCollisionShape();
	BcBool IsDynamic = (Mass_ != 0.f);
	btVector3 LocalInertia(0,0,0);
	if (IsDynamic)
	{
		CollisionShape->calculateLocalInertia( Mass_, LocalInertia );
	}
	
	// Create rigid body.
	btTransform StartTransform;
	const MaMat4d& LocalMatrix = getParentEntity()->getLocalMatrix();
	StartTransform.setFromOpenGLMatrix( reinterpret_cast< const btScalar* >( &LocalMatrix ) );

	btRigidBody::btRigidBodyConstructionInfo ConstructionInfo(
			Mass_,
			NULL,
			CollisionShape,
			LocalInertia );
	ConstructionInfo.m_startWorldTransform = StartTransform;
	ConstructionInfo.m_friction = Friction_;
	ConstructionInfo.m_restitution = Restitution_;
	RigidBody_ = new btRigidBody( ConstructionInfo );

	World_->addRigidBody( RigidBody_ );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnPhysicsRigidBodyComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Remove rigid body.
	World_->removeRigidBody( RigidBody_ );
	delete RigidBody_;
	RigidBody_ = NULL;

	// Clear world.
	World_ = NULL;

	Super::onDetach( Parent );
}
