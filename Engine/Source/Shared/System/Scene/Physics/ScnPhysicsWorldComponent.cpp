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

#include "System/Content/CsCore.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnPhysicsWorldComponent );

BCREFLECTION_EMPTY_REGISTER( ScnPhysicsWorldComponent );
/*
BCREFLECTION_DERIVED_BEGIN( CsResource, ScnPhysicsWorldComponent )
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPhysicsWorldComponent::initialise()
{
	CollisionConfiguration_ = NULL;
	Dispatcher_ = NULL;
	Broadphase_ = NULL;
	Solver_ = NULL;
	DynamicsWorld_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnPhysicsWorldComponent::create()
{
	// Setup dynamics world.
	CollisionConfiguration_ = new btDefaultCollisionConfiguration();
	Dispatcher_ = new	btCollisionDispatcher( CollisionConfiguration_ );
	Broadphase_ = new btDbvtBroadphase();
	Solver_ = new btSequentialImpulseConstraintSolver();
	DynamicsWorld_ = new btDiscreteDynamicsWorld( Dispatcher_, Broadphase_, Solver_, CollisionConfiguration_ );
	DynamicsWorld_->setGravity( btVector3( 0.0f, -10.0f, 0.0f ) );

	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnPhysicsWorldComponent::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnPhysicsWorldComponent::update( BcF32 Tick )
{
	// Step simulation.
	DynamicsWorld_->stepSimulation( Tick, 0 );

	Super::update( Tick ):
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnPhysicsWorldComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnPhysicsWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
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
