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

#include "Math/MaQuat.h"

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
	void applyLocalTorque( const MaVec3d& Torque );
	void applyForce( const MaVec3d& Force, const MaVec3d& RelativePos );
	void applyCentralForce( const MaVec3d& Force );

	void applyTorqueImpulse( const MaVec3d& Torque );
	void applyLocalTorqueImpulse( const MaVec3d& Torque );
	void applyImpulse( const MaVec3d& Impulse, const MaVec3d& RelativePos );
	void applyCentralImpulse( const MaVec3d& Impulse );

	void setLinearVelocity( const MaVec3d& Velocity );
	MaVec3d getLinearVelocity() const;
	void setAngularVelocity( const MaVec3d& Velocity );
	MaVec3d getAngularVelocity() const;

	MaVec3d getVelocityInLocalPoint( const MaVec3d& LocalPoint );

	void setMass( BcF32 Mass );
	BcF32 getMass() const;

	void translate( const MaVec3d& V );
	MaVec3d getPosition() const;
	MaQuat getRotation() const;  

	bool isTrigger() const { return !!IsTrigger_; }

	void setTransform( const MaMat4d& NewTransform );

	void onAttach( ScnEntityWeakRef Parent ) override;
	void onDetach( ScnEntityWeakRef Parent ) override;

	class btRigidBody* getRigidBody();

private:
	class ScnPhysicsWorldComponent* World_;
	class ScnPhysicsCollisionComponent* CollisionComponent_;

	class btRigidBody* RigidBody_;

	BcF32 Mass_;
	BcBool IsTrigger_;

	/// What collision group we belong to.
	BcU16 CollisionGroup_ = BcU16( 1 );

	/// What collision groups we collide with.
	BcU16 CollisionMask_ = BcU16( -1 );

	// TODO: Physics material.
	BcF32 Friction_;
	BcF32 RollingFriction_;
	BcF32 Restitution_;
	BcF32 LinearSleepingThreshold_;
	BcF32 AngularSleepingThreshold_;
};

#endif
