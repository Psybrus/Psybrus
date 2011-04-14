/**************************************************************************
*
* File:		RsEvents.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Rs Events
*		
*
*
* 
**************************************************************************/

#ifndef __RSEVENTS_H__
#define __RSEVENTS_H__

#include "EvtEvent.h"

////////////////////////////////////////////////////////////////////////////////
// Event groups
#define RS_EVENTGROUP_CORE		EVT_MAKE_ID( 'Rs', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum RsEvents
{
	// All core events.
	rsEVT_CORE_FIRST			= RS_EVENTGROUP_CORE,

	//
	rsEVT_MAX
};

#endif
