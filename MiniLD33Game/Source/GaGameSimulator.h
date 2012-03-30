/**************************************************************************
*
* File:		GaGameSimulator.h
* Author:	Neil Richardson 
* Ver/Date:	24/03/12
* Description:
*		Game unit.
*		
*
*
* 
**************************************************************************/

#ifndef __GAGAMESIMULATOR_H__
#define __GAGAMESIMULATOR_H__

#include "Psybrus.h"

#include "GaGameUnit.h"

//////////////////////////////////////////////////////////////////////////
// GaGameSimulator
class GaGameSimulator:
	public EvtPublisher
{
public:
	GaGameSimulator( BcFixed SimulationRate, BcFixed SimulationSpeed, BcU32 TeamID, BcBool Networked );
	virtual ~GaGameSimulator();

	BcFixed getSimulationRate() const;

	GaGameUnit* addUnit( const GaGameUnitDescriptor& Desc, BcU32 TeamID, const BcFixedVec2d& Position );
	GaGameUnit* getUnit( BcU32 ID );
	
	BcU32 findNearestUnit( const BcFixedVec2d& Position, BcU32 IgnoreID = BcErrorCode, BcU32 TeamIDMask = BcErrorCode );
	void findUnits( GaGameUnitIDList& List, const BcFixedVec2d& Position, BcFixed Radius, BcU32 IgnoreID = BcErrorCode, BcU32 TeamIDMask = BcErrorCode );
	void findUnits( GaGameUnitIDList& List, const BcFixedVec2d& Start, const BcFixedVec2d& End, BcU32 IgnoreID = BcErrorCode, BcU32 TeamIDMask = BcErrorCode );
	void applyDamage( const BcFixedVec2d& Position, BcFixed Range, BcFixed Amount );

	void findProjectiles( GaGameUnitIDList& List );

	BcU32 rand();

	void tick( BcReal Delta );

	void render( ScnCanvasComponentRef Canvas, BcU32 TeamID );
	void renderHUD( ScnCanvasComponentRef Canvas, const GaGameUnitIDList& CurrentSelection, BcU32 TeamID );

	void addDebugPoint( const BcFixedVec2d& Position, BcFixed Size, const RsColour& Colour );

	void runAI( BcU32 TeamID );

	BcReal getTimeOut() const;
	BcU32 getUnitCount( BcU32 TeamID ) const;

private:
	eEvtReturn onUnitIdle( EvtID ID, const GaGameUnitIdleEvent& Event );
	eEvtReturn onUnitGuard( EvtID ID, const GaGameUnitGuardEvent& Event );
	eEvtReturn onUnitMove( EvtID ID, const GaGameUnitMoveEvent& Event );
	eEvtReturn onUnitAttack( EvtID ID, const GaGameUnitAttackEvent& Event );

private:
	class EvtProxyLockstep* pEventProxy_;
	class EvtBridgeRakNet* pEventBridge_;

	BcFixed SimulationRate_;
	BcFixed SimulationSpeed_;
	BcFixed TickAccumulator_;
	BcU32 CurrentFrame_;
	BcU32 UnitID_;

	typedef std::map< BcU32, GaGameUnit* > TGameUnitMap;
	typedef std::vector< GaGameUnit* > TGameUnitList;

	struct TDebugPoint
	{
		BcFixedVec2d Position_;
		BcFixed Size_;
		RsColour Colour_;
		BcFixed Timer_;
	};

	std::vector< TDebugPoint > DebugPoints_;
	
	TGameUnitMap GameUnitMap_;
	TGameUnitList GameUnitList_;

	BcU32 Checksum_;

	BcRandom RNG_;

	BcReal TimeOut_;
	
	
};

#endif
