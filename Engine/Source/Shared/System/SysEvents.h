/**************************************************************************
*
* File:		OsEvents.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Os Events
*		
*
*
* 
**************************************************************************/

#ifndef __SYSEVENTS_H__
#define __SYSEVENTS_H__

#include "EvtEvent.h"

////////////////////////////////////////////////////////////////////////////////
// Event groups
#define SYS_EVENTGROUP_SYSTEM	EVT_MAKE_ID( 'Sy', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum SysEvents
{
	// All core events.
	sysEVT_SYSTEM_FIRST			= SYS_EVENTGROUP_SYSTEM,
	sysEVT_SYSTEM_PRE_OPEN,
	sysEVT_SYSTEM_POST_OPEN,
	sysEVT_SYSTEM_PRE_UPDATE,
	sysEVT_SYSTEM_POST_UPDATE,
	sysEVT_SYSTEM_PRE_CLOSE,
	sysEVT_SYSTEM_POST_CLOSE,
	
	// Last for platform specifics.
	sysEVT_SYSTEM_LAST,
	
	//
	sysEVT_MAX
};

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class SysSystem;

////////////////////////////////////////////////////////////////////////////////
// SysSystemEvent
struct SysSystemEvent: EvtEvent< SysSystemEvent >
{
	SysSystemEvent( SysSystem* pSystem ):
		pSystem_( pSystem )
	{

	}
	
	SysSystem*		pSystem_;
};

#endif
