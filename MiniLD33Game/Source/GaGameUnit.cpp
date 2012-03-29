/**************************************************************************
*
* File:		GaGameUnit.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/03/12	
* Description:
*		Game unit.
*		
*
*
* 
**************************************************************************/

#include "GaGameUnit.h"

#include "GaGameSimulator.h"

#include "GaTopState.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
GaGameUnit::GaGameUnit( GaGameSimulator* pSimulator, const GaGameUnitDescriptor& Desc, BcU32 TeamID, BcU32 ID, const BcFixedVec2d& Position ):
	pSimulator_( pSimulator ),
	Desc_( Desc ),
	TeamID_( TeamID ),
	ID_( ID ),
	Behaviour_( BEHAVIOUR_IDLE )
{
	CurrState_.Position_ = Position;
	PrevState_ = CurrState_;
	NextState_ = CurrState_;

	Health_ = Desc_.Health_;
	IsAttackMove_ = BcFalse;

	TargetUnitID_ = BcErrorCode;

	WalkTimer_ = ( BcAbs( BcRandom::Global.randReal() ) * Desc_.MoveSpeed_ ) * 0.2f;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
GaGameUnit::~GaGameUnit()
{

}

//////////////////////////////////////////////////////////////////////////
// getTeamID
BcU32 GaGameUnit::getTeamID() const
{
	return TeamID_;
}

//////////////////////////////////////////////////////////////////////////
// getID
BcU32 GaGameUnit::getID() const
{
	return ID_;
}

//////////////////////////////////////////////////////////////////////////
// getPosition
const BcFixedVec2d& GaGameUnit::getPosition() const
{
	return CurrState_.Position_;
}

//////////////////////////////////////////////////////////////////////////
// getVelocity
const BcFixedVec2d& GaGameUnit::getVelocity() const
{
	return CurrState_.Velocity_;
}

//////////////////////////////////////////////////////////////////////////
// isDead
BcBool GaGameUnit::isDead() const
{
	return Behaviour_ == BEHAVIOUR_DEAD;
}

//////////////////////////////////////////////////////////////////////////
// getInterpolatedPosition
BcFixedVec2d GaGameUnit::getInterpolatedPosition( BcFixed Fraction ) const
{
	return PrevState_.Position_ + ( ( CurrState_.Position_ - PrevState_.Position_ ) * Fraction );
}

//////////////////////////////////////////////////////////////////////////
// getDesc
const GaGameUnitDescriptor& GaGameUnit::getDesc() const
{
	return Desc_;
}

//////////////////////////////////////////////////////////////////////////
// getMaxHealth
BcFixed GaGameUnit::getMaxHealth() const
{
	return Desc_.Health_;
}

//////////////////////////////////////////////////////////////////////////
// getHealth
BcFixed GaGameUnit::getHealth() const
{
	return Health_;
}

//////////////////////////////////////////////////////////////////////////
// getMaxAttackTime
BcFixed GaGameUnit::getMaxAttackTime() const
{
	return BcFixed( 1.0f ) / Desc_.RateOfAttack_;
}

//////////////////////////////////////////////////////////////////////////
// getAttackTime
BcFixed GaGameUnit::getAttackTime() const
{
	return AttackTimer_;
}

//////////////////////////////////////////////////////////////////////////
// getMoveTargetPosition
BcFixedVec2d GaGameUnit::getMoveTargetPosition() const
{
	return MoveTargetPosition_;
}

//////////////////////////////////////////////////////////////////////////
// getChecksum
BcU32 GaGameUnit::getChecksum() const
{
	BcU32 Checksum = 0;

	Checksum += BcHash( (BcU8*)&TeamID_, sizeof( TeamID_ ) );
	Checksum += BcHash( (BcU8*)&ID_, sizeof( ID_ ) );
	Checksum += BcHash( (BcU8*)&Behaviour_, sizeof( Behaviour_ ) );
	Checksum += BcHash( (BcU8*)&NextState_, sizeof( NextState_ ) );
	Checksum += BcHash( (BcU8*)&CurrState_, sizeof( CurrState_ ) );
	Checksum += BcHash( (BcU8*)&PrevState_, sizeof( PrevState_ ) );
	Checksum += BcHash( (BcU8*)&AttackTimer_, sizeof( AttackTimer_ ) );
	Checksum += BcHash( (BcU8*)&Health_, sizeof( Health_ ) );
	Checksum += BcHash( (BcU8*)&MoveTargetPosition_, sizeof( MoveTargetPosition_ ) );

	return Checksum;
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourIdle
void GaGameUnit::setBehaviourIdle()
{
	if( Health_ > 0.0f )
	{
		Behaviour_ = BEHAVIOUR_IDLE;
		CurrState_.Velocity_ = BcFixedVec2d( 0.0f, 0.0f );

		pSimulator_->addDebugPoint( CurrState_.Position_, 0.5f, RsColour::WHITE );
	}
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourGuard
void GaGameUnit::setBehaviourGuard()
{
	if( Health_ > 0.0f )
	{
		Behaviour_ = BEHAVIOUR_GUARD;
		CurrState_.Velocity_ = BcFixedVec2d( 0.0f, 0.0f );

		pSimulator_->addDebugPoint( CurrState_.Position_, 0.5f, RsColour::WHITE );
	}
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourMove
void GaGameUnit::setBehaviourMove( const BcFixedVec2d& Target, BcBool IsAttackMove, BcBool DrawDebug )
{
	if( Health_ > 0.0f )
	{
		Behaviour_ = BEHAVIOUR_MOVE;
		MoveTargetPosition_ = Target;
		IsAttackMove_ = IsAttackMove;

		// Hacky clamp.
		BcFixed HW = ( 1280.0f * 0.5f ) / 32.0f - 1.0f;
		BcFixed HH = ( 720.0f * 0.5f ) / 32.0f - 1.0f;
		MoveTargetPosition_.x( BcClamp( MoveTargetPosition_.x(), -HW, HW ) );
		MoveTargetPosition_.y( BcClamp( MoveTargetPosition_.y(), -HH, HH ) );
		
		if( DrawDebug )
		{
			pSimulator_->addDebugPoint( Target, 0.5f, IsAttackMove ? RsColour::RED : RsColour::GREEN );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourAttack
void GaGameUnit::setBehaviourAttack( BcU32 TargetUnitID, BcBool DrawDebug )
{
	if( Health_ > 0.0f )
	{
		Behaviour_ = BEHAVIOUR_ATTACK;
		TargetUnitID_ = TargetUnitID;

		GaGameUnit* pGameUnit = pSimulator_->getUnit( TargetUnitID );
		if( pGameUnit && DrawDebug )
		{
			pSimulator_->addDebugPoint( pGameUnit->getPosition(), 0.5f, RsColour::RED );
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourDamage
void GaGameUnit::setBehaviourDamage( const BcFixedVec2d& Target )
{
	if( Health_ > 0.0f )
	{
		Behaviour_ = BEHAVIOUR_DAMAGE;
		MoveTargetPosition_ = Target;

		if( Desc_.pLaunchSound_ != NULL )
		{
			GaTopState::pImpl()->playSound( Desc_.pLaunchSound_, getPosition() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourDead
void GaGameUnit::setBehaviourDead()
{
	Behaviour_ = BEHAVIOUR_DEAD;
	
	if( Desc_.pDieSound_ != NULL )
	{
		GaTopState::pImpl()->playSound( Desc_.pDieSound_, getPosition() );
	}

	pSimulator_->addDebugPoint( CurrState_.Position_, 1.2f, RsColour::RED );
}

//////////////////////////////////////////////////////////////////////////
// inRangeForAttack
GaGameUnit::TRange GaGameUnit::inRangeForAttack( BcU32 TargetID )
{
	if( Desc_.pDamageUnit_ != NULL )
	{
		// Check target unit is in range.
		GaGameUnit* pTargetUnit = pSimulator_->getUnit( TargetID );

		if( pTargetUnit != NULL )
		{
			BcFixed DistanceSquared = ( pTargetUnit->getPosition() - getPosition() ).magnitudeSquared();
			BcFixed MinRange = ( Desc_.MinRange_ * Desc_.MinRange_ );
			BcFixed MaxRange = ( Desc_.Range_ * Desc_.Range_ );
			if( DistanceSquared > MaxRange )
			{
				return RANGE_OUT_MAX;
			}
			else if( DistanceSquared < MinRange )
			{
				return RANGE_OUT_MIN;
			}
			else
			{
				return RANGE_IN;
			}
		}
	}

	return RANGE_NONE;
}

//////////////////////////////////////////////////////////////////////////
// doAttack
void GaGameUnit::doAttack( BcU32 TargetID )
{
	// Check in range.
	if( inRangeForAttack( TargetID ) == RANGE_IN )
	{
		// If we can attack, launch projectile.
		if( AttackTimer_ <= 0.0f )
		{
			AttackTimer_ = BcFixed( 1.0f ) / Desc_.RateOfAttack_;
	
			GaGameUnit* pTargetUnit = pSimulator_->getUnit( TargetID );

			// Movement prediction.
			BcU32 PredictionIterations = 6;
			BcFixedVec2d TargetPosition = pTargetUnit->CurrState_.Position_;

			for( BcU32 Idx = 0; Idx < PredictionIterations; ++Idx )
			{
				BcFixed FlyDistance = ( TargetPosition - CurrState_.Position_ ).magnitude();
				BcFixed FlyTime = FlyDistance / Desc_.pDamageUnit_->MoveSpeed_;
				TargetPosition = TargetPosition + pTargetUnit->CurrState_.Velocity_ * FlyTime * pSimulator_->getSimulationRate();
			}
			
			// Fire projectile.
			GaGameUnit* pGameProjectile = pSimulator_->addUnit( *Desc_.pDamageUnit_, TeamID_ + 2, CurrState_.Position_ );
			pGameProjectile->setBehaviourDamage( TargetPosition );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// applyDamage
void GaGameUnit::applyDamage( BcFixed Amount )
{
	Health_ -= Amount;
}

//////////////////////////////////////////////////////////////////////////
// tick
void GaGameUnit::tick( BcFixed Delta )
{
	tickBehaviour( Delta );
	tickState( Delta );
}

//////////////////////////////////////////////////////////////////////////
// tickState
void GaGameUnit::tickState( BcFixed Delta )
{
	PrevState_ = CurrState_;
	NextState_ = CurrState_;

	//if( NextState_.Velocity_.magnitudeSquared() > 0.0f )
	{
		NextState_.Position_ += NextState_.Velocity_ * Delta;

		if( Behaviour_ != BEHAVIOUR_DAMAGE )
		{
			// Check new state is valid.
			BcFixedVec2d Min( NextState_.Position_ - ( Desc_.Size_ * 0.9f ) );
			BcFixedVec2d Max( NextState_.Position_ + ( Desc_.Size_ * 0.9f ) );
			
			GaGameUnitIDList FoundUnits;
			
			pSimulator_->findUnits( FoundUnits, Min, Max, ID_, 0x3 );

			// Move unit away from it's nearest one.
			if( FoundUnits.size() != 0 )
			{
				BcU32 NearestUnit = pSimulator_->findNearestUnit( CurrState_.Position_, ID_, 0x3 );
				GaGameUnit* pGameUnit = pSimulator_->getUnit( NearestUnit );

				if( pGameUnit != NULL )
				{
					BcFixedVec2d Direction = ( CurrState_.Position_ - pGameUnit->CurrState_.Position_ ).normal();

					if( Direction.dot( NextState_.Velocity_.normal() ) > 0.0f || NextState_.Velocity_.magnitudeSquared() == 0.0f )
					{
						BcFixedVec2d RandomOffset( (int)( pSimulator_->rand() % 64 ), (int)( pSimulator_->rand() % 64 ) );
						NextState_ = CurrState_;
						NextState_.Velocity_ = NextState_.Velocity_ * 0.4f + ( Direction * Desc_.MoveSpeed_ ) * 0.6f;
						NextState_.Position_ += NextState_.Velocity_ * Delta + ( RandomOffset / 4096.0f );
					}
				}
				else
				{
					NextState_ = CurrState_;
				}
			}
		}
	}

	//
	WalkTimer_ -= NextState_.Velocity_.magnitude() * Delta;

	if( WalkTimer_ < 0.0f )
	{
		GaTopState::pImpl()->playSound( "Walk", getPosition() );
		WalkTimer_ += ( Desc_.MoveSpeed_ + (BcAbs( BcRandom::Global.randReal() ) * 0.02f ) ) * 0.2f;
	}

	// Clamp position and move away.
	BcFixed HW = ( 1280.0f * 0.5f ) / 32.0f - 1.0f;
	BcFixed HH = ( 720.0f * 0.5f ) / 32.0f - 1.0f;
	BcFixedVec2d ClampedPosition;
	ClampedPosition.x( BcClamp( NextState_.Position_.x(), -HW, HW ) );
	ClampedPosition.y( BcClamp( NextState_.Position_.y(), -HH, HH ) );
	if( NextState_.Position_ != ClampedPosition )
	{
		BcFixedVec2d RandomOffset( (int)( pSimulator_->rand() % 64 ), (int)( pSimulator_->rand() % 64 ) );
		NextState_.Velocity_ += RandomOffset / 4096.0f;
		NextState_.Position_ = ClampedPosition;
	}
}

//////////////////////////////////////////////////////////////////////////
// tickBehaviour
void GaGameUnit::tickBehaviour( BcFixed Delta )
{
#if PSY_DEBUG
	static BcU32 BreakID = BcErrorCode;
	if( ID_ == BreakID )
	{
		BcBreakpoint;
	}
#endif

	switch( Behaviour_ )
	{
	case BEHAVIOUR_IDLE:
		{
			CurrState_.Velocity_ = BcFixedVec2d( 0.0f, 0.0f );
			BcU32 NearestUnit = pSimulator_->findNearestUnit( CurrState_.Position_, ID_, 1 << ( 1 - TeamID_ ) );
			if( NearestUnit != BcErrorCode )
			{
				doAttack( NearestUnit );
			}
		}
		break;

	case BEHAVIOUR_GUARD:
		{
			CurrState_.Velocity_ = BcFixedVec2d( 0.0f, 0.0f );
			BcU32 NearestUnit = pSimulator_->findNearestUnit( CurrState_.Position_, ID_, 1 << ( 1 - TeamID_ ) );
			if( NearestUnit != BcErrorCode )
			{
				doAttack( NearestUnit );
			}
		}
		break;

	case BEHAVIOUR_MOVE:
		{
			BcU32 NearestUnit = pSimulator_->findNearestUnit( CurrState_.Position_, ID_, 1 << ( 1 - TeamID_ ) );
			if( IsAttackMove_ && inRangeForAttack( NearestUnit ) == RANGE_IN )
			{
				CurrState_.Velocity_ = BcFixedVec2d( 0.0f, 0.0f );

				doAttack( NearestUnit );
			}
			else
			{
				// Reset attack timer.
				AttackTimer_ = BcMax( AttackTimer_,  BcFixed( Desc_.CoolDownMultiplier_ ) / Desc_.RateOfAttack_ );
				
				// Make longer whe moving.
				AttackTimer_ = BcMin( BcFixed( 1.0f ) / Desc_.RateOfAttack_, AttackTimer_ + ( BcFixed( 1.0f ) / Desc_.RateOfAttack_ ) * ( Delta * 0.05f ) );

				// Calculate velocity vector.
				CurrState_.Velocity_ = BcFixedVec2d( MoveTargetPosition_ - CurrState_.Position_ ).normal() * Desc_.MoveSpeed_;
				BcFixed MoveDistance = Desc_.MoveSpeed_ * Delta;
				if( ( CurrState_.Position_ - MoveTargetPosition_ ).magnitudeSquared() < ( MoveDistance * MoveDistance ) )
				{
					CurrState_.Position_ = MoveTargetPosition_;
					setBehaviourIdle();
				}
			}
		}
		break;

	case BEHAVIOUR_ATTACK:
		{
			TRange Range = inRangeForAttack( TargetUnitID_ );
			if( Range == RANGE_IN )
			{
				doAttack( TargetUnitID_ );
			}
			else
			{
				// Reset attack timer.
				AttackTimer_ = BcMax( AttackTimer_,  BcFixed( Desc_.CoolDownMultiplier_ ) / ( Desc_.RateOfAttack_ ) );

				// Make longer whe moving.
				AttackTimer_ = BcMin( BcFixed( 1.0f ) / Desc_.RateOfAttack_, AttackTimer_ + ( BcFixed( 1.0f ) / Desc_.RateOfAttack_ ) * ( Delta * 0.05f ) );

				// Get unit position.
				GaGameUnit* pUnit = pSimulator_->getUnit( TargetUnitID_ );
				if( pUnit != NULL )
				{
					BcFixedVec2d Direction = ( getPosition() - pUnit->getPosition() ).normal() * ( ( Desc_.MinRange_ + Desc_.Range_ ) * 0.5f );
					MoveTargetPosition_ = pUnit->getPosition() + Direction;
				}
				else
				{
					// Attack move to previous target.
					IsAttackMove_ = BcTrue;
					Behaviour_ = BEHAVIOUR_MOVE;
				}

				// Calculate velocity vector.
				CurrState_.Velocity_ = BcFixedVec2d( MoveTargetPosition_ - CurrState_.Position_ ).normal() * Desc_.MoveSpeed_;

				// If we moved further away, then stop and set target.
				BcFixed CurrMagnitudeSquared = ( CurrState_.Position_ - MoveTargetPosition_ ).magnitudeSquared();
				BcFixed PrevMagnitudeSquared = ( PrevState_.Position_ - MoveTargetPosition_ ).magnitudeSquared();
				if( CurrMagnitudeSquared > PrevMagnitudeSquared )
				{
					setBehaviourIdle();
				}
			}
		}
		break;

	case BEHAVIOUR_DAMAGE:
		{
			// Calculate velocity vector.
			CurrState_.Velocity_ = BcFixedVec2d( MoveTargetPosition_ - CurrState_.Position_ ).normal() * Desc_.MoveSpeed_;
			BcFixed MoveDistance = Desc_.MoveSpeed_ * Delta;
			if( ( CurrState_.Position_ - MoveTargetPosition_ ).magnitudeSquared() < ( MoveDistance * MoveDistance ) )
			{
				CurrState_.Position_ = MoveTargetPosition_;

				// Cause damage.
				pSimulator_->applyDamage( MoveTargetPosition_, Desc_.Range_, Desc_.Health_ );

				// Debug point!
				pSimulator_->addDebugPoint( getPosition(), Desc_.Range_, RsColour::YELLOW );

				if( Desc_.pHitSound_ != NULL )
				{
					GaTopState::pImpl()->playSound( Desc_.pHitSound_, MoveTargetPosition_ );
				}

				// Set as dead.
				setBehaviourDead();
			}
		}
		break;
	}

	// Attack timer.
	if( AttackTimer_ > 0.0f )
	{
		AttackTimer_ -= Delta;
	}
	else if( AttackTimer_ < 0.0f )
	{
		AttackTimer_ = 0.0f;
	}

	// Death.
	if( Health_ <= 0.0f )
	{
		Health_ = 0.0f;
		Behaviour_ = BEHAVIOUR_DEAD;
	}
}

//////////////////////////////////////////////////////////////////////////
// advanceState
void GaGameUnit::advanceState()
{
	CurrState_ = NextState_;
}

//////////////////////////////////////////////////////////////////////////
// render
void GaGameUnit::render( ScnCanvasComponentRef Canvas, BcFixed TimeFraction )
{
	if( Behaviour_ != BEHAVIOUR_DEAD )
	{
		BcU32 TextureIdx = Desc_.Type_;
		BcFixedVec2d GamePosition( getInterpolatedPosition( TimeFraction ) );
		const BcReal ScaleFactor = 32.0f;
		BcVec2d Position( GamePosition.x(), GamePosition.y() );
		BcVec2d Size( Desc_.Size_.x(), Desc_.Size_.y() );

		Position *= ScaleFactor;
		Size *= ScaleFactor;

		Position = BcVec2d( BcFloor( Position.x() ), BcFloor( Position.y() ) );

		Canvas->drawSpriteCentered( Position, Size, TextureIdx, RsColour::WHITE, 2 );
	}
}

//////////////////////////////////////////////////////////////////////////
// renderShadow
void GaGameUnit::renderShadow( ScnCanvasComponentRef Canvas, BcFixed TimeFraction )
{
	if( Behaviour_ != BEHAVIOUR_DEAD )
	{
		BcU32 TextureIdx = Desc_.Type_;
		BcFixedVec2d GamePosition( getInterpolatedPosition( TimeFraction ) );
		const BcReal ScaleFactor = 32.0f;
		BcVec2d Position( GamePosition.x(), GamePosition.y() );
		BcVec2d Size( Desc_.Size_.x(), Desc_.Size_.y() );

		Position *= ScaleFactor;
		Size *= ScaleFactor;
		Canvas->drawSpriteCentered( Position + BcVec2d( 0.0f, Size.y() * 0.5f ), Size * BcVec2d( 1.4f, 0.4f ), 8, RsColour::WHITE, 1 );
	}
}

//////////////////////////////////////////////////////////////////////////
// renderHUD
void GaGameUnit::renderHUD( ScnCanvasComponentRef Canvas, BcFixed TimeFraction )
{
	if( TeamID_ > 1 )
	{
		return;
	}

	if( Behaviour_ != BEHAVIOUR_DEAD )
	{
		BcU32 TextureIdx = Desc_.Type_;
		BcFixedVec2d GamePosition( getInterpolatedPosition( TimeFraction ) );
		const BcReal ScaleFactor = 32.0f;
		BcVec2d Position( GamePosition.x(), GamePosition.y() );
		BcVec2d Size( Desc_.Size_.x(), Desc_.Size_.y() );

		Position *= ScaleFactor;
		Size *= ScaleFactor;

		// Draw range if selected.
		const BcReal MaxRangeRadius = Desc_.Range_ / ScaleFactor;
		
		
		// Draw health bar.
		BcFixed HealthFraction = Health_ / Desc_.Health_;
		BcVec2d HealthBarPosition = Position - Size * 0.5f + BcVec2d( 2.0f, -6.0f );
		BcVec2d HealthBarSize = BcVec2d( HealthFraction * ( Size.x() - 4.0f ), 2.0f );
		RsColour HealthColour = RsColour::GREEN ;

		if( HealthFraction <= 0.5f )
		{
			HealthColour = RsColour::YELLOW;
		}
		if( HealthFraction <= 0.25f )
		{
			HealthColour = RsColour::RED;
		}

		Canvas->drawSprite( HealthBarPosition, HealthBarSize, 0, HealthColour * RsColour( 1.0f, 1.0f, 1.0f, 0.75f ), 3 );

		// Draw cool down bar.
		BcFixed MaxAttackTime = BcFixed( 1.0f ) / Desc_.RateOfAttack_;
		BcFixed CoolDownFraction = BcFixed( 1.0f ) - ( AttackTimer_ / MaxAttackTime );
		BcVec2d CoolDownBarPosition = Position - Size * 0.5f + BcVec2d( 2.0f, -4.0f );
		BcVec2d CoolDownBarSize = BcVec2d( CoolDownFraction * ( Size.x() - 4.0f ), 2.0f );
		RsColour CoolDownColour = RsColour::PURPLE;

		Canvas->drawSprite( CoolDownBarPosition, CoolDownBarSize, 0, CoolDownColour * RsColour( 1.0f, 1.0f, 1.0f, 0.75f ), 3 );
	}
}


//////////////////////////////////////////////////////////////////////////
// renderSelectionHUD
void GaGameUnit::renderSelectionHUD( ScnCanvasComponentRef Canvas, BcFixed TimeFraction, BcU32 TeamID )
{
	if( Behaviour_ != BEHAVIOUR_DEAD )
	{
		BcU32 TextureIdx = Desc_.Type_;
		BcFixedVec2d GamePosition( getInterpolatedPosition( TimeFraction ) );
		const BcReal ScaleFactor = 32.0f;
		BcVec2d Position( GamePosition.x(), GamePosition.y() );
		BcVec2d Size( Desc_.Size_.x(), Desc_.Size_.y() );

		Position *= ScaleFactor;
		Size *= ScaleFactor;

		BcVec2d PositionA( Position + ( BcVec2d( -Size.x(), -Size.y() ) * 0.45f ) );
		BcVec2d PositionB( Position + ( BcVec2d(  Size.x(), -Size.y() ) * 0.45f ) );
		BcVec2d PositionC( Position + ( BcVec2d(  Size.x(),  Size.y() ) * 0.45f ) );
		BcVec2d PositionD( Position + ( BcVec2d( -Size.x(),  Size.y() ) * 0.45f ) );
		BcVec2d SizeA( BcVec2d(  16.0f,  16.0f ) );
		BcVec2d SizeB( BcVec2d( -16.0f,  16.0f ) );
		BcVec2d SizeC( BcVec2d( -16.0f, -16.0f ) );
		BcVec2d SizeD( BcVec2d(  16.0f, -16.0f ) );

		// Draw selection marker.
		RsColour MarkerColour = TeamID == TeamID_ ? ( RsColour::WHITE ) : ( RsColour::RED ) * RsColour( 1.0f, 1.0f, 1.0f, 0.75f );
		Canvas->drawSpriteCentered( PositionA, SizeA, 2, MarkerColour, 3 );
		Canvas->drawSpriteCentered( PositionB, SizeB, 2, MarkerColour, 3 );
		Canvas->drawSpriteCentered( PositionC, SizeC, 2, MarkerColour, 3 );
		Canvas->drawSpriteCentered( PositionD, SizeD, 2, MarkerColour, 3 );
	}
}

//////////////////////////////////////////////////////////////////////////
// renderRangeHUD
void GaGameUnit::renderRangeHUD( ScnCanvasComponentRef Canvas, BcFixed TimeFraction, BcU32 TeamID )
{
	if( Behaviour_ != BEHAVIOUR_DEAD )
	{
		BcU32 TextureIdx = Desc_.Type_;
		BcFixedVec2d GamePosition( getInterpolatedPosition( TimeFraction ) );
		const BcReal ScaleFactor = 32.0f;
		BcVec2d Position( GamePosition.x(), GamePosition.y() );
		Position *= ScaleFactor;

		// Draw range markers if need be.
		if( Desc_.Range_ > 12.0f )
		{
			BcReal Radius = Desc_.Range_ * ScaleFactor;
			for( BcReal Theta = 0.0f; Theta < BcPIMUL2; Theta += (BcPIMUL2 / 48.0f ) )
			{
				BcVec2d OutPos = Position + BcVec2d( BcCos( Theta ), -BcSin( Theta ) ) * Radius;
				Canvas->drawSpriteCentered( OutPos, BcVec2d( 8.0f, 8.0f ), 3, RsColour::WHITE, 4 );
			}
		}
	}
}
