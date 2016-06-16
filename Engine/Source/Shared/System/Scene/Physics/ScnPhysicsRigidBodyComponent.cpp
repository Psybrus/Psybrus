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
#include "System/Scene/Physics/ScnPhysics.h"
#include "System/Scene/ScnComponentProcessor.h"
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
		new ReField( "CollisionGroup_", &ScnPhysicsRigidBodyComponent::CollisionGroup_, bcRFF_IMPORTER ),
		new ReField( "CollisionMask_", &ScnPhysicsRigidBodyComponent::CollisionMask_, bcRFF_IMPORTER ),
		new ReField( "Friction_", &ScnPhysicsRigidBodyComponent::Friction_, bcRFF_IMPORTER ),
		new ReField( "RollingFriction_", &ScnPhysicsRigidBodyComponent::RollingFriction_, bcRFF_IMPORTER ),
		new ReField( "Restitution_", &ScnPhysicsRigidBodyComponent::Restitution_, bcRFF_IMPORTER ),
		new ReField( "LinearSleepingThreshold_", &ScnPhysicsRigidBodyComponent::LinearSleepingThreshold_, bcRFF_IMPORTER ),
		new ReField( "AngularSleepingThreshold_", &ScnPhysicsRigidBodyComponent::AngularSleepingThreshold_, bcRFF_IMPORTER ),
	};

	using namespace std::placeholders;
	ReRegisterClass< ScnPhysicsRigidBodyComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Update",
					ScnComponentPriority::PHYSICS_RIGID_BODY_UPDATE,
					std::bind( &ScnPhysicsRigidBodyComponent::updateBodies, _1 ) )
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsRigidBodyComponent::ScnPhysicsRigidBodyComponent():
	RigidBody_( nullptr ),
	Mass_ ( 0.0f ),
	RollingFriction_( 0.0f ),
	Friction_( 0.0f ),
	Restitution_( 0.0f ),
	LinearSleepingThreshold_( 0.8f ), // bullet default.
	AngularSleepingThreshold_( 1.0f ) // bullet default.
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsRigidBodyComponent::~ScnPhysicsRigidBodyComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// applyTorque
void ScnPhysicsRigidBodyComponent::applyTorque( const MaVec3d& Torque )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyTorque( ScnPhysicsToBullet( Torque ) );
}

//////////////////////////////////////////////////////////////////////////
// applyLocalTorque
void ScnPhysicsRigidBodyComponent::applyLocalTorque( const MaVec3d& Torque )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyTorque( RigidBody_->getInvInertiaTensorWorld().inverse() * RigidBody_->getWorldTransform().getBasis() * ScnPhysicsToBullet( Torque ) );
}

//////////////////////////////////////////////////////////////////////////
// applyForce
void ScnPhysicsRigidBodyComponent::applyForce( const MaVec3d& Force, const MaVec3d& RelativePos )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyForce( ScnPhysicsToBullet( Force ), ScnPhysicsToBullet( RelativePos ) );
}

//////////////////////////////////////////////////////////////////////////
// applyCentralForce
void ScnPhysicsRigidBodyComponent::applyCentralForce( const MaVec3d& Force )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyCentralForce( ScnPhysicsToBullet( Force ) );
}

//////////////////////////////////////////////////////////////////////////
// applyTorqueImpulse
void ScnPhysicsRigidBodyComponent::applyTorqueImpulse( const MaVec3d& Torque )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyTorqueImpulse( ScnPhysicsToBullet( Torque ) );
}

//////////////////////////////////////////////////////////////////////////
// applyLocalTorqueImpulse
void ScnPhysicsRigidBodyComponent::applyLocalTorqueImpulse( const MaVec3d& Torque )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyTorqueImpulse( RigidBody_->getInvInertiaTensorWorld().inverse() * RigidBody_->getWorldTransform().getBasis() * ScnPhysicsToBullet( Torque ) );
}

//////////////////////////////////////////////////////////////////////////
// applyImpulse
void ScnPhysicsRigidBodyComponent::applyImpulse( const MaVec3d& Impulse, const MaVec3d& RelativePos )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyImpulse( ScnPhysicsToBullet( Impulse ), ScnPhysicsToBullet( RelativePos ) );
}

//////////////////////////////////////////////////////////////////////////
// applyCentralImpulse
void ScnPhysicsRigidBodyComponent::applyCentralImpulse( const MaVec3d& Impulse )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->applyCentralImpulse( ScnPhysicsToBullet( Impulse ) );
}

//////////////////////////////////////////////////////////////////////////
// setLinearVelocity
void ScnPhysicsRigidBodyComponent::setLinearVelocity( const MaVec3d& Velocity )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->setLinearVelocity( ScnPhysicsToBullet( Velocity ) );
}

