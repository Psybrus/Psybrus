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
#include "GaFoodEntity.h"

#include "GaMainGameState.h"

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Swarm behaviour.
const BcReal TARGET_POSITION_ARRIVE_RADIUS = 32.0f;
const BcReal TARGET_POSITION_MULTIPLIER = 1.0f;
const BcReal SWARM_AVOID_DISTANCE = 16.0f;
const BcReal SWARM_AVOID_MULTIPLIER = 2.0f;
const BcReal FLOCK_VELOCITY_MULTIPLIER = 1.0f;
const BcReal PLAYER_AVOID_DISTANCE = 96.0f;
const BcReal PLAYER_AVOID_MULTIPLIER = 32.0f;
const BcReal SWARM_BODY_DEATH_DISTANCE = 196.0f;
const BcReal SWARM_FOCUS_ON_FOOD_DISTANCE = 48.0f;
const BcReal SWARM_EAT_FOOD_DISTANCE = 48.0f;
const BcReal SWARM_SLOW_DOWN_AT_FOOD_MULTIPLIER = 16.0f;
const BcReal ENCLOSURE_DISTANCE = 64.0f;
const BcReal ENCLOSURE_MULTIPLIER = 256.0f;
const BcReal SWARM_EAT_VELOCITY = 12.0f;
const BcReal HOP_DISTANCE = 64.0f;
const BcReal HOP_SPEED_MULTIPLIER = 3.0f;
const BcReal HOP_HEIGHT = 48.0f;
const BcReal EMOTE_RUN_AWAY_DISTANCE = 64.0f;

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaSwarmEntity::GaSwarmEntity( BcU32 Level )
{
	Position_ = BcVec2d( 256.0f, 0.0f );
	
	for( BcU32 Idx = 0; Idx < ( Level + 1 ) * 2; ++Idx )
	{
		BcVec2d Position( BcVec2d( BcRandom::Global.randReal(), BcRandom::Global.randReal() ).normal() * 32.0f + Position_ );
		Bodies_.push_back( new GaPhysicsBody( Position, 128.0f, 256.0f, 16.0f ) );
		AnimationLogicList_.push_back( new TAnimationLogic() );

		AnimationLogicList_[ Idx ]->StartPosition_ = Bodies_[ Idx ]->Position_;
		AnimationLogicList_[ Idx ]->EndPosition_ = Bodies_[ Idx ]->Position_;

		ScnMaterialRef Material;
		GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_BUNNY, Material );
		AnimationLogicList_[ Idx ]->BunnyRenderer_.setMaterial( Material, BcVec3d( 0.4f, 0.4f, 0.4f ) );

		CsCore::pImpl()->createResource( "bunnyemitter", AnimationLogicList_[ Idx ]->Emitter_ );

		// Slightly different pitch for all bunnies.
		AnimationLogicList_[ Idx ]->Emitter_->setPitch( BcRandom::Global.randReal() * 0.1f + 1.0f );
	}
	
	// Bind input events.
	//OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseMove >( this );
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaSwarmEntity, &GaSwarmEntity::onMouseDown >( this );
	//OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< GaPlayerEntity, &GaPlayerEntity::onMouseUp >( this );

	//OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	//OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );

	pTargetFoodEntity_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaSwarmEntity::~GaSwarmEntity()
{
	// Unbind all events.
	OsCore::pImpl()->unsubscribeAll( this );

	// Free stuff.
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		// Stop all sounds playing from emitter.
		AnimationLogicList_[ Idx ]->Emitter_->stopAll();

		// Delete bodies and logic.
		delete Bodies_[ Idx ];
		delete AnimationLogicList_[ Idx ];
	}
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
// averagePosition
BcVec2d GaSwarmEntity::averagePosition() const
{
	BcVec2d TotalPosition( 0.0f, 0.0f );

	BcReal Total = 0.0f;
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];

		if( pBody->isLive() )
		{
			TotalPosition += pBody->Position_;
			Total += 1.0f;
		}
	}

	// Missed divide by zero, but also check the total position anyway.
	if( Total > 0 && BcCheckFloat( TotalPosition ) == BcTrue )
	{
		return TotalPosition / Total;
	}

	return BcVec2d( 0.0f, 0.0f );
}

