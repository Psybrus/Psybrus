/**************************************************************************
*
* File:		AkEvents.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Audiokinetic Wwise Events.
*		
*
*
* 
**************************************************************************/

#ifndef __AKEVENTS_H__
#define __AKEVENTS_H__

#include "Events/EvtEvent.h"

#include <AK/SoundEngine/Common/AkTypes.h>

////////////////////////////////////////////////////////////////////////////////
// Event groups
#define AK_EVENTGROUP_CORE		EVT_MAKE_ID( 'A', 'k', 0 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum AkEvents
{
	// All core events.
	akEVT_CORE_FIRST			= AK_EVENTGROUP_CORE,

	akEVT_CORE_POST,			// Post event.
	akEVT_CORE_SETRTPC,			// Set RTPC.

	//
	akEVT_MAX
};

////////////////////////////////////////////////////////////////////////////////
// AkEventPost
struct AkEventPost: EvtEvent< AkEventPost >
{
	AkEventPost( AkUniqueID ID ):
		ID_( ID )
	{}

	AkUniqueID ID_;
};

////////////////////////////////////////////////////////////////////////////////
// AkEventSetRTPC
struct AkEventSetRTPC: EvtEvent< AkEventSetRTPC >
{
	AkEventSetRTPC( AkUniqueID ID, BcReal Value ):
		ID_( ID ),
		Value_( Value )
	{}

	AkUniqueID ID_;
	BcReal Value_;
};

#endif
