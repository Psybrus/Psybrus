/**************************************************************************
*
* File:		GaPlayerComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Low level Boid.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPhysicsBody_H__
#define __GaPhysicsBody_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// GaPhysicsBody
class GaPhysicsBody
{
public:
	GaPhysicsBody( const BcVec2d& Position, BcReal MaxVelocity, BcReal MaxAcceleration, BcReal WanderAmount = 0.0f );
	~GaPhysicsBody();

	// Reset.
	void reset();

	// Set target.
	void target( const BcVec2d& TargetPosition, BcReal ArriveRadius, BcReal Multiplier = 1.0f );

	// Set avoid.
	void avoid( const BcVec2d& AvoidPosition, BcReal AvoidRadius, BcReal Multiplier = 1.0f );

	// Set wander.
	void wander( BcReal Amount );

	// Accelerate.
	void accelerate( const BcVec2d& Amount );

	// Enclose.
	void enclose( const BcVec2d& Min, const BcVec2d& Max, BcReal RepelDistance, BcReal Multiplier );

	// Update.
	const BcVec2d& update( BcReal Tick );

	// Clamp.
	void clamp();

	// Kill.
	void kill();

	// Is live?
	BcBool isLive() const;

public:
	BcVec2d Position_;
	BcVec2d Velocity_;
	BcVec2d Acceleration_;

	BcReal MaxVelocity_;
	BcReal MaxAcceleration_;
	
	BcReal WanderTicker_;
	BcReal WanderAmount_;

	BcBool IsLive_;
};

#endif
