/**************************************************************************
*
* File:		GaMonsterComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#include "GaMonsterComponent.h"

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaMonsterComponent )

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaMonsterComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaMonsterComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaMonsterComponent::initialise()
{
	// Find player entity to use for being mean to :E
	if( CsCore::pImpl()->findResource( "PlayerEntity_0", PlayerEntity_ ) )
	{
		PlayerComponent_ = PlayerEntity_->getComponent( 0 );
	}

	// Create resources.
	CsCore::pImpl()->createResource( BcName::INVALID, LocalEmitter_ );

	// Get resources.
	CsCore::pImpl()->findResource( "footstep", FootStepSound_ );
	CsCore::pImpl()->findResource( "monsteralert0", MonsterAlertSound_ );
	CsCore::pImpl()->findResource( "monsterroar0", MonsterRoarSound_[0] );
	CsCore::pImpl()->findResource( "monsterroar1", MonsterRoarSound_[1] );
	CsCore::pImpl()->findResource( "monsterroar2", MonsterRoarSound_[2] );
	CsCore::pImpl()->findResource( "monsterroar3", MonsterRoarSound_[3] );
	CsCore::pImpl()->findResource( "dead", DeadSound_ );
	
	//
	FootStepTimer_ = 0.0f;
	IdleTimer_ = 10.0f;

	MonsterState_ = MS_IDLE;

	TrackSpeed_ = 0.0f;
	MoveSpeed_ = 5.0f;
	MoveCircle_ = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaMonsterComponent::update( BcReal Tick )
{
	// Steal rotation from player.
	getParentEntity()->setRotation( PlayerEntity_->getTransform().getRotation() );

	BcVec3d Position = getParentEntity()->getTransform().getTranslation();
	BcVec3d PlayerPosition = PlayerEntity_->getTransform().getTranslation();
	BcVec3d DirectionFromPlayer = Position - PlayerPosition;
	BcReal DistanceAwayFromPlayer = ( DirectionFromPlayer ).magnitude();
	BcReal DistanceFromPlayer = 64.0f;
	BcReal CircleNoiseThing = BcPIMUL2 * BcRandom::Global.interpolatedNoise( MoveCircle_, 64 );
	BcVec3d Intersection;
	BcBool IsInSight = ScnRigidBodyWorldComponent::StaticGetComponent()->lineCheck( Position, PlayerPosition, Intersection );
	switch( MonsterState_ )
	{
	case MS_IDLE:
		{
			IdleTimer_ -= Tick;
			TrackSpeed_ = 1.0f;
			DistanceFromPlayer = 80.0f;
			TargetPosition_ = BcVec3d( -BcCos( CircleNoiseThing ), 0.0f, BcSin( CircleNoiseThing ) ) * DistanceFromPlayer + PlayerPosition;

			// Don't get too close when idling!
			if( DistanceAwayFromPlayer < 64.0f )
			{
				TargetPosition_ = ( DirectionFromPlayer.normal() * DistanceFromPlayer ) + PlayerPosition;
			}
			
			if( DistanceAwayFromPlayer < 48.0f || IdleTimer_ < 0.0f )
			{
				IdleTimer_ = ( BcRandom::Global.randReal() + 1.0f ) * 25.0f;
				LocalEmitter_->play( MonsterAlertSound_ );
				
				// 
				MonsterState_ = MS_HUNT;
			}
		}
		break;

	case MS_HUNT:
		{
			DistanceFromPlayer = 32.0f;
			TrackSpeed_ = 1.5f;
			TargetPosition_ = BcVec3d( -BcCos( CircleNoiseThing ), 0.0f, BcSin( CircleNoiseThing * 2.0f ) ) * DistanceFromPlayer + PlayerPosition;

			if( DistanceAwayFromPlayer > 68.0f )
			{
				MonsterState_ = MS_IDLE;
			}

			if( DistanceAwayFromPlayer < 20.0f && IsInSight )
			{
				KillTimer_ = 0.0f;
				MonsterState_ = MS_KILL;
			}
		}
		break;

	case MS_KILL:
		{
			KillTimer_ -= Tick;
			DistanceFromPlayer = 16.0f;
			TrackSpeed_ = 2.25f;
			TargetPosition_ = BcVec3d( -BcCos( CircleNoiseThing * 2.0f ), 0.0f, BcSin( CircleNoiseThing ) ) * DistanceFromPlayer + PlayerPosition;

			// Run for player if in sight!
			if( IsInSight )
			{
				TargetPosition_ = PlayerPosition;
			}
			
			if( KillTimer_ < 0.0f )
			{
				BcU32 RoarSound = BcRandom::Global.randRange( 0, 3 );
				LocalEmitter_->stopAll();
				LocalEmitter_->play( MonsterRoarSound_[ RoarSound ] );
				KillTimer_ = ( BcRandom::Global.randReal() + 2.0f ) * 2.0f;
			}

			if( DistanceAwayFromPlayer > 20.0f )
			{
				BcU32 RoarSound = BcRandom::Global.randRange( 0, 3 );
				LocalEmitter_->stopAll();
				LocalEmitter_->play( MonsterRoarSound_[ RoarSound ] );
				MonsterState_ = MS_IDLE;
			}
			else if( DistanceAwayFromPlayer < 4.0f )
			{
				BcU32 RoarSound = BcRandom::Global.randRange( 0, 3 );
				LocalEmitter_->stopAll();
				MonsterState_ = MS_WIN;

				// Remove player.
				ScnCore::pImpl()->removeEntity( PlayerEntity_ );
				
				// Spawn title screen.
				GaTopState::pImpl()->startTitle();
			}
		}
		break;

	case MS_WIN:
		{
			TrackSpeed_ = 0.0f;			
		}
		break;
	}

	BcPrintf( "T: %.2f\n", DistanceAwayFromPlayer );

	MoveCircle_ += Tick * ( TrackSpeed_ * 0.1f );
	if( MoveCircle_ > 64.0f )
	{
		MoveCircle_ -= 64.0f;
	}

	BcVec3d Velocity = ( TargetPosition_ - Position ).normal() * MoveSpeed_ * TrackSpeed_;
	Position += Velocity * Tick;
	getParentEntity()->setPosition( Position );

	LocalEmitter_->setGain( 0.7f );
	LocalEmitter_->setPosition( Position );
	
	// Do foot steps.
	FootStepTimer_ -= Tick;
	if( FootStepTimer_ < 0.0f )
	{
		FootStepTimer_ = ( BcRandom::Global.randReal() * 0.2f + 0.9f ) * ( 1.0f / TrackSpeed_ );

		if( SsCore::pImpl() )
		{
			SsChannel* pChannel = SsCore::pImpl()->play( FootStepSound_->getSample(), NULL );
			if( pChannel != NULL )
			{
				pChannel->position( Position );
				pChannel->gain( 0.7f );
				pChannel->pitch( BcRandom::Global.randReal() * 0.4f + 0.8f );
				pChannel->refDistance( 4.0f );
				pChannel->rolloffFactor( 1.0f );
			}

			// Calculate distance from player.
			BcReal Distance = ( PlayerPosition - getParentEntity()->getTransform().getTranslation() ).magnitude();
			BcReal InvDistanceShakeFactor = BcMin( 10.0f / Distance, 0.5f );
			PlayerComponent_->shakeCamera( InvDistanceShakeFactor );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaMonsterComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaMonsterComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	LocalEmitter_->stopAll();
}