////////////////////////////////////////////////////////////////////////////////
// averageVelocity
BcVec2d GaSwarmEntity::averageVelocity() const
{
	BcVec2d TotalVelocity( 0.0f, 0.0f );
	BcReal Total = 0.0f;
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];
		
		if( pBody->isLive() )
		{
			TotalVelocity += pBody->Velocity_;
			Total += 1.0f;
		}
	}

	// Missed divide by zero, but also check the total position anyway.
	if( Total > 0 && BcCheckFloat( TotalVelocity ) == BcTrue )
	{
		return TotalVelocity / Total;
	}

	return BcVec2d( 0.0f, 0.0f );
}


////////////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaSwarmEntity::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	/* TODO: REMOVE>
	if( Event.ButtonCode_ == 1 )
	{
		BcVec2d HalfResolution = BcVec2d( GResolutionWidth / 2, GResolutionHeight / 2 );
		BcVec2d CursorPosition = BcVec2d( Event.MouseX_, GResolutionHeight - Event.MouseY_ );
		BcVec2d ScreenPosition = ( CursorPosition - HalfResolution ) / HalfResolution;

		BcMat4d InverseProjection( Projection_ );
		InverseProjection.inverse();

		Position_ = ScreenPosition * InverseProjection;
	}
	*/

	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// update
void GaSwarmEntity::update( BcReal Tick )
{
	BcVec2d AverageVelocity = averageVelocity();
	BcVec2d AveragePosition = averagePosition();

	// Find the nearest food entity.
	GaFoodEntity* pFoodEntity = pParent()->getNearestEntity< GaFoodEntity >( AveragePosition );

	// If we have food, set our swarm position to be that entity's.
	if( pFoodEntity != NULL )
	{
		BcReal Distance = ( AveragePosition - pFoodEntity->getPosition() ).magnitude();

		// Check if we're on average close to food, and goto it if we are.
		// If not goto the further food away from player.
		if( Distance < SWARM_FOCUS_ON_FOOD_DISTANCE )
		{		
			Position_ = pFoodEntity->getPosition();
			pTargetFoodEntity_ = pFoodEntity;
		}
		else
		{
			GaPlayerEntity* pPlayerEntity = pParent()->getEntity< GaPlayerEntity >( 0 );
			GaFoodEntity* pFoodEntity = pParent()->getFarthestEntity< GaFoodEntity >( pPlayerEntity->getPosition() );
			Position_ = pFoodEntity->getPosition();
			pTargetFoodEntity_ = pFoodEntity;	
		}
	}

	// Update body jobs.
	BcU32 WorkerMask = 0;
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];
		TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];

		// Enqueue the delegate.
		typedef BcDelegate< void(*)( BcReal, GaFoodEntity*, GaPhysicsBody*, GaSwarmEntity::TAnimationLogic* ) > TDelegate;
		TDelegate Delegate( TDelegate::bind< GaSwarmEntity, &GaSwarmEntity::updateBody_Threaded >( this ) );
		SysKernel::pImpl()->enqueueDelegateJob( WorkerMask, Delegate, Tick, pFoodEntity, pBody, pAnimationLogic );
	}

	// Queue fence on all workers and wait for it to be hit.
	if( WorkerMask != 0 )
	{
		SysFence Fence2;
		Fence2.queue( WorkerMask );
		Fence2.wait();
	}

	// Update animation at the end.
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];
		TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];

		// Do animation.
		if( pAnimationLogic->NotNeeded_ == BcFalse )
		{
			if( shouldStartMoveAnimation( Idx ) )
			{
				startMoveAnimation( Idx, pAnimationLogic->EndPosition_, pBody->Position_ );
			}
			
			// Update animation.
			updateAnimation( Idx, Tick );
		}
	}

	GaEntity::update( Tick );
}

