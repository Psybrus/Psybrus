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

#include "EvtBridgeIRC.h"
#include "EvtProxyLockstep.h"

#include "GaGameSimulator.h"

//////////////////////////////////////////////////////////////////////////
// GaGameSimulator
GaGameSimulator::GaGameSimulator( BcFixed SimulationRate, BcFixed SimulationSpeed ):
	SimulationRate_( SimulationRate ),
	SimulationSpeed_( SimulationSpeed ),
	TickAccumulator_( 0.0f ),
	CurrentFrame_( 0 ),
	UnitID_( 0 )
{
	GaGameUnitIdleEvent::Delegate OnUnitIdle( GaGameUnitIdleEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitIdle >( this ) );
	GaGameUnitGuardEvent::Delegate OnUnitGuard( GaGameUnitGuardEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitGuard >( this ) );
	GaGameUnitMoveEvent::Delegate OnUnitMove( GaGameUnitMoveEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitMove >( this ) );
	GaGameUnitAttackEvent::Delegate OnUnitAttack( GaGameUnitAttackEvent::Delegate::bind< GaGameSimulator, &GaGameSimulator::onUnitAttack >( this ) );

	subscribe( gaEVT_UNIT_IDLE, OnUnitIdle );
	subscribe( gaEVT_UNIT_GUARD, OnUnitGuard );
	subscribe( gaEVT_UNIT_MOVE, OnUnitMove );
	subscribe( gaEVT_UNIT_ATTACK, OnUnitAttack );

	pEventProxy_ = new EvtProxyLockstep( this );
	pEventBridge_ = new EvtBridgeIRC( this );
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
void GaGameSimulator::tick( BcReal Delta )
{
	TickAccumulator_ += Delta * SimulationSpeed_;
	CurrentFrame_ += 1;

	while( TickAccumulator_ > SimulationRate_ )
	{
		BcFixed SimulationRate = SimulationRate_;
		TickAccumulator_ -= SimulationRate;
		
		// Setup proxy as required.
		pEventProxy_->dispatchFrameIndex( CurrentFrame_ );
		pEventProxy_->setFrameIndex( CurrentFrame_ + 8 );
		
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
	}
}

//////////////////////////////////////////////////////////////////////////
// render
void GaGameSimulator::render( ScnCanvasComponentRef Canvas )
{
	for( BcU32 Idx = 0; Idx < DebugPoints_.size(); ++Idx )
	{
		TDebugPoint& DebugPoint = DebugPoints_[ Idx ];

		if( DebugPoint.Timer_ > 0.0f )
		{		
			BcVec2d Position = BcVec2d( DebugPoint.Position_.x(), DebugPoint.Position_.y() ) * 32.0f;
			BcVec2d Size = BcVec2d( DebugPoint.Size_, DebugPoint.Size_ ) * 32.0f;

			Canvas->drawSpriteCentered( Position, Size, 0, DebugPoint.Colour_ * RsColour( 1.0f, 1.0f, 1.0f, DebugPoint.Timer_ ), 1000 );

			DebugPoint.Timer_ -= BcReal( 0.05f );
		}
	}

	for( BcU32 Idx = 0; Idx < GameUnitList_.size(); ++Idx )
	{
		GaGameUnit* pGameUnit = GameUnitList_[ Idx ];
		pGameUnit->render( Canvas );
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
		pGameUnit->setBehaviourMove( Event.Position_ );
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
