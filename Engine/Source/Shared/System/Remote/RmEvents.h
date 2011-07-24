/**************************************************************************
*
* File:		RmEvents.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Rm Events
*		
*
*
* 
**************************************************************************/

#ifndef __RMEVENTS_H__
#define __RMEVENTS_H__

#include "EvtEvent.h"

////////////////////////////////////////////////////////////////////////////////
// Event groups
#define RM_EVENTGROUP_CORE		EVT_MAKE_ID( 'R', 'm', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum RmEvents
{
	// All core events.
	rmEVT_CORE_FIRST			= RM_EVENTGROUP_CORE,
	
	// Connect events.
	rmEVT_CONNECT_SUCCEEDED,
	rmEVT_CONNECT_FAILED,

	// Listen events.
	rmEVT_LISTEN_SUCCEEDED,
	rmEVT_LISTEN_FAILED,

	// Connection events.
	rmEVT_CONNECTION_CONNECTED,
	rmEVT_CONNECTION_DISCONNECTED,

	//
	rmEVT_MAX
};

////////////////////////////////////////////////////////////////////////////////
// RmEventCore
struct RmEventCore: EvtEvent< RmEventCore >
{
	
};

#endif
