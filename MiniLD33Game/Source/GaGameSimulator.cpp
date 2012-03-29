/**************************************************************************
*
* File:		GaGameSimulator.cpp
* Author:	Neil Richardson 
* Ver/Date:	24/03/12
* Description:
*		Game unit.
*		
*
*
* 
**************************************************************************/

#include "EvtBridgeRakNet.h"
#include "EvtProxyLockstep.h"
#include "GaMatchmakingState.h"

#include "GaGameSimulator.h"

//////////////////////////////////////////////////////////////////////////
// GaGameSimulator
GaGameSimulator::GaGameSimulator( BcFixed SimulationRate, BcFixed SimulationSpeed, BcU32 TeamID, BcBool Networked ):
	SimulationRate_( SimulationRate ),
	SimulationSpeed_( SimulationSpeed ),
	TickAccumulator_( 0.0f ),
	CurrentFrame_( 0 ),
	UnitID_( 0 ),
	Checksum_( 0 )
{
	BcAssert( TeamID != BcErrorCode );

	GaGameUnitIdleEvent::Delegate OnUnitIdle( GaGameUnitIdleEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitIdle >( this ) );
	GaGameUnitGuardEvent::Delegate OnUnitGuard( GaGameUnitGuardEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitGuard >( this ) );
	GaGameUnitMoveEvent::Delegate OnUnitMove( GaGameUnitMoveEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitMove >( this ) );
	GaGameUnitAttackEvent::Delegate OnUnitAttack( GaGameUnitAttackEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitAttack >( this ) );

	subscribe( gaEVT_UNIT_IDLE, OnUnitIdle );
	subscribe( gaEVT_UNIT_GUARD, OnUnitGuard );
	subscribe( gaEVT_UNIT_MOVE, OnUnitMove );
	subscribe( gaEVT_UNIT_ATTACK, OnUnitAttack );

	pEventBridge_ = NULL;
	pEventProxy_ = new EvtProxyLockstep( this, TeamID, Networked ? 2 : 1 );

	// If we're networked, create an event bridge.
	if( Networked )
	{
		pEventBridge_ = new EvtBridgeRakNet( this );

		// If the remote address we are trying to connect isn't the same as the mapped address we want to connect to, then connect up normally.
		// If they match, we're behind the same NAT, so use the LAN address we've been given too.
		if( GaMatchmakingState::getRemoteAddr() != GaMatchmakingState::getMappedAddr() )
		{
			pEventBridge_->connect( TeamID, GaMatchmakingState::getRemoteAddr(), GaMatchmakingState::getRemotePort(), GaMatchmakingState::getLocalPort(), GaMatchmakingState::getSocketFileDescriptor() );
		}
		else
		{
			pEventBridge_->connect( TeamID, GaMatchmakingState::getLANAddr(), GaMatchmakingState::getLANPort(), GaMatchmakingState::getLocalPort(), GaMatchmakingState::getSocketFileDescriptor() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// GaGameSimulator
GaGameSimulator::~GaGameSimulator()
{
	unsubscribeAll( this );

	delete pEventProxy_;
	pEventProxy_ = NULL;

	delete pEventBridge_;
	pEventBridge_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// getSimulationRate
BcFixed GaGameSimulator::getSimulationRate() const
{
	return SimulationRate_;
}

//////////////////////////////////////////////////////////////////////////
// addUnit
GaGameUnit* GaGameSimulator::addUnit( const GaGameUnitDescriptor& Desc, BcU32 TeamID, const BcFixedVec2d& Position )
{
	GaGameUnit* pGameUnit = new GaGameUnit( this, Desc, TeamID, UnitID_, Position );
	GameUnitMap_[ UnitID_ ] = pGameUnit;
	GameUnitList_.push_back( pGameUnit );
	++UnitID_;
	return pGameUnit;
}

//////////////////////////////////////////////////////////////////////////
// getUnit
GaGameUnit* GaGameSimulator::getUnit( BcU32 ID )
{
	TGameUnitMap::iterator It( GameUnitMap_.find( ID ) );

	if( It != GameUnitMap_.end() )
	{
		return (*It).second;
	}

	return NULL;	
}

//////////////////////////////////////////////////////////////////////////
// findNearestUnit
BcU32 GaGameSimulator::findNearestUnit( const BcFixedVec2d& Position, BcU32 IgnoreID, BcU32 TeamIDMask )
{
	BcFixed ClosestUnitDistanceSquared( 32766.0f );
	BcU32 ClosestUnitID = BcErrorCode;

	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		const BcFixed DistanceSquared( ( pGameUnit->getPosition() - Position ).magnitudeSquared() );
		
		if( DistanceSquared < ClosestUnitDistanceSquared && pGameUnit->getID() != IgnoreID && ( ( 1 << pGameUnit->getTeamID() ) & TeamIDMask ) != 0 )
		{
			ClosestUnitDistanceSquared = DistanceSquared;
			ClosestUnitID = pGameUnit->getID();
		}
	}
	
	return ClosestUnitID;
}

//////////////////////////////////////////////////////////////////////////
// findUnits
void GaGameSimulator::findUnits( GaGameUnitIDList& List, const BcFixedVec2d& Position, BcFixed Radius, BcU32 IgnoreID, BcU32 TeamIDMask )
{
	List.reserve( GameUnitList_.size() );
	
	const BcFixed RadiusSquared( Radius * Radius );
	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		const BcFixed DistanceSquared( ( pGameUnit->getPosition() - Position ).magnitudeSquared() );
		
		if( DistanceSquared < RadiusSquared && pGameUnit->getID() != IgnoreID && ( ( 1 << pGameUnit->getTeamID() ) & TeamIDMask ) != 0 )
		{
			List.push_back( pGameUnit->getID() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// findUnits
void GaGameSimulator::findUnits( GaGameUnitIDList& List, const BcFixedVec2d& Start, const BcFixedVec2d& End, BcU32 IgnoreID, BcU32 TeamIDMask )
{
	BcFixedVec2d Min;
	BcFixedVec2d Max;
	Min.x( BcMin( Start.x(), End.x() ) );
	Min.y( BcMin( Start.y(), End.y() ) );
	Max.x( BcMax( Start.x(), End.x() ) );
	Max.y( BcMax( Start.y(), End.y() ) );

	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		BcFixedVec2d Position( pGameUnit->getPosition() );
		
		if( Position.x() > Min.x() && Position.y() > Min.y() &&
			Position.x() < Max.x() && Position.y() < Max.y() && 
			pGameUnit->getID() != IgnoreID &&
			( ( 1 << pGameUnit->getTeamID() ) & TeamIDMask ) != 0 )
		{
			List.push_back( pGameUnit->getID() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// findProjectiles
void GaGameSimulator::findProjectiles( GaGameUnitIDList& List )
{
	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		if( pGameUnit->getTeamID() > 1 && !pGameUnit->isDead() )
		{
			List.push_back( pGameUnit->getID() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// applyDamage
void GaGameSimulator::applyDamage( const BcFixedVec2d& Position, BcFixed Range, BcFixed Amount )
{
	GaGameUnitIDList FoundUnits;

	findUnits( FoundUnits, Position, Range );

	BcPrintf( "GaGameSimulator: Apply damage to %u units.\n", FoundUnits.size() );

	for( BcU32 Idx = 0; Idx < FoundUnits.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = getUnit( FoundUnits[ Idx ] );
		if( pGameUnit != NULL )
		{
			BcPrintf( "- TeamID %u, UnitID %u.\n", pGameUnit->getTeamID(), pGameUnit->getID() );

			pGameUnit->applyDamage( Amount );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// tick
BcU32 GaGameSimulator::rand()
{
	return RNG_.rand();
}

//////////////////////////////////////////////////////////////////////////
// tick
void GaGameSimulator::tick( BcReal Delta )
{
	// Only run if we can update the event bridge.
	if( pEventBridge_ == NULL || pEventBridge_->update( Delta ) )
	{
		TickAccumulator_ += Delta * SimulationSpeed_;

		while( TickAccumulator_ > SimulationRate_ )
		{
			BcFixed SimulationRate = SimulationRate_;

			// Set the current frame in the proxy.
			pEventProxy_->setFrameIndex( CurrentFrame_, Checksum_ );
		
			// If we can dispatch for the current frame, we can simulate.
			if( pEventProxy_->dispatchFrameIndex( CurrentFrame_ ) )
			{				
				// Checksum the state.
				for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
				{
					GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
					Checksum_ += pGameUnit->getChecksum();
				}

				// Seed RNG with checksum.
				RNG_ = BcRandom( Checksum_ );

				// Tick units.
				for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
				{
					GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
					pGameUnit->tick( SimulationRate );
				}

				// Advance their state.
				for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
				{
					GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
					pGameUnit->advanceState();
				}

				// Remove dead units.
				for( TGameUnitList::iterator It( GameUnitList_.begin() ); It != GameUnitList_.end();  )
				{
					GaGameUnit* pGameUnit = (*It);

					if( pGameUnit->isDead() )
					{
						GameUnitMap_.erase( GameUnitMap_.find( pGameUnit->getID() ) );
						It = GameUnitList_.erase( It );
						delete pGameUnit;
					}
					else
					{
						++It;
					}
				}

				// Advance frame counter.
				CurrentFrame_ += 1;
				TickAccumulator_ -= SimulationRate;
			}
			else
			{
				// If we're ahead we need to drop back our tick accumulator
				// so the other client can catch up.
				if( pEventProxy_->isAhead() )
				{
					TickAccumulator_ -= SimulationRate;
				}
				return;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// render
void GaGameSimulator::render( ScnCanvasComponentRef Canvas, BcU32 TeamID )
{
	BcFixed TimeFraction = BcClamp( TickAccumulator_ / SimulationRate_, 0.0f, 1.0f );

	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		if( ( pGameUnit->getTeamID() % 2 ) == TeamID )
		{
			pGameUnit->renderShadow( Canvas, TimeFraction );
		}
	}

	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		if( ( pGameUnit->getTeamID() % 2 ) == TeamID )
		{
			pGameUnit->render( Canvas, TimeFraction );
		}
	}	
}

//////////////////////////////////////////////////////////////////////////
// renderHUD
void GaGameSimulator::renderHUD( ScnCanvasComponentRef Canvas, const GaGameUnitIDList& CurrentSelection, BcU32 TeamID )
{
	BcFixed TimeFraction = BcClamp( TickAccumulator_ / SimulationRate_, 0.0f, 1.0f );
	for( BcU32 Idx = 0; Idx < CurrentSelection.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = getUnit( CurrentSelection[ Idx ] );
		if( pGameUnit != NULL )
		{
			pGameUnit->renderSelectionHUD( Canvas, TimeFraction, TeamID );
			pGameUnit->renderRangeHUD( Canvas, TimeFraction, TeamID );
		}
	}

	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		pGameUnit->renderHUD( Canvas, TimeFraction );
	}

	for( std::vector< TDebugPoint >::iterator It( DebugPoints_.begin() ); It != DebugPoints_.end() ; )
	{
		TDebugPoint& DebugPoint( *It );

		if( DebugPoint.Timer_ > 0.0f )
		{		
			BcVec2d Position = BcVec2d( DebugPoint.Position_.x(), DebugPoint.Position_.y() ) * 32.0f;
			BcVec2d Size = BcVec2d( DebugPoint.Size_, DebugPoint.Size_ ) * 32.0f;

			Canvas->drawSpriteCentered( Position, Size, 4, DebugPoint.Colour_ * RsColour( 1.0f, 1.0f, 1.0f, DebugPoint.Timer_ ), 1000 );

			DebugPoint.Timer_ -= BcReal( 0.05f );

			++It;
		}
		else
		{
			It = DebugPoints_.erase( It );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onUnitIdle
eEvtReturn GaGameSimulator::onUnitIdle( EvtID ID, const GaGameUnitIdleEvent& Event )
{
	GaGameUnit* pGameUnit( getUnit( Event.UnitID_ ) );

	if( pGameUnit != NULL )
	{
		pGameUnit->setBehaviourIdle();
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onUnitGuard
eEvtReturn GaGameSimulator::onUnitGuard( EvtID ID, const GaGameUnitGuardEvent& Event )
{
	GaGameUnit* pGameUnit( getUnit( Event.UnitID_ ) );

	if( pGameUnit != NULL )
	{
		pGameUnit->setBehaviourIdle();
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onUnitMove
eEvtReturn GaGameSimulator::onUnitMove( EvtID ID, const GaGameUnitMoveEvent& Event )
{
	GaGameUnit* pGameUnit( getUnit( Event.UnitID_ ) );

	if( pGameUnit != NULL )
	{
		pGameUnit->setBehaviourMove( Event.Position_, Event.IsAttackMove_ );
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onUnitAttack
eEvtReturn GaGameSimulator::onUnitAttack( EvtID ID, const GaGameUnitAttackEvent& Event )
{
	GaGameUnit* pGameUnit( getUnit( Event.UnitID_ ) );

	if( pGameUnit != NULL )
	{
		pGameUnit->setBehaviourAttack( Event.TargetUnitID_ );
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// addDebugPoint
void GaGameSimulator::addDebugPoint( const BcFixedVec2d& Position, BcFixed Size, const RsColour& Colour )
{
	TDebugPoint DebugPoint = 
	{
		Position,
		Size,
		Colour,
		1.0f
	};

	DebugPoints_.push_back( DebugPoint );
}

//////////////////////////////////////////////////////////////////////////
// runAI
void GaGameSimulator::runAI( BcU32 TeamID )
{
	// Skill (our of 32)
	BcU32 Skill = 16;

	// Find average centre of units.
	BcFixedVec2d Centre;
	BcFixed Total;

	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		if( pGameUnit->getTeamID() == TeamID && !pGameUnit->isDead() )
		{
			Centre += pGameUnit->getPosition();
			Total += 1.0f;
		}
	}

	if( Total > 0.0f )
	{
		Centre /= Total;
	}

	// Prio 1: Attack nearest unit to centre of group.
	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		if( ( rand() % 32 ) < Skill )
		{
			if( pGameUnit->getTeamID() == TeamID && !pGameUnit->isDead() )
			{
				BcU32 NearestID = findNearestUnit( pGameUnit->getPosition(), BcErrorCode, 1 << ( 1 - TeamID ) );
				GaGameUnit* pNearestUnit = getUnit( NearestID );
				if( pNearestUnit != NULL )
				{
					pGameUnit->setBehaviourAttack( NearestID, BcFalse );
				}
			}
		}
	}

	// Prio 2: Dodge projectiles that move slow.
	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		if( ( rand() % 32 ) < Skill )
		{
			if( pGameUnit->getTeamID() == TeamID && !pGameUnit->isDead() )
			{
				GaGameUnitIDList Projectiles;
				findProjectiles( Projectiles );

				for( BcU32 ProjIdx = 0; ProjIdx < Projectiles.size(); ++ProjIdx )
				{
					GaGameUnit* pProjectileUnit = getUnit( Projectiles[ ProjIdx ] );
					if( pProjectileUnit != NULL )
					{
						BcFixedVec2d Direction = ( pGameUnit->getPosition() - pProjectileUnit->getMoveTargetPosition() );
						BcFixed MagnitudeSquared = Direction.magnitudeSquared();
						BcFixed RangeSquared = pProjectileUnit->getDesc().Range_ * pProjectileUnit->getDesc().Range_;

						if( MagnitudeSquared == 0.0f )
						{
							Direction = BcFixedVec2d( (int)( rand() % 32 - 16 ), (int)( rand() % 32 - 16 ) );
						}

						if( MagnitudeSquared < RangeSquared && pProjectileUnit->getDesc().MoveSpeed_ < 20.0f )
						{
							pGameUnit->setBehaviourMove( pGameUnit->getPosition() + ( Direction.normal() * BcSqrt( RangeSquared )  ), BcFalse, BcFalse );
						}
					}
				}
			}
		}
	}
}
