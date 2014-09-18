/**************************************************************************
*
* File:		ScnPhysicsRigidBodyComponent.h
* Author:	Neil Richardson 
* Ver/Date:	20/02/13	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsRigidBodyComponent_H__
#define __ScnPhysicsRigidBodyComponent_H__

#include "System/Scene/ScnComponent.h"
#include "System/Scene/Physics/ScnPhysicsCollisionShape.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsRigidBodyComponent
class ScnPhysicsRigidBodyComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnPhysicsRigidBodyComponent, ScnComponent );
	

	virtual void							initialise();
	virtual void							create();
	virtual void							destroy();

	virtual void							update( BcF32 Tick );

	void									onAttach( ScnEntityWeakRef Parent );
	void									onDetach( ScnEntityWeakRef Parent );

private:
	class ScnPhysicsWorldComponent*			World_;
	class ScnPhysicsCollisionShape*			CollisionShape_;

	class btRigidBody*						RigidBody_;

	BcF32									Mass_;

	// TODO: Physics material.
	BcF32									Friction_;
	BcF32									Restitution_;
};

#endif
