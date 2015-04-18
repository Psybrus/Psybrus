/**************************************************************************
*
* File:		ScnPhysicsRigidBodyComponent.h
* Author:	Neil Richardson 
* Ver/Date:	
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

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsRigidBodyComponent
class ScnPhysicsRigidBodyComponent:
	public ScnComponent
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsRigidBodyComponent, ScnComponent );
	
	ScnPhysicsRigidBodyComponent();
	virtual ~ScnPhysicsRigidBodyComponent();

	void applyTorque( const MaVec3d& Torque );
	void applyForce( const MaVec3d& Force, const MaVec3d& RelativePos );
	void applyCentralForce( const MaVec3d& Force );

	void applyTorqueImpulse( const MaVec3d& Torque );
	void applyImpulse( const MaVec3d& Impulse, const MaVec3d& RelativePos );
	void applyCentralImpulse( const MaVec3d& Impulse );

	void setLinearVelocity( const MaVec3d& Velocity );
	void setAngularVelocity( const MaVec3d& Velocity );

	void translate( const MaVec3d& V );

	MaVec3d getLinearVelocity() const;
	MaVec3d getAngularVelocity() const;

	MaVec3d getPosition() const;

	BcF32 getMass() const;

	virtual void update( BcF32 Tick );

	void onAttach( ScnEntityWeakRef Parent );
	void onDetach( ScnEntityWeakRef Parent );

	class btRigidBody* getRigidBody();

private:
	class ScnPhysicsWorldComponent* World_;
	class ScnPhysicsCollisionComponent* CollisionComponent_;

	class btRigidBody* RigidBody_;

	BcF32 Mass_;

	// TODO: Physics material.
	BcF32 Friction_;
	BcF32 RollingFriction_;
	BcF32 Restitution_;
	BcF32 LinearSleepingThreshold_;
	BcF32 AngularSleepingThreshold_;
};

#endif
