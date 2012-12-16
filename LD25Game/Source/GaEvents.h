/**************************************************************************
*
* File:		GaEvents.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Enemy component.
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
#define GA_EVENTGROUP_CORE		EVT_MAKE_ID( 'G', 'a', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum GaEvents
{
	gaEVT_CORE_FIRST				= GA_EVENTGROUP_CORE,
	gaEVT_PLAYER_SHOOT,
	gaEVT_PLAYER_POSITION,
	gaEVT_PLAYER_DIE,

	gaEVT_ENEMY_SHOOT,
	gaEVT_ENEMY_POSITION,
	gaEVT_ENEMY_DIE,

	gaEVT_PROJECTILE_POSITION,
	gaEVT_PROJECTILE_KILL,

	gaEVT_CORE_LAST
};

////////////////////////////////////////////////////////////////////////////////
// GaEventShoot
struct GaEventShoot: EvtEvent< GaEventShoot >
{
	ScnComponent* pSender_;
	BcVec3d Position_;
};

////////////////////////////////////////////////////////////////////////////////
// GaEventPosition
struct GaEventPosition: EvtEvent< GaEventPosition >
{
	ScnComponent* pSender_;
	BcVec3d Position_;
};

#endif