void GaSwarmEntity::updateBody_Threaded( BcReal Tick,  GaFoodEntity* pFoodEntity, GaPhysicsBody* pBody, TAnimationLogic* pAnimationLogic )
{
	BcVec2d AverageVelocity = averageVelocity();
	BcVec2d AveragePosition = averagePosition();

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

				// TODO: NOT IN HERE.
				doEmote( EMOTE_SCARED, BcVec3d( pBody->Position_.x(), pBody->Position_.y(), 64.0f ) );
				ScnSoundRef Sound = GaTopState::pImpl()->getSound( GaTopState::SOUND_SCARED );
				pAnimationLogic->Emitter_->play( Sound );
			}
		} 
		else
		{
			pBody->kill();

			// TODO: NOT IN HERE.
			doEmote( EMOTE_SCARED, BcVec3d( pBody->Position_.x(), pBody->Position_.y(), 64.0f ) );
			ScnSoundRef Sound = GaTopState::pImpl()->getSound( GaTopState::SOUND_SCARED );
			pAnimationLogic->Emitter_->play( Sound );
		}

		// Appear like flocking by tending towards the average direction.
		pBody->accelerate( AverageVelocity * FLOCK_VELOCITY_MULTIPLIER );

		// Slow down at food to eat.
		if( pTargetFoodEntity_ != NULL )
		{
			BcReal Distance = ( pTargetFoodEntity_->getPosition() -  pBody->Position_ ).magnitude();

			if( Distance < SWARM_EAT_FOOD_DISTANCE )
			{
				pBody->accelerate( -pBody->Velocity_ * SWARM_SLOW_DOWN_AT_FOOD_MULTIPLIER );

				// If moving slow enough, we can eat.
				if( pBody->Velocity_.magnitude() < SWARM_EAT_VELOCITY )
				{
					pTargetFoodEntity_->eat( Tick );

					// TODO: NOT IN HERE.
					if( doEmote( EMOTE_EATING, BcVec3d( pBody->Position_.x(), pBody->Position_.y(), 64.0f ) ) )
					{
						ScnSoundRef Sound = GaTopState::pImpl()->getSound( BcRandom::Global.randRange( GaTopState::SOUND_CHEW0, GaTopState::SOUND_CHEW1 ) );
						pAnimationLogic->Emitter_->play( Sound );
					}
				}
			}
		}

		// Avoid player massively.
		GaPlayerEntity* pPlayerEntity = pParent()->getEntity< GaPlayerEntity >( 0 );
		pBody->avoid( pPlayerEntity->getPosition(), PLAYER_AVOID_DISTANCE, PLAYER_AVOID_MULTIPLIER );

		// Enclose in the play area.
		pBody->enclose( BcVec2d( -320.0f, -240.0f ), BcVec2d( 320.0f, 240.0f ), ENCLOSURE_DISTANCE, ENCLOSURE_MULTIPLIER );

		// Update.
		pBody->update( Tick );

		// If the player is close by we want to do the run away..
		if( ( pPlayerEntity->getPosition() - pBody->Position_ ).magnitude() < EMOTE_RUN_AWAY_DISTANCE )
		{
			// TODO: NOT IN HERE.
			doEmote( EMOTE_RUNAWAY, BcVec3d( pBody->Position_.x(), pBody->Position_.y(), 64.0f ) );
		}
	}
	else
	{
		if( pAnimationLogic->NotNeeded_ == BcFalse )
		{
			// Run away!
			if( pBody->Position_.magnitude() < 1024.0f )
			{
				// Keep running away from avg.
				BcVec2d NewTarget = ( pBody->Position_ - AveragePosition ).normal() * 2048.f;
				pBody->target( NewTarget, 256.0f, 256.0 );

				// Update.
				pBody->update( Tick );

				if( pBody->Position_.magnitude() > 512.0f )
				{
					pAnimationLogic->NotNeeded_ = BcTrue;
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaSwarmEntity::render( ScnCanvasComponentRef Canvas )
{
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];
		TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];

		if( pAnimationLogic->NotNeeded_ == BcFalse )
		{
			GaPhysicsBody* pOther = findNearestBody( pBody );
			BcReal ColourLerp = pOther != NULL ? ( pBody->Position_ - pOther->Position_ ).magnitude() / SWARM_BODY_DEATH_DISTANCE : 1.0f;
			RsColour Colour;
			Colour.lerp( RsColour::GREEN, RsColour::RED, ColourLerp );
			
			BcVec3d Position = animationPosition( Idx );

			pAnimationLogic->BunnyRenderer_.render( pParent(), Canvas, Position, pAnimationLogic->AnimationPosition_ < 1.0f ? pBody->Velocity_ : BcVec2d( 0.0f, 0.0f ) );

			pAnimationLogic->Emitter_->setPosition( Position );
			
			/*
			// DEBUG DATA.
			BcVec2d Size( 32.0f, 32.0f );
			Canvas->setMaterialComponent( ScnMaterialComponent::Default );
			Canvas->drawSpriteCentered( pBody->Position_, Size, 0, RsColour::RED * 0.5f, 2 );
			Canvas->drawLine( Position, pBody->Position_, Colour, 0 );*/
		}
	}

	GaEntity::render( Canvas );
}

////////////////////////////////////////////////////////////////////////////////
// updateAnimation
void GaSwarmEntity::updateAnimation( BcU32 Idx, BcReal Tick )
{
	TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];
	
	BcReal TotalTickTime = ( Tick * pAnimationLogic->AnimationSpeed_ * HOP_SPEED_MULTIPLIER );

	BcReal NewAnimationPosition = BcMin( pAnimationLogic->AnimationPosition_ + TotalTickTime, 1.0f );

	if( NewAnimationPosition >= 1.0f && pAnimationLogic->AnimationPosition_ < 1.0f )
	{
		ScnSoundRef Sound = GaTopState::pImpl()->getSound( BcRandom::Global.randRange( GaTopState::SOUND_HOP1, GaTopState::SOUND_HOP4 ) );
		pAnimationLogic->Emitter_->play( Sound );
	}

	pAnimationLogic->AnimationPosition_ = NewAnimationPosition;

	pAnimationLogic->BunnyRenderer_.update( TotalTickTime );
}

