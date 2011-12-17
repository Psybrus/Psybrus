/**************************************************************************
*
* File:		GaPhysicsBody.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Low level Boid.
*		
*
*
* 
**************************************************************************/

#include "GaPhysicsBody.h"

////////////////////////////////////////////////////////////////////////////////
// GaPhysicsBody
GaPhysicsBody::GaPhysicsBody( const BcVec2d& Position, BcReal MaxVelocity, BcReal MaxAcceleration ):
	Position_( Position ),
	Velocity_( 0.0f, 0.0f ),
	Acceleration_( 0.0f, 0.0f ),
	MaxVelocity_( MaxVelocity ),
	MaxAcceleration_( MaxAcceleration )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// GaPhysicsBody
GaPhysicsBody::~GaPhysicsBody()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// target
void GaPhysicsBody::target( BcVec2d TargetPosition, BcReal ArriveRadius )
{
	BcReal Distance = ( TargetPosition - Position_ ).magnitude();

	BcVec2d VelSquaredKeepSign = BcVec2d( BcSquaredKeepSign( Velocity_.x() ), BcSquaredKeepSign( Velocity_.y() ) );
	BcVec2d ArriveAcceleration = Distance > 0.0f ? -( VelSquaredKeepSign ) / ( 2.0f * Distance ) : BcVec2d( 0.0f, 0.0f );
	BcVec2d SeekAcceleration = ( TargetPosition - Position_ ).normal() * MaxAcceleration_;

	BcReal LerpDelta = BcMin( Distance / ArriveRadius, 1.0f );

	Acceleration_.lerp( ArriveAcceleration, SeekAcceleration, LerpDelta );
}

////////////////////////////////////////////////////////////////////////////////
// update
const BcVec2d& GaPhysicsBody::update( BcReal Tick )
{
	BcReal VelocityMag = Velocity_.magnitude();
	BcReal AccelerationMag = Acceleration_.magnitude();
	BcVec2d ClampedVelocity = VelocityMag > 0.0f ? ( Velocity_ / VelocityMag ) * BcMin( VelocityMag, MaxVelocity_ ) : BcVec2d( 0.0f, 0.0f );
	BcVec2d ClampedAcceleration = AccelerationMag > 0.0f ? ( Acceleration_ / AccelerationMag ) * BcMin( AccelerationMag, MaxAcceleration_ ) : BcVec2d( 0.0f, 0.0f );

	Position_ += ClampedVelocity * Tick;
	Velocity_ += ClampedAcceleration * Tick;

	{
		BcReal VelocityMag = Velocity_.magnitude();
		BcReal AccelerationMag = Acceleration_.magnitude();
		Velocity_ = VelocityMag > 0.0f ? ( Velocity_ / VelocityMag ) * BcMin( VelocityMag, MaxVelocity_ ) : BcVec2d( 0.0f, 0.0f );
		Acceleration_ = AccelerationMag > 0.0f ? ( Acceleration_ / AccelerationMag ) * BcMin( AccelerationMag, MaxAcceleration_ ) : BcVec2d( 0.0f, 0.0f );
	}

	return Position_;
}
