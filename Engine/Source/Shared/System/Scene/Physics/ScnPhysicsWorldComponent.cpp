/**************************************************************************
*
* File:		ScnPhysicsWorldComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsWorldComponent.h"
#include "System/Scene/Physics/ScnPhysicsRigidBodyComponent.h"
#include "System/Scene/Physics/ScnPhysicsCollisionComponent.h"
#include "System/Scene/Physics/ScnPhysics.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Debug/DsCore.h"
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

#include "LinearMath/btIDebugDraw.h"

class DebugRenderer: 
	public btIDebugDraw
{
public:
	DebugRenderer()
	{
		DebugMode_ = 
			btIDebugDraw::DBG_DrawWireframe |
			btIDebugDraw::DBG_DrawContactPoints |
			btIDebugDraw::DBG_DrawConstraints;
	}

	virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		ScnDebugRenderComponent::pImpl()->drawLine(
			MaVec3d( from.x(), from.y(), from.z() ),
			MaVec3d( to.x(), to.y(), to.z() ),
			RsColour( color.x(), color.y(), color.z(), 1.0f ) );
	}

	virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
	{
		ScnDebugRenderComponent::pImpl()->drawLine(
			ScnPhysicsFromBullet( PointOnB ) - ScnPhysicsFromBullet( normalOnB ),
			ScnPhysicsFromBullet( PointOnB ) + ScnPhysicsFromBullet( normalOnB ),
			RsColour( color.x(), color.y(), color.z(), 1.0f ) );
	}

	virtual void reportErrorWarning(const char* warningString)
	{

	}

	virtual void draw3dText(const btVector3& location,const char* textString)
	{

	}
	
	virtual void setDebugMode(int debugMode)
	{
		DebugMode_ = debugMode;
	}
	
	virtual int getDebugMode() const
	{
		return DebugMode_;
	}

private:
	int DebugMode_;
};

static DebugRenderer gDebugRenderer;


class UpdateActions: 
	public btActionInterface
{
public:
	UpdateActions( ScnPhysicsWorldComponent* World ):
		World_( World )
	{

	}

	void updateAction( btCollisionWorld* collisionWorld, btScalar deltaTimeStep ) override
	{
		for( auto* Handler : World_->WorldUpdateHandler_ )
		{
			Handler->onPhysicsUpdate( deltaTimeStep );
		}
	}

	void debugDraw( btIDebugDraw* debugDrawer ) override
	{

	}

private:
	ScnPhysicsWorldComponent* World_;
};



//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsWorldComponent );

void ScnPhysicsWorldComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Gravity_", &ScnPhysicsWorldComponent::Gravity_, bcRFF_IMPORTER ),
		new ReField( "MaxSubSteps_", &ScnPhysicsWorldComponent::MaxSubSteps_, bcRFF_IMPORTER ),
		new ReField( "FrameRate_", &ScnPhysicsWorldComponent::FrameRate_, bcRFF_IMPORTER ),
		new ReField( "DebugDrawWorld_", &ScnPhysicsWorldComponent::DebugDrawWorld_, bcRFF_IMPORTER ),

		new ReField( "InvFrameRate_", &ScnPhysicsWorldComponent::FrameRate_, bcRFF_TRANSIENT ),
	};

	ReRegisterClass< ScnPhysicsWorldComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -100 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsWorldComponent::ScnPhysicsWorldComponent():
	Gravity_( 0.0f, -9.8f, 0.0f ),
	MaxSubSteps_( 10 ),
	FrameRate_( 60.0f ),
	InvFrameRate_( 1.0f / FrameRate_ ),
	DebugDrawWorld_( BcFalse ),
	CollisionConfiguration_( nullptr ),
	Dispatcher_( nullptr ),
	Broadphase_( nullptr ),
	Solver_( nullptr ),
	DynamicsWorld_( nullptr ),
	UpdateActions_( new UpdateActions( this ) )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsWorldComponent::~ScnPhysicsWorldComponent()
{
	delete UpdateActions_;
	delete DynamicsWorld_;
	delete Solver_;
	delete Broadphase_;
	delete Dispatcher_;
	delete CollisionConfiguration_;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnPhysicsWorldComponent::initialise()
{
	BcAssertMsg( FrameRate_ > 0.0f, "FrameRate must be greater than 0." );
	BcAssertMsg( MaxSubSteps_ > 0 && MaxSubSteps_ < 100, "MaxSubSteps must be 0 and 100." )
	InvFrameRate_ = 1.0f / FrameRate_;
}

//////////////////////////////////////////////////////////////////////////
// preUpdate
void ScnPhysicsWorldComponent::preUpdate( BcF32 Tick )
{
	// Step simulation.
	DynamicsWorld_->stepSimulation( Tick, MaxSubSteps_, InvFrameRate_ );

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
	if( DebugDrawWorld_ )
	{
		DynamicsWorld_->debugDrawWorld();
	}

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
	DynamicsWorld_->setGravity( ScnPhysicsToBullet( Gravity_ ) );
	DynamicsWorld_->setDebugDrawer( &gDebugRenderer );
	DynamicsWorld_->addAction( UpdateActions_ );

	// Pre-tick.
	DynamicsWorld_->setInternalTickCallback(
		[]( btDynamicsWorld* DynamicsWorld, btScalar Tick )->void
		{
			ScnPhysicsWorldComponent* World = static_cast< ScnPhysicsWorldComponent* >( DynamicsWorld->getWorldUserInfo() );

			// TODO: Physics tick should go here.

		}, this, true );

	// Post-tick.
	DynamicsWorld_->setInternalTickCallback(
		[]( btDynamicsWorld* DynamicsWorld, btScalar Tick )->void
		{
			ScnPhysicsWorldComponent* World = static_cast< ScnPhysicsWorldComponent* >( DynamicsWorld->getWorldUserInfo() );

			// Gather collisions.
			int NumManifolds = World->Dispatcher_->getNumManifolds();
			ScnPhysicsEventCollision EventA;
			ScnPhysicsEventCollision EventB;
			World->Collisions_.reserve( NumManifolds * 2 );
			for( int ManifoldIdx = 0; ManifoldIdx < NumManifolds; ++ManifoldIdx )
			{
				btPersistentManifold* ContactManifold = World->Dispatcher_->getManifoldByIndexInternal( ManifoldIdx );
				const auto* ObA_ = static_cast< const btCollisionObject* >( ContactManifold->getBody0() );
				const auto* ObB_ = static_cast< const btCollisionObject* >( ContactManifold->getBody1() );
				auto RigidBodyA = static_cast< ScnPhysicsRigidBodyComponent* >( ObA_->getUserPointer() );	
				auto RigidBodyB = static_cast< ScnPhysicsRigidBodyComponent* >( ObB_->getUserPointer() );	
				BcAssert( RigidBodyA->isTypeOf< ScnPhysicsRigidBodyComponent >() );
				BcAssert( RigidBodyB->isTypeOf< ScnPhysicsRigidBodyComponent >() );
				
				EventA.BodyA_ = RigidBodyA;
				EventA.BodyB_ = RigidBodyB;
				EventA.NoofContactPoints_ = 0;
				EventB.BodyA_ = RigidBodyB;
				EventB.BodyB_ = RigidBodyA;
				EventB.NoofContactPoints_ = 0;
				size_t NumContacts = ContactManifold->getNumContacts();
				for( int ContactIdx = 0; ContactIdx < std::min( NumContacts, EventA.ContactPoints_.size() ); ++ContactIdx )
				{
					const btManifoldPoint& Point = ContactManifold->getContactPoint( ContactIdx );
					ScnPhysicsEventCollision::ContactPoint& ContactPointA = EventA.ContactPoints_[ EventA.NoofContactPoints_++ ];
					ScnPhysicsEventCollision::ContactPoint& ContactPointB = EventB.ContactPoints_[ EventB.NoofContactPoints_++ ];
					ContactPointA.PointA_ = ScnPhysicsFromBullet( Point.getPositionWorldOnA() );
					ContactPointA.PointB_ = ScnPhysicsFromBullet( Point.getPositionWorldOnB() );

					ContactPointB.PointA_ = ScnPhysicsFromBullet( Point.getPositionWorldOnB() );
					ContactPointB.PointB_ = ScnPhysicsFromBullet( Point.getPositionWorldOnA() );
				}

				if( EventA.NoofContactPoints_ > 0 )
				{
					World->Collisions_.push_back( EventA );
					World->Collisions_.push_back( EventB );
				}				
			}

			// Publish collision events.
			for( auto& Collision : World->Collisions_ )
			{
				auto Entity = Collision.BodyA_->getParentEntity();
				Entity->publish( (EvtID)ScnPhysicsEvents::COLLISION, Collision );
			}

			// Clear gathered collisions.
			World->Collisions_.clear();

		}, this, false );

	btGImpactCollisionAlgorithm::registerAlgorithm( Dispatcher_ );




#if !PLATFORM_HTML5
	if( DsCore::pImpl() )
	{
		DebugRenderingHandle_ = DsCore::pImpl()->registerFunction("Toggle Debug Physics Rendering",
			[ this ]
			{
				DebugDrawWorld_ = !DebugDrawWorld_;
			});
	}
#endif

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnPhysicsWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
	DynamicsWorld_->removeAction( UpdateActions_ );

#if !PLATFORM_HTML5
	if( DsCore::pImpl() )
	{
		DsCore::pImpl()->deregisterFunction( DebugRenderingHandle_ );
	}
#endif
	
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// addRigidBody
void ScnPhysicsWorldComponent::addRigidBody( btRigidBody* RigidBody )
{
	BcAssert( DynamicsWorld_ != nullptr );
	BcAssert( RigidBody != nullptr );
	DynamicsWorld_->addRigidBody( RigidBody );
}

//////////////////////////////////////////////////////////////////////////
// removeRigidBody
void ScnPhysicsWorldComponent::removeRigidBody( btRigidBody* RigidBody )
{
	BcAssert( DynamicsWorld_ != nullptr );
	BcAssert( RigidBody != nullptr );
	DynamicsWorld_->removeRigidBody( RigidBody );
}

//////////////////////////////////////////////////////////////////////////
// addConstraint
void ScnPhysicsWorldComponent::addConstraint( class btTypedConstraint* Constraint )
{
	BcAssert( DynamicsWorld_ != nullptr );
	BcAssert( Constraint != nullptr );
	DynamicsWorld_->addConstraint( Constraint );
}

//////////////////////////////////////////////////////////////////////////
// removeConstraint
void ScnPhysicsWorldComponent::removeConstraint( class btTypedConstraint* Constraint )
{
	BcAssert( DynamicsWorld_ != nullptr );
	BcAssert( Constraint != nullptr );
	DynamicsWorld_->removeConstraint( Constraint );
}

//////////////////////////////////////////////////////////////////////////
// registerWorldUpdateHandler
void ScnPhysicsWorldComponent::registerWorldUpdateHandler( ScnIPhysicsWorldUpdate* Handler )
{
	WorldUpdateHandler_.push_back( Handler );
}

//////////////////////////////////////////////////////////////////////////
// deregisterWorldUpdateHandler
void ScnPhysicsWorldComponent::deregisterWorldUpdateHandler( ScnIPhysicsWorldUpdate* Handler )
{
	std::remove( WorldUpdateHandler_.begin(), WorldUpdateHandler_.end(), Handler );
	BcAssert( std::find( WorldUpdateHandler_.begin(), WorldUpdateHandler_.end(), Handler ) == WorldUpdateHandler_.end() );
}

//////////////////////////////////////////////////////////////////////////
// lineCast
BcBool ScnPhysicsWorldComponent::lineCast( const MaVec3d& A, const MaVec3d& B, ScnPhysicsLineCastResult* Result )
{
	btCollisionWorld::ClosestRayResultCallback HitResult( 
		btVector3( A.x(), A.y(), A.z() ),
		btVector3( B.x(), B.y(), B.z() ) );
	DynamicsWorld_->rayTest( 
		btVector3( A.x(), A.y(), A.z() ),
		btVector3( B.x(), B.y(), B.z() ),
		HitResult );

	if( HitResult.hasHit() )
	{
		if( Result != nullptr )
		{
			Result->Intersection_ = MaVec3d( 
				HitResult.m_hitPointWorld.x(),
				HitResult.m_hitPointWorld.y(),
				HitResult.m_hitPointWorld.z() );
			Result->Normal_ = MaVec3d( 
				HitResult.m_hitNormalWorld.x(),
				HitResult.m_hitNormalWorld.y(),
				HitResult.m_hitNormalWorld.z() );
			auto RB = static_cast< ScnPhysicsRigidBodyComponent* >( HitResult.m_collisionObject->getUserPointer() );
			Result->Entity_ = RB->getParentEntity();
		}

		return BcTrue;
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// sphereCast
BcBool ScnPhysicsWorldComponent::sphereCast( const MaVec3d& A, const MaVec3d& B, BcF32 Radius, ScnPhysicsLineCastResult* Result )
{
	std::unique_ptr< btConvexShape > Sphere( new btSphereShape( Radius ) );

	btTransform To( btQuaternion::getIdentity(), ScnPhysicsToBullet( A ) );
	btTransform From( btQuaternion::getIdentity(), ScnPhysicsToBullet( B ) );

	btCollisionWorld::ClosestConvexResultCallback HitResult( 
		ScnPhysicsToBullet( A ),
		ScnPhysicsToBullet( B ) );
	DynamicsWorld_->convexSweepTest( 
		Sphere.get(),
		To,
		From,
		HitResult );

	if( HitResult.hasHit() )
	{
		if( Result != nullptr )
		{
			Result->Intersection_ = MaVec3d( 
				HitResult.m_hitPointWorld.x(),
				HitResult.m_hitPointWorld.y(),
				HitResult.m_hitPointWorld.z() );
			Result->Normal_ = MaVec3d( 
				HitResult.m_hitNormalWorld.x(),
				HitResult.m_hitNormalWorld.y(),
				HitResult.m_hitNormalWorld.z() );
			auto RB = static_cast< ScnPhysicsRigidBodyComponent* >( HitResult.m_hitCollisionObject->getUserPointer() );
			Result->Entity_ = RB->getParentEntity();
		}

		return BcTrue;
	}
	return BcFalse;
}
