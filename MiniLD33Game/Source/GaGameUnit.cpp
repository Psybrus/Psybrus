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
void GaGameUnit::setBehaviourMove( const BcFixedVec2d& Target )
{
	if( Health_ > 0.0f )
	{
		Behaviour_ = BEHAVIOUR_MOVE;
		MoveTargetPosition_ = Target;

		pSimulator_->addDebugPoint( CurrState_.Position_, 0.5f, RsColour::GREEN );
	}
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourAttack
void GaGameUnit::setBehaviourAttack( BcU32 TargetUnitID )
{
	if( Health_ > 0.0f )
	{
		Behaviour_ = BEHAVIOUR_ATTACK;
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
	}
}

//////////////////////////////////////////////////////////////////////////
// setBehaviourDead
void GaGameUnit::setBehaviourDead()
{
	Behaviour_ = BEHAVIOUR_DEAD;

	pSimulator_->addDebugPoint( CurrState_.Position_, 1.0f, RsColour::RED );
}

//////////////////////////////////////////////////////////////////////////
// inRangeForAttack
BcBool GaGameUnit::inRangeForAttack( BcU32 TargetID )
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
			if( DistanceSquared < MaxRange && DistanceSquared > MinRange )
			{
				return BcTrue;
			}
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// doAttack
void GaGameUnit::doAttack( BcU32 TargetID )
{
	// Check in range.
	if( inRangeForAttack( TargetID ) )
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

	if( NextState_.Velocity_.magnitudeSquared() > 0.0f )
	{
		NextState_.Position_ += NextState_.Velocity_ * Delta;

		if( Behaviour_ != BEHAVIOUR_DAMAGE )
		{
			// Check new state is valid.
			BcFixedVec2d Min( NextState_.Position_ - ( Desc_.Size_ * 0.45f ) );
			BcFixedVec2d Max( NextState_.Position_ + ( Desc_.Size_ * 0.45f ) );
			
			GaGameUnitIDList FoundUnits;
			
			pSimulator_->findUnits( FoundUnits, Min, Max, ID_, 0x3 );

			// TODO: Recalculate velocity vector.
			if( FoundUnits.size() != 0 )
			{
				NextState_ = CurrState_;
			}
		}
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
			BcU32 NearestUnit = pSimulator_->findNearestUnit( CurrState_.Position_, ID_, 1 << ( 1 - TeamID_ ) );
			if( NearestUnit != BcErrorCode )
			{
				doAttack( NearestUnit );
			}
		}
		break;

	case BEHAVIOUR_GUARD:
		{
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
			if( inRangeForAttack( NearestUnit ) )
			{
				CurrState_.Velocity_ = BcFixedVec2d( 0.0f, 0.0f );

				doAttack( NearestUnit );
			}
			else
			{
				// Reset attack timer.
				AttackTimer_ = BcFixed( 1.0f ) / Desc_.RateOfAttack_;

				// Calculate velocity vector.
				CurrState_.Velocity_ = BcFixedVec2d( MoveTargetPosition_ - CurrState_.Position_ ).normal() * Desc_.MoveSpeed_;
				BcFixed MoveDistance = Desc_.MoveSpeed_ * Delta;
				if( ( CurrState_.Position_ - MoveTargetPosition_ ).magnitudeSquared() < MoveDistance )
				{
					CurrState_.Position_ = MoveTargetPosition_;
					setBehaviourIdle();
				}
			}
		}
		break;

	case BEHAVIOUR_ATTACK:
		{
			/*
			BcU32 NearestUnit = pSimulator_->findNearestUnit( CurrState_.Position_, ID_, 1 - TeamID_ );
			if( NearestUnit != BcErrorCode )
			{
				doAttack( NearestUnit );
			}
			else
			{
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
			*/
		}
		break;

	case BEHAVIOUR_DAMAGE:
		{
			// Calculate velocity vector.
			CurrState_.Velocity_ = BcFixedVec2d( MoveTargetPosition_ - CurrState_.Position_ ).normal() * Desc_.MoveSpeed_;
			BcFixed MoveDistance = Desc_.MoveSpeed_ * Delta;
			if( ( CurrState_.Position_ - MoveTargetPosition_ ).magnitudeSquared() < MoveDistance )
			{
				CurrState_.Position_ = MoveTargetPosition_;

				// Cause damage.
				pSimulator_->applyDamage( MoveTargetPosition_, Desc_.Range_, Desc_.Health_ );

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
	if( Behaviour_ != BEHAVIOUR_DEAD )
	{
		BcU32 TextureIdx = Desc_.Type_;
		BcFixedVec2d GamePosition( getInterpolatedPosition( TimeFraction ) );
		const BcReal ScaleFactor = 32.0f;
		BcVec2d Position( GamePosition.x(), GamePosition.y() );
		BcVec2d Size( Desc_.Size_.x(), Desc_.Size_.y() );

		Position *= ScaleFactor;
		Size *= ScaleFactor;
		
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
void GaGameUnit::renderSelectionHUD( ScnCanvasComponentRef Canvas, BcFixed TimeFraction )
{
	if( Behaviour_ != BEHAVIOUR_DEAD )
	{
		static RsColour TeamColour[] = 
		{
			RsColour::RED,
			RsColour::BLUE,
		};

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
		Canvas->drawSpriteCentered( PositionA, SizeA, 2, RsColour::WHITE * RsColour( 1.0f, 1.0f, 1.0f, 0.75f ), 3 );
		Canvas->drawSpriteCentered( PositionB, SizeB, 2, RsColour::WHITE * RsColour( 1.0f, 1.0f, 1.0f, 0.75f ), 3 );
		Canvas->drawSpriteCentered( PositionC, SizeC, 2, RsColour::WHITE * RsColour( 1.0f, 1.0f, 1.0f, 0.75f ), 3 );
		Canvas->drawSpriteCentered( PositionD, SizeD, 2, RsColour::WHITE * RsColour( 1.0f, 1.0f, 1.0f, 0.75f ), 3 );
	}
}
