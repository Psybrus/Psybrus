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

	void preUpdate( BcF32 Tick ) override;
	void update( BcF32 Tick ) override;
	void postUpdate( BcF32 Tick ) override;

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	void addRigidBody( class btRigidBody* RigidBody );
	void removeRigidBody( class btRigidBody* RigidBody );

	BcBool lineCast( const MaVec3d& A, const MaVec3d& B, MaVec3d& Intersection, MaVec3d& Normal );


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

	BcU32 DebugRenderingHandle_;
};

#endif
