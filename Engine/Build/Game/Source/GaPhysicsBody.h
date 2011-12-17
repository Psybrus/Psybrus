/**************************************************************************
*
* File:		GaPlayerEntity.h
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
	GaPhysicsBody( const BcVec2d& Position, BcReal MaxVelocity, BcReal MaxAcceleration );
	~GaPhysicsBody();

	// Set target.
	void target( BcVec2d TargetPosition, BcReal ArriveRadius );

	// Update.
	const BcVec2d& update( BcReal Tick );

public:
	BcVec2d Position_;
	BcVec2d Velocity_;
	BcVec2d Acceleration_;

	BcReal MaxVelocity_;
	BcReal MaxAcceleration_;
};

#endif
