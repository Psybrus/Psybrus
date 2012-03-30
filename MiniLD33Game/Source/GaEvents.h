/**************************************************************************
*
* File:		GaEvents.h
* Author:	Neil Richardson 
* Ver/Date:	24/03/12
* Description:
*		Game events.
*		
*
*
* 
**************************************************************************/

#ifndef __GAEVENTS_H__
#define __GAEVENTS_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Event groups
#define GA_EVENTGROUP_UNIT	EVT_MAKE_ID( 'G', 'a', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum GaEvents
{
	// All unit events.
	gaEVT_UNIT_FIRST			= GA_EVENTGROUP_UNIT,
	gaEVT_UNIT_IDLE,
	gaEVT_UNIT_GUARD,
	gaEVT_UNIT_MOVE,
	gaEVT_UNIT_ATTACK
};

//////////////////////////////////////////////////////////////////////////
// GaGameEvent
template< typename _Ty >
struct GaGameUnitEvent: EvtEvent< _Ty >
{
	BcU32 UnitID_;
};

struct GaGameUnitIdleEvent: GaGameUnitEvent< GaGameUnitIdleEvent >
{
};

struct GaGameUnitGuardEvent: GaGameUnitEvent< GaGameUnitGuardEvent >
{
};

struct GaGameUnitMoveEvent: GaGameUnitEvent< GaGameUnitMoveEvent >
{
	BcFixedVec2d Position_;
	BcBool IsAttackMove_;
};

struct GaGameUnitAttackEvent: GaGameUnitEvent< GaGameUnitAttackEvent >
{
	BcU32 TargetUnitID_;
};


#endif
