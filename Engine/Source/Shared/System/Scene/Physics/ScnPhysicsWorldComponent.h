/**************************************************************************
*
* File:		ScnPhysicsWorldComponent.h
* Author:	Neil Richardson 
* Ver/Date:	20/02/13	
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
	DECLARE_RESOURCE( ScnComponent, ScnPhysicsWorldComponent );
	

	virtual void initialise();
	virtual void initialise( const Json::Value& Object );
	virtual void create();
	virtual void destroy();

	virtual void preUpdate( BcF32 Tick );
	virtual void update( BcF32 Tick );
	virtual void postUpdate( BcF32 Tick );

	void onAttach( ScnEntityWeakRef Parent );
	void onDetach( ScnEntityWeakRef Parent );

	void addRigidBody( class btRigidBody* RigidBody );
	void removeRigidBody( class btRigidBody* RigidBody );

private:
	class btBroadphaseInterface* Broadphase_;
	class btCollisionDispatcher* Dispatcher_;
	class btConstraintSolver* Solver_;
	class btDefaultCollisionConfiguration* CollisionConfiguration_;
	class btDynamicsWorld* DynamicsWorld_;

	BcBool DebugDrawWorld_;
};

#endif
