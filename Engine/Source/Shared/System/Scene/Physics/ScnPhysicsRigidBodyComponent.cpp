/**************************************************************************
*
* File:		ScnPhysicsRigidBodyComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsRigidBodyComponent.h"
#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysicsWorldComponent.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsRigidBodyComponent );

void ScnPhysicsRigidBodyComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Mass_", &ScnPhysicsRigidBodyComponent::Mass_, bcRFF_IMPORTER ),
		new ReField( "Friction_", &ScnPhysicsRigidBodyComponent::Friction_, bcRFF_IMPORTER ),
		new ReField( "Restitution_", &ScnPhysicsRigidBodyComponent::Restitution_, bcRFF_IMPORTER ),
	};

	ReRegisterClass< ScnPhysicsRigidBodyComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsRigidBodyComponent::ScnPhysicsRigidBodyComponent():
	RigidBody_( nullptr ),
	Mass_ ( 0.0f ),
	Friction_( 0.0f ),
	Restitution_( 0.0f )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsRigidBodyComponent::~ScnPhysicsRigidBodyComponent()
{
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
	BcAssert( World_ );

	// Get collision component from parent.
	CollisionComponent_ = getComponentByType< ScnPhysicsCollisionComponent >();
	BcAssert( CollisionComponent_ );

	// Calculate local inertia.
	btCollisionShape* CollisionShape = CollisionComponent_->getCollisionShape();
	BcBool IsDynamic = ( Mass_ != 0.0f );
	btVector3 LocalInertia( 0.0f, 0.0f, 0.0f );
	if( IsDynamic )
	{
		CollisionShape->calculateLocalInertia( Mass_, LocalInertia );
	}
	
	// Create rigid body.
	btTransform StartTransform;
	const MaMat4d& LocalMatrix = getParentEntity()->getLocalMatrix();
	StartTransform.setFromOpenGLMatrix( reinterpret_cast< const btScalar* >( &LocalMatrix ) );

	btRigidBody::btRigidBodyConstructionInfo ConstructionInfo(
			Mass_,
			nullptr,
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
	RigidBody_ = nullptr;

	// Clear world.
	World_ = nullptr;

	Super::onDetach( Parent );
}
