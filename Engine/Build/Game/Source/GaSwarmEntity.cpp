/**************************************************************************
*
* File:		GaSwarmEntity.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaSwarmEntity.h"
#include "GaPlayerEntity.h"

#include "GaMainGameState.h"

////////////////////////////////////////////////////////////////////////////////
// Swarm behaviour.
const BcReal TARGET_POSITION_ARRIVE_RADIUS = 256.0f;
const BcReal TARGET_POSITION_MULTIPLIER = 1.0f;
const BcReal SWARM_AVOID_DISTANCE = 16.0f;
const BcReal SWARM_AVOID_MULTIPLIER = 1.0f;
const BcReal FLOCK_VELOCITY_MULTIPLIER = 1.0f;
const BcReal PLAYER_AVOID_DISTANCE = 128.0f;
const BcReal PLAYER_AVOID_MULTIPLIER = 32.0f;
const BcReal SWARM_BODY_DEATH_DISTANCE = 128.0f;

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaSwarmEntity::GaSwarmEntity( const BcMat4d& Projection )
{
	Projection_ = Projection;
	Position_ = BcVec2d( 0.0f, 0.0f );

	for( BcU32 Idx = 0; Idx < 16; ++Idx )
	{
		BcVec2d Position( BcVec2d( BcRandom::Global.randReal(), BcRandom::Global.randReal() ).normal() * 32.0f );
		Bodies_.push_back( new GaPhysicsBody( Position, 64.0f, 128.0f, 16.0f ) );
	}

	// Bind input events.
	//OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseMove >( this );
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaSwarmEntity, &GaSwarmEntity::onMouseDown >( this );
	//OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseUp >( this );

	//OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	//OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaSwarmEntity::~GaSwarmEntity()
{
	// Unbind all events.
	OsCore::pImpl()->unsubscribeAll( this );
}

////////////////////////////////////////////////////////////////////////////////
// findNearestBody
GaPhysicsBody* GaSwarmEntity::findNearestBody( GaPhysicsBody* pSource )
{
	GaPhysicsBody* pNearest = NULL;
	BcReal NearestDistanceSquared = 1e16f;

	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];
		
		if( pBody->isLive() && pBody != pSource )
		{
			BcReal DistanceSquared = ( pBody->Position_ - pSource->Position_ ).magnitudeSquared();

			if( DistanceSquared < NearestDistanceSquared )
			{
				NearestDistanceSquared = DistanceSquared;
				pNearest = pBody;
			}
		}
	}

	return pNearest;
}

////////////////////////////////////////////////////////////////////////////////
// averageVelocity
BcVec2d GaSwarmEntity::averageVelocity() const
{
	BcVec2d TotalVelocity( 0.0f, 0.0f );

	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];
		
		if( pBody->isLive() )
		{
			TotalVelocity += pBody->Velocity_;
		}
	}

	return TotalVelocity / (BcReal)Bodies_.size();
}

////////////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaSwarmEntity::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	if( Event.ButtonCode_ == 1 )
	{
		BcVec2d HalfResolution = BcVec2d( GResolutionWidth / 2, GResolutionHeight / 2 );
		BcVec2d CursorPosition = BcVec2d( Event.MouseX_, GResolutionHeight - Event.MouseY_ );
		BcVec2d ScreenPosition = ( CursorPosition - HalfResolution ) / HalfResolution;

		BcMat4d InverseProjection( Projection_ );
		InverseProjection.inverse();

		Position_ = ScreenPosition * InverseProjection;
	}

	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// update
void GaSwarmEntity::update( BcReal Tick )
{
	BcVec2d AverageVelocity = averageVelocity();

	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];

		if( pBody->isLive() )
		{
			// Set target position.
			pBody->target( Position_, TARGET_POSITION_ARRIVE_RADIUS, TARGET_POSITION_MULTIPLIER );

			// Find nearest body, and avoid.
			GaPhysicsBody* pOther = findNearestBody( pBody );
			if( pOther != NULL )
			{
				pBody->avoid( pOther->Position_, SWARM_AVOID_DISTANCE, SWARM_AVOID_MULTIPLIER );

				// If body is too far from nearest, kill it.
				BcReal NearestDistance = ( pBody->Position_ - pOther->Position_ ).magnitude();
				if( NearestDistance > SWARM_BODY_DEATH_DISTANCE )
				{
					pBody->kill();
				}
			} 
			else
			{
				pBody->kill();
			}

			// Appear like flocking by tending towards the average direction.
			pBody->accelerate( AverageVelocity * FLOCK_VELOCITY_MULTIPLIER );

			// Avoid player massively.
			GaPlayerEntity* pPlayerEntity = pParent()->getEntity< GaPlayerEntity >( 0 );
			pBody->avoid( pPlayerEntity->getPosition(), PLAYER_AVOID_DISTANCE, PLAYER_AVOID_MULTIPLIER );

			// Enclose in the play area.
			pBody->enclose( BcVec2d( -320.0f, -240.0f ), BcVec2d( 320.0f, 240.0f ), 32.0f, 64.0f );

			// Update.
			pBody->update( Tick );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaSwarmEntity::render( ScnCanvasRef Canvas )
{
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];

		if( pBody->isLive() )
		{
			GaPhysicsBody* pOther = findNearestBody( pBody );
			BcReal ColourLerp = pOther != NULL ? ( pBody->Position_ - pOther->Position_ ).magnitude() / SWARM_BODY_DEATH_DISTANCE : 1.0f;
			
			RsColour Colour;
			Colour.lerp( RsColour::GREEN, RsColour::RED, ColourLerp );

			BcVec2d Position = pBody->Position_;
			BcVec2d Size( 8.0f, 8.0f );
			Canvas->drawSpriteCentered( Position, Size, 0, Colour, 0 );
	
			// DEBUG DATA.
			//Canvas->drawLine( Position, Position + pBody->Velocity_, RsColour::WHITE, 1 );
			//Canvas->drawLine( Position, Position + pBody->Acceleration_, RsColour::RED, 1 );
		}
	}
}
