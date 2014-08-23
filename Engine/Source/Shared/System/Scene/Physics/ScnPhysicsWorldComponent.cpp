/**************************************************************************
*
* File:		ScnPhysicsWorldComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	20/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsWorldComponent.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnPhysicsWorldComponent );

void ScnPhysicsWorldComponent::StaticRegisterClass()
{
	ReRegisterClass< ScnPhysicsWorldComponent, Super >()
		.addAttribute( new ScnComponentAttribute( -100 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPhysicsWorldComponent::initialise()
{
	Super::initialise();

	CollisionConfiguration_ = nullptr;
	Dispatcher_ = nullptr;
	Broadphase_ = nullptr;
	Solver_ = nullptr;
	DynamicsWorld_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPhysicsWorldComponent::initialise( const Json::Value& Object )
{
	ScnPhysicsWorldComponent::initialise();
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnPhysicsWorldComponent::create()
{

	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnPhysicsWorldComponent::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
//virtual
void ScnPhysicsWorldComponent::preUpdate( BcF32 Tick )
{
	// Step simulation.
	DynamicsWorld_->stepSimulation( Tick, 0 );

	// Resolve collisions?

	Super::preUpdate( Tick );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnPhysicsWorldComponent::update( BcF32 Tick )
{

	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// postUpdate
//virtual
void ScnPhysicsWorldComponent::postUpdate( BcF32 Tick )
{

	Super::postUpdate( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnPhysicsWorldComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Setup dynamics world.
	CollisionConfiguration_ = new btDefaultCollisionConfiguration();
	Dispatcher_ = new	btCollisionDispatcher( CollisionConfiguration_ );
	Broadphase_ = new btDbvtBroadphase();
	Solver_ = new btSequentialImpulseConstraintSolver();
	DynamicsWorld_ = new btDiscreteDynamicsWorld( Dispatcher_, Broadphase_, Solver_, CollisionConfiguration_ );
	DynamicsWorld_->setGravity( btVector3( 0.0f, -10.0f, 0.0f ) );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnPhysicsWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
	delete DynamicsWorld_;
	delete Solver_;
	delete Broadphase_;
	delete Dispatcher_;
	delete CollisionConfiguration_;
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// addRigidBody
void ScnPhysicsWorldComponent::addRigidBody( btRigidBody* RigidBody )
{
	DynamicsWorld_->addRigidBody( RigidBody );
}

//////////////////////////////////////////////////////////////////////////
// removeRigidBody
void ScnPhysicsWorldComponent::removeRigidBody( btRigidBody* RigidBody )
{
	DynamicsWorld_->removeRigidBody( RigidBody );
}
