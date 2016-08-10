/**************************************************************************
*
* File:		ScnPhysicsWorldComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsWorldComponent_H__
#define __ScnPhysicsWorldComponent_H__

#include "System/Scene/ScnComponent.h"
#include "System/Scene/Physics/ScnPhysicsEvents.h"

//////////////////////////////////////////////////////////////////////////
// ScnIPhysicsWorldUpdate
class ScnIPhysicsWorldUpdate
{
public:
	ScnIPhysicsWorldUpdate() {};
	virtual ~ScnIPhysicsWorldUpdate() {};

	/**
	 * Called for each physics update tick.
	 * Can be called multiple times per frame.
	 */
	virtual void onPhysicsUpdate( BcF32 Tick ) = 0;
};

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsLineCastResult
struct ScnPhysicsLineCastResult
{
	MaVec3d Intersection_;
	MaVec3d Normal_;
	class ScnEntity* Entity_;
};

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsWorldComponent
class ScnPhysicsWorldComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsWorldComponent, ScnComponent );
	
	ScnPhysicsWorldComponent();
	virtual ~ScnPhysicsWorldComponent();

	void initialise() override;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void addRigidBody( class btRigidBody* RigidBody, BcU16 CollisionGroup, BcU16 CollisionMask );
	void removeRigidBody( class btRigidBody* RigidBody );

	void addConstraint( class btTypedConstraint* Constraint );
	void removeConstraint( class btTypedConstraint* Constraint );

	void registerWorldUpdateHandler( ScnIPhysicsWorldUpdate* Handler );
	void deregisterWorldUpdateHandler( ScnIPhysicsWorldUpdate* Handler );

	BcBool lineCast( const MaVec3d& A, const MaVec3d& B, BcU16 CollisionMask, ScnPhysicsLineCastResult* Result );
	BcBool sphereCast( const MaVec3d& A, const MaVec3d& B, BcF32 Radius, BcU16 CollisionMask, ScnPhysicsLineCastResult* Result );

private:
	static void simulate( const ScnComponentList& Components );
	static void debugDraw( const ScnComponentList& Components );

private:
	MaVec3d Gravity_;
	BcU32 MaxSubSteps_;
	BcF32 FrameRate_;
	BcF32 InvFrameRate_;
	BcBool DebugDrawWorld_;

	class btBroadphaseInterface* Broadphase_;
	class btCollisionDispatcher* Dispatcher_;
	class btConstraintSolver* Solver_;
	class btDefaultCollisionConfiguration* CollisionConfiguration_;
	class btDynamicsWorld* DynamicsWorld_;

	friend class UpdateActions;
	class UpdateActions* UpdateActions_;
	std::vector< ScnIPhysicsWorldUpdate* > WorldUpdateHandler_;

	std::vector< ScnPhysicsEventCollision > Collisions_;
	std::vector< ScnPhysicsEventTrigger > Triggers_;

	BcU32 DebugRenderingHandle_;
};

#endif
