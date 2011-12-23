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
GaPhysicsBody::GaPhysicsBody( const BcVec2d& Position, BcReal MaxVelocity, BcReal MaxAcceleration, BcReal WanderAmount ):
	Position_( Position ),
	Velocity_( 0.0f, 0.0f ),
	Acceleration_( 0.0f, 0.0f ),
	MaxVelocity_( MaxVelocity ),
	MaxAcceleration_( MaxAcceleration ),
	WanderTicker_( (BcReal)BcRandom::Global.randRange( 0, 1024 ) ),
	WanderAmount_( WanderAmount ),
	IsLive_( BcTrue )
{
	
}

////////////////////////////////////////////////////////////////////////////////
// GaPhysicsBody
GaPhysicsBody::~GaPhysicsBody()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// reset
void GaPhysicsBody::reset()
{
	Acceleration_ = BcVec2d( 0.0f, 0.0f );
}

////////////////////////////////////////////////////////////////////////////////
// target
void GaPhysicsBody::target( const BcVec2d& TargetPosition, BcReal ArriveRadius, BcReal Multiplier )
{
	BcReal Distance = ( TargetPosition - Position_ ).magnitude();

	BcVec2d VelSquaredKeepSign = BcVec2d( BcSquaredKeepSign( Velocity_.x() ), BcSquaredKeepSign( Velocity_.y() ) );
	BcVec2d ArriveAcceleration = Distance > 0.0f ? -( VelSquaredKeepSign ) / ( 2.0f * Distance ) : BcVec2d( 0.0f, 0.0f );
	BcVec2d SeekAcceleration = ( TargetPosition - Position_ ).normal() * MaxAcceleration_;

	BcReal LerpDelta = BcMin( Distance / ArriveRadius, 1.0f );

	BcVec2d Apply;
	Apply.lerp( ArriveAcceleration, SeekAcceleration, LerpDelta );

	accelerate( Apply * Multiplier );
}

////////////////////////////////////////////////////////////////////////////////
// avoid
void GaPhysicsBody::avoid( const BcVec2d& AvoidPosition, BcReal AvoidRadius, BcReal Multiplier )
{
	BcReal Distance = ( AvoidPosition - Position_ ).magnitude();

	BcVec2d VelSquaredKeepSign = BcVec2d( BcSquaredKeepSign( Velocity_.x() ), BcSquaredKeepSign( Velocity_.y() ) );
	BcVec2d AvoidAcceleration = ( Position_ - AvoidPosition ).normal() * MaxAcceleration_;
	BcVec2d OtherAcceleration = BcVec2d( 0.0f, 0.0f );

	BcReal LerpDelta = BcMin( Distance / AvoidRadius, 1.0f );

	BcVec2d Apply;
	Apply.lerp( AvoidAcceleration, OtherAcceleration, LerpDelta );
	accelerate( Apply * Multiplier );
}

////////////////////////////////////////////////////////////////////////////////
// wander
void GaPhysicsBody::wander( BcReal Amount )
{
	WanderAmount_ = Amount;
}

////////////////////////////////////////////////////////////////////////////////
// accelerate
void GaPhysicsBody::accelerate( const BcVec2d& Amount )
{
	Acceleration_ += Amount;

	// Check.
	BcAssert( BcCheckFloat( Position_ ) );
	BcAssert( BcCheckFloat( Velocity_ ) );
	BcAssert( BcCheckFloat( Acceleration_ ) );

	BcAssert( ( Velocity_.magnitude() * 8.0f / 60.0f ) < 100.0f );
}

////////////////////////////////////////////////////////////////////////////////
// enclose
void GaPhysicsBody::enclose( const BcVec2d& Min, const BcVec2d& Max, BcReal RepelDistance, BcReal Multiplier )
{
	avoid( BcVec2d( Min.x(), Position_.y() ), RepelDistance, Multiplier );
	avoid( BcVec2d( Max.x(), Position_.y() ), RepelDistance, Multiplier );
	avoid( BcVec2d( Position_.x(), Min.y() ), RepelDistance, Multiplier );
	avoid( BcVec2d( Position_.x(), Max.y() ), RepelDistance, Multiplier );

	// Check.
	BcAssert( BcCheckFloat( Position_ ) );
	BcAssert( BcCheckFloat( Velocity_ ) );
	BcAssert( BcCheckFloat( Acceleration_ ) );
}

////////////////////////////////////////////////////////////////////////////////
// update
const BcVec2d& GaPhysicsBody::update( BcReal Tick )
{
	// Check.
	BcAssert( BcCheckFloat( Position_ ) );
	BcAssert( BcCheckFloat( Velocity_ ) );
	BcAssert( BcCheckFloat( Acceleration_ ) );

	// Apply wander.
	WanderTicker_ += Tick;

	if( WanderTicker_ >= 1024.0f )
	{
		WanderTicker_ -= 1024.0f;
	}

	BcReal WanderX = BcRandom::Global.interpolatedNoise( WanderTicker_, 512 );
	BcReal WanderY = BcRandom::Global.interpolatedNoise( WanderTicker_ + 256, 1024 );

	Acceleration_ += BcVec2d( WanderX, WanderY ).normal() * WanderAmount_;

	// Clamp to max before updating.
	clamp();

	BcVec2d Position = Position_;

	// Update physics.
	Position_ += Velocity_ * Tick;
	Velocity_ += Acceleration_ * Tick;

	// ARGH.
	BcAssert( Tick < 8.0f / 60.0f );
	BcAssert( ( Position_ - Position ).magnitude() < 100.0f );
	BcAssert( ( Velocity_.magnitude() * 8.0f / 60.0f ) < 100.0f );

	// 
	return Position_;
}

////////////////////////////////////////////////////////////////////////////////
// clamp
void GaPhysicsBody::clamp()
{
	BcReal VelocityMag = Velocity_.magnitude();
	BcReal AccelerationMag = Acceleration_.magnitude();
	Velocity_ = VelocityMag > 0.0f ? ( Velocity_ / VelocityMag ) * BcMin( VelocityMag, MaxVelocity_ ) : BcVec2d( 0.0f, 0.0f );
	Acceleration_ = AccelerationMag > 0.0f ? ( Acceleration_ / AccelerationMag ) * BcMin( AccelerationMag, MaxAcceleration_ ) : BcVec2d( 0.0f, 0.0f );

	// Check.
	BcAssert( BcCheckFloat( Position_ ) );
	BcAssert( BcCheckFloat( Velocity_ ) );
	BcAssert( BcCheckFloat( Acceleration_ ) );
}

////////////////////////////////////////////////////////////////////////////////
// kill
void GaPhysicsBody::kill()
{
	IsLive_ = BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// isLive
BcBool GaPhysicsBody::isLive() const
{
	return IsLive_;
}
