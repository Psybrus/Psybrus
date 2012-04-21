/**************************************************************************
*
* File:		GaEvents.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
* Description:
*		Events.
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
#define GA_EVENTGROUP_STRONGFORCE		EVT_MAKE_ID( 'G', 'a', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum OsEvents
{
	// All core events.
	gaEVT_STRONGFORCE_FIRST			= GA_EVENTGROUP_STRONGFORCE,
	gaEVT_STRONGFORCE_BEGIN_CHARGING,
	gaEVT_STRONGFORCE_ACTIVATE,
	gaEVT_STRONGFORCE_LAST,

};

////////////////////////////////////////////////////////////////////////////////
// GaEventStrongForceBeginCharging
struct GaEventStrongForceBeginCharging: EvtEvent< GaEventStrongForceBeginCharging >
{
	BcVec3d Position_;

};

////////////////////////////////////////////////////////////////////////////////
// GaEventStrongForceActivate
struct GaEventStrongForceActivate: EvtEvent< GaEventStrongForceActivate >
{
	BcVec3d Position_;

};

#endif
