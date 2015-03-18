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

	btGImpactCollisionAlgorithm::registerAlgorithm( Dispatcher_ );

#if defined( PSY_DEBUG )
	DebugDrawWorld_ = BcTrue;
#endif

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
BcBool ScnPhysicsWorldComponent::lineCast( const MaVec3d& A, const MaVec3d& B, MaVec3d& Intersection, MaVec3d& Normal )
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
		Intersection = MaVec3d( 
			HitResult.m_hitPointWorld.x(),
			HitResult.m_hitPointWorld.y(),
			HitResult.m_hitPointWorld.z() );
		Normal = MaVec3d( 
			HitResult.m_hitNormalWorld.x(),
			HitResult.m_hitNormalWorld.y(),
			HitResult.m_hitNormalWorld.z() );
		return BcTrue;
	}
	return BcFalse;
}