//////////////////////////////////////////////////////////////////////////
// getLinearVelocity
MaVec3d ScnPhysicsRigidBodyComponent::getLinearVelocity() const
{
	BcAssert( RigidBody_ != nullptr );	
	return ScnPhysicsFromBullet( RigidBody_->getLinearVelocity() );
}

//////////////////////////////////////////////////////////////////////////
// setAngularVelocity
void ScnPhysicsRigidBodyComponent::setAngularVelocity( const MaVec3d& Velocity )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->setAngularVelocity( ScnPhysicsToBullet( Velocity ) );
}

//////////////////////////////////////////////////////////////////////////
// getAngularVelocity
MaVec3d ScnPhysicsRigidBodyComponent::getAngularVelocity() const
{
	BcAssert( RigidBody_ != nullptr );	
	return ScnPhysicsFromBullet( RigidBody_->getAngularVelocity() );
}

//////////////////////////////////////////////////////////////////////////
// getVelocityInLocalPoint
MaVec3d ScnPhysicsRigidBodyComponent::getVelocityInLocalPoint( const MaVec3d& LocalPoint )
{
	return ScnPhysicsFromBullet( RigidBody_->getVelocityInLocalPoint( ScnPhysicsToBullet( LocalPoint ) ) );
}

//////////////////////////////////////////////////////////////////////////
// setMass
void ScnPhysicsRigidBodyComponent::setMass( BcF32 Mass )
{
	btVector3 Inertia;
	RigidBody_->getCollisionShape()->calculateLocalInertia( Mass, Inertia );
	RigidBody_->setMassProps( Mass, Inertia );
	RigidBody_->activate();
	Mass_ = Mass;
}

//////////////////////////////////////////////////////////////////////////
// getMass
BcF32 ScnPhysicsRigidBodyComponent::getMass() const
{
	return Mass_;
}

//////////////////////////////////////////////////////////////////////////
// translate
void ScnPhysicsRigidBodyComponent::translate( const MaVec3d& V )
{
	BcAssert( RigidBody_ != nullptr );	
	RigidBody_->activate();
	RigidBody_->translate( ScnPhysicsToBullet( V ) ); 
}

//////////////////////////////////////////////////////////////////////////
// getPosition
MaVec3d ScnPhysicsRigidBodyComponent::getPosition() const
{
	return ScnPhysicsFromBullet( RigidBody_->getCenterOfMassPosition() );
}

//////////////////////////////////////////////////////////////////////////
// getRotation
MaQuat ScnPhysicsRigidBodyComponent::getRotation() const
{
	return ScnPhysicsFromBullet( RigidBody_->getCenterOfMassTransform().getRotation() );
}

//////////////////////////////////////////////////////////////////////////
// setTransform
void ScnPhysicsRigidBodyComponent::setTransform( const MaMat4d& NewTransform )
{
	auto Trans = ScnPhysicsToBullet( NewTransform );
	RigidBody_->setCenterOfMassTransform( Trans );
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
	ConstructionInfo.m_rollingFriction = RollingFriction_;
	ConstructionInfo.m_restitution = Restitution_;
	ConstructionInfo.m_linearSleepingThreshold = LinearSleepingThreshold_;
	ConstructionInfo.m_angularSleepingThreshold = AngularSleepingThreshold_;
	RigidBody_ = new btRigidBody( ConstructionInfo );
	RigidBody_->setUserPointer( this );
	World_->addRigidBody( RigidBody_, CollisionGroup_, CollisionMask_ );
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

//////////////////////////////////////////////////////////////////////////
// getRigidBody
btRigidBody* ScnPhysicsRigidBodyComponent::getRigidBody()
{
	return RigidBody_;
}

//////////////////////////////////////////////////////////////////////////
// updateBodies
// static
void ScnPhysicsRigidBodyComponent::updateBodies( const ScnComponentList& Components )
{
	// TODO: Interpolate?
	
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnPhysicsRigidBodyComponent >() );
		auto* RBComponent = static_cast< ScnPhysicsRigidBodyComponent* >( Component.get() );

		// Set transform from rigid body.
		BcAssert( RBComponent->RigidBody_ );
		const btTransform& BulletTransform = RBComponent->RigidBody_->getCenterOfMassTransform();
		ATTRIBUTE_ALIGNED16( MaMat4d ) Transform;

		// Set transform using bullet's GL matrix stuff.
		BulletTransform.getOpenGLMatrix( reinterpret_cast< btScalar* >( &Transform ) );

		// TODO: Use inverse of parent transform.
		RBComponent->getParentEntity()->setLocalMatrix( Transform );
	}
}
