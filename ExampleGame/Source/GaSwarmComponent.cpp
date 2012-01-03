/**************************************************************************
*
* File:		GaSwarmComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaSwarmComponent.h"
#include "GaPlayerComponent.h"
#include "GaFoodComponent.h"

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
// Define resource.
DEFINE_RESOURCE( GaSwarmComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaSwarmComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaSwarmComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaSwarmComponent::initialise( BcU32 Level )
{
	Super::initialise();

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

		// Create layered sprite component.
		CsCore::pImpl()->createResource( BcName::INVALID, AnimationLogicList_[ Idx ]->LayeredSpriteComponent_, Material, BcVec3d( 0.4f, 0.4f, 0.4f ) );
		CsCore::pImpl()->createResource( BcName::INVALID, AnimationLogicList_[ Idx ]->Emitter_ );

		// Slightly different pitch for all bunnies.
		AnimationLogicList_[ Idx ]->Emitter_->setPitch( BcRandom::Global.randReal() * 0.1f + 1.0f );
	}
	
	// Bind input events.
	//OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onMouseMove >( this );
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaSwarmComponent, &GaSwarmComponent::onMouseDown >( this );
	//OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< GaPlayerComponent, &GaPlayerComponent::onMouseUp >( this );

	//OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	//OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );

	pTargetFoodEntity_ = NULL;
}

////////////////////////////////////////////////////////////////////////////////
// destory
//virtual
void GaSwarmComponent::destroy()
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
GaPhysicsBody* GaSwarmComponent::findNearestBody( GaPhysicsBody* pSource )
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
BcVec2d GaSwarmComponent::averagePosition() const
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
BcVec2d GaSwarmComponent::averageVelocity() const
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
eEvtReturn GaSwarmComponent::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
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
void GaSwarmComponent::update( BcReal Tick )
{
	Super::update( Tick );

	BcVec2d AverageVelocity = averageVelocity();
	BcVec2d AveragePosition = averagePosition();

	// Find the nearest food entity.
	GaFoodComponent* pFoodEntity = pParent()->getNearestEntity< GaFoodComponent >( AveragePosition );

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
			GaPlayerComponent* pPlayerEntity = pParent()->getEntity< GaPlayerComponent >( 0 );
			GaFoodComponent* pFoodEntity = pParent()->getFarthestEntity< GaFoodComponent >( pPlayerEntity->getPosition() );
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
		typedef BcDelegate< void(*)( BcReal, GaFoodComponent*, GaPhysicsBody*, GaSwarmComponent::TAnimationLogic* ) > TDelegate;
		TDelegate Delegate( TDelegate::bind< GaSwarmComponent, &GaSwarmComponent::updateBody_Threaded >( this ) );
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

	GaGameComponent::update( Tick );
}

void GaSwarmComponent::updateBody_Threaded( BcReal Tick,  GaFoodComponent* pFoodEntity, GaPhysicsBody* pBody, TAnimationLogic* pAnimationLogic )
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
		GaPlayerComponent* pPlayerEntity = pParent()->getEntity< GaPlayerComponent >( 0 );
		if( pPlayerEntity != NULL )
		{
			pBody->avoid( pPlayerEntity->getPosition(), PLAYER_AVOID_DISTANCE, PLAYER_AVOID_MULTIPLIER );
		}
		else
		{
			int a = 0; ++a;
		}

		// Enclose in the play area.
		pBody->enclose( BcVec2d( -320.0f, -240.0f ), BcVec2d( 320.0f, 240.0f ), ENCLOSURE_DISTANCE, ENCLOSURE_MULTIPLIER );

		// Update.
		pBody->update( Tick );

		// If the player is close by we want to do the run away..
		if( pPlayerEntity != NULL )
		{
			if( ( pPlayerEntity->getPosition() - pBody->Position_ ).magnitude() < EMOTE_RUN_AWAY_DISTANCE )
			{
				// TODO: NOT IN HERE.
				doEmote( EMOTE_RUNAWAY, BcVec3d( pBody->Position_.x(), pBody->Position_.y(), 64.0f ) );
			}
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
void GaSwarmComponent::render( ScnCanvasComponentRef Canvas )
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

			pAnimationLogic->LayeredSpriteComponent_->render( pParent(), Canvas, Position, pAnimationLogic->AnimationPosition_ < 1.0f ? pBody->Velocity_ : BcVec2d( 0.0f, 0.0f ) );

			pAnimationLogic->Emitter_->setPosition( Position );
			
			/*
			// DEBUG DATA.
			BcVec2d Size( 32.0f, 32.0f );
			Canvas->setMaterialComponent( ScnMaterialComponent::Default );
			Canvas->drawSpriteCentered( pBody->Position_, Size, 0, RsColour::RED * 0.5f, 2 );
			Canvas->drawLine( Position, pBody->Position_, Colour, 0 );*/
		}
	}

	GaGameComponent::render( Canvas );
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaSwarmComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	// Attach all swarm thingies.
	for( BcU32 Idx = 0; Idx < AnimationLogicList_.size(); ++Idx )
	{
		TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];

		Parent->attach( pAnimationLogic->LayeredSpriteComponent_ );
		//Parent->attach( pAnimationLogic->Emitter_ );
	}
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaSwarmComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	// Detach all swarm thingies.
	for( BcU32 Idx = 0; Idx < AnimationLogicList_.size(); ++Idx )
	{
		TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];

		Parent->detach( pAnimationLogic->LayeredSpriteComponent_ );
		//Parent->detach( pAnimationLogic->Emitter_ );
	}
}

////////////////////////////////////////////////////////////////////////////////
// updateAnimation
void GaSwarmComponent::updateAnimation( BcU32 Idx, BcReal Tick )
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

	pAnimationLogic->LayeredSpriteComponent_->update( TotalTickTime );
}

////////////////////////////////////////////////////////////////////////////////
// animationPosition
BcVec3d GaSwarmComponent::animationPosition( BcU32 Idx ) const
{
	TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];
	
	BcReal HopPosition = BcSin( BcPI * pAnimationLogic->AnimationPosition_ ) * HOP_HEIGHT;
	BcVec2d LinearPosition;
	LinearPosition.lerp( pAnimationLogic->StartPosition_, pAnimationLogic->EndPosition_,  BcSmoothStep( pAnimationLogic->AnimationPosition_ ) );
	
	return BcVec3d( LinearPosition.x(), LinearPosition.y(), HopPosition );
}

////////////////////////////////////////////////////////////////////////////////
// shouldStartMoveAnimation
BcBool GaSwarmComponent::shouldStartMoveAnimation( BcU32 Idx )
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
void GaSwarmComponent::startMoveAnimation( BcU32 Idx, const BcVec2d& Start, const BcVec2d& End )
{
	TAnimationLogic* pAnimationLogic = AnimationLogicList_[ Idx ];
	
	pAnimationLogic->StartPosition_ = Start;
	pAnimationLogic->EndPosition_ = End;
	pAnimationLogic->AnimationPosition_ = 0.0f;
	pAnimationLogic->AnimationSpeed_ = BcMax( ( Start - End ).magnitude(), 1.0f );
}

////////////////////////////////////////////////////////////////////////////////
// isAlive
BcBool GaSwarmComponent::isAlive()
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
