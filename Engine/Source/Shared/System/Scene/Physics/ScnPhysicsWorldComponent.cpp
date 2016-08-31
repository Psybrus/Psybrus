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
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Debug/DsCore.h"
#include "System/Debug/DsUtils.h"
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

#include "System/SysKernel.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"

#include "LinearMath/btIDebugDraw.h"

#if !PSY_PRODUCTION
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
		if( ScnDebugRenderComponent::pImpl() )
		{
			Debug::DrawCategory Category( CategoryMask_ );
			Debug::DrawLine(
				MaVec3d( from.x(), from.y(), from.z() ),
				MaVec3d( to.x(), to.y(), to.z() ),
				RsColour( color.x(), color.y(), color.z(), 1.0f ) );
		}
	}

	virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
	{
		if( ScnDebugRenderComponent::pImpl() )
		{
			Debug::DrawCategory Category( CategoryMask_ );
			Debug::DrawLine(
				ScnPhysicsFromBullet( PointOnB ) - ScnPhysicsFromBullet( normalOnB ),
				ScnPhysicsFromBullet( PointOnB ) + ScnPhysicsFromBullet( normalOnB ),
				RsColour( color.x(), color.y(), color.z(), 1.0f ) );
		}
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

	int DebugMode_ = 0;
	BcU32 CategoryMask_ = 0;
};

static DebugRenderer gDebugRenderer;
#endif

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

	using namespace std::placeholders;
	ReRegisterClass< ScnPhysicsWorldComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Simulate",
					ScnComponentPriority::PHYSICS_WORLD_SIMULATE,
					std::bind( &ScnPhysicsWorldComponent::simulate, _1 ) ),
				ScnComponentProcessFuncEntry(
					"DebugDraw",
					ScnComponentPriority::PHYSICS_WORLD_DEBUG_DRAW,
					std::bind( &ScnPhysicsWorldComponent::debugDraw, _1 ) )
			} ) );
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
#if !PSY_PRODUCTION
	DynamicsWorld_->setDebugDrawer( &gDebugRenderer );
#endif
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
			
				size_t NumContacts = ContactManifold->getNumContacts();
				if( NumContacts > 0 )
				{
					if( RigidBodyA->isTrigger() || RigidBodyB->isTrigger() )
					{
						ScnPhysicsEventTrigger EventA;
						ScnPhysicsEventTrigger EventB;
						EventA.BodyA_ = RigidBodyA;
						EventA.BodyB_ = RigidBodyB;
						EventB.BodyA_ = RigidBodyB;
						EventB.BodyB_ = RigidBodyA;

						World->Triggers_.push_back( EventA );
						World->Triggers_.push_back( EventB );
					}
					else
					{
						ScnPhysicsEventCollision EventA;
						ScnPhysicsEventCollision EventB;
						EventA.BodyA_ = RigidBodyA;
						EventA.BodyB_ = RigidBodyB;
						EventA.NoofContactPoints_ = 0;
						EventB.BodyA_ = RigidBodyB;
						EventB.BodyB_ = RigidBodyA;
						EventB.NoofContactPoints_ = 0;
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
				}
			}

			// Publish collision events.
			for( auto& Collision : World->Collisions_ )
			{
				auto Entity = Collision.BodyA_->getParentEntity();
				Entity->publish( (EvtID)ScnPhysicsEvents::COLLISION, Collision );
			}

			// Publish trigger events.
			for( auto& Collision : World->Triggers_ )
			{
				auto Entity = Collision.BodyA_->getParentEntity();
				Entity->publish( (EvtID)ScnPhysicsEvents::TRIGGER, Collision );
			}

			// Clear gathered collisions & triggers.
			World->Collisions_.clear();
			World->Triggers_.clear();

		}, this, false );

	btGImpactCollisionAlgorithm::registerAlgorithm( Dispatcher_ );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnPhysicsWorldComponent::onDetach( ScnEntityWeakRef Parent )
{
	DynamicsWorld_->removeAction( UpdateActions_ );
	
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// addRigidBody
void ScnPhysicsWorldComponent::addRigidBody( btRigidBody* RigidBody, BcU16 CollisionGroup, BcU16 CollisionMask )
{
	BcAssert( DynamicsWorld_ != nullptr );
	BcAssert( RigidBody != nullptr );
	DynamicsWorld_->addRigidBody( RigidBody, CollisionGroup, CollisionMask );
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
	auto It = std::find( WorldUpdateHandler_.begin(), WorldUpdateHandler_.end(), Handler );
	BcAssert( It != WorldUpdateHandler_.end() );
	WorldUpdateHandler_.erase( It );	
}

//////////////////////////////////////////////////////////////////////////
// lineCast
BcBool ScnPhysicsWorldComponent::lineCast( const MaVec3d& A, const MaVec3d& B, BcU16 CollisionMask, ScnPhysicsLineCastResult* Result )
{
	btCollisionWorld::ClosestRayResultCallback HitResult( 
		btVector3( A.x(), A.y(), A.z() ),
		btVector3( B.x(), B.y(), B.z() ) );
	HitResult.m_collisionFilterMask = CollisionMask;
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
BcBool ScnPhysicsWorldComponent::sphereCast( const MaVec3d& A, const MaVec3d& B, BcF32 Radius, BcU16 CollisionMask, ScnPhysicsLineCastResult* Result )
{
	std::unique_ptr< btConvexShape > Sphere( new btSphereShape( Radius ) );

	btTransform To( btQuaternion::getIdentity(), ScnPhysicsToBullet( A ) );
	btTransform From( btQuaternion::getIdentity(), ScnPhysicsToBullet( B ) );

	btCollisionWorld::ClosestConvexResultCallback HitResult( 
		ScnPhysicsToBullet( A ),
		ScnPhysicsToBullet( B ) );
	HitResult.m_collisionFilterMask = CollisionMask;
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

//////////////////////////////////////////////////////////////////////////
// simulate
void ScnPhysicsWorldComponent::simulate( const ScnComponentList& Components )
{
	auto Tick = SysKernel::pImpl()->getFrameTime();
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnPhysicsWorldComponent >() );
		auto* WorldComponent = static_cast< ScnPhysicsWorldComponent* >( Component.get() );

		// Setup grabity and other parameters.
		WorldComponent->DynamicsWorld_->setGravity( ScnPhysicsToBullet( WorldComponent->Gravity_ ) );
		
		// Step simulation.
		WorldComponent->DynamicsWorld_->stepSimulation( 
			Tick, 
			WorldComponent->MaxSubSteps_, 
			WorldComponent->InvFrameRate_ );
	}

}

//////////////////////////////////////////////////////////////////////////
// debugDraw
//virtual
void ScnPhysicsWorldComponent::debugDraw( const ScnComponentList& Components )
{
	for( auto Component : Components )
	{
		BcAssert( Component->isTypeOf< ScnPhysicsWorldComponent >() );
		auto* WorldComponent = static_cast< ScnPhysicsWorldComponent* >( Component.get() );

#if !PSY_PRODUCTION
		if( ScnDebugRenderComponent::pImpl() )
		{
			gDebugRenderer.CategoryMask_ = ScnDebugRenderComponent::pImpl()->getCategoryMask( "Physics" );
			WorldComponent->DebugDrawWorld_ = !!( ScnDebugRenderComponent::pImpl()->getDrawCategoryMask() & gDebugRenderer.CategoryMask_ );
		}
#endif
		if( WorldComponent->DebugDrawWorld_ )
		{
			WorldComponent->DynamicsWorld_->debugDrawWorld();
		}
	}
}