////////////////////////////////////////////////////////////////////////////////
// animationPosition
BcVec3d GaSwarmEntity::animationPosition( BcU32 Idx ) const
{
	TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];
	
	BcReal HopPosition = BcSin( BcPI * pAnimationLogic->AnimationPosition_ ) * HOP_HEIGHT;
	BcVec2d LinearPosition;
	LinearPosition.lerp( pAnimationLogic->StartPosition_, pAnimationLogic->EndPosition_,  BcSmoothStep( pAnimationLogic->AnimationPosition_ ) );
	
	return BcVec3d( LinearPosition.x(), LinearPosition.y(), HopPosition );
}

////////////////////////////////////////////////////////////////////////////////
// shouldStartMoveAnimation
BcBool GaSwarmEntity::shouldStartMoveAnimation( BcU32 Idx )
{
	GaPhysicsBody* pBody = Bodies_[ Idx ];
	TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];

	BcReal Distance = ( pAnimationLogic->EndPosition_ - pBody->Position_ ).magnitude();
	
	// If we've moved far enough..
	if( Distance > HOP_DISTANCE )
	{
		// And not currently animating.
		if( pAnimationLogic->AnimationPosition_ >= 1.0f )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// setEnemyAnimation
void GaSwarmEntity::startMoveAnimation( BcU32 Idx, const BcVec2d& Start, const BcVec2d& End )
{
	TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];
	
	pAnimationLogic->StartPosition_ = Start;
	pAnimationLogic->EndPosition_ = End;
	pAnimationLogic->AnimationPosition_ = 0.0f;
	pAnimationLogic->AnimationSpeed_ = BcMax( ( Start - End ).magnitude(), 1.0f );
}

////////////////////////////////////////////////////////////////////////////////
// isAlive
BcBool GaSwarmEntity::isAlive()
{
	for( BcU32 Idx = 0; Idx < Bodies_.size(); ++Idx )
	{
		GaPhysicsBody* pBody = Bodies_[ Idx ];
		if( pBody->isLive() )
		{
			return BcTrue;
		}
	}

	return BcFalse;
}
