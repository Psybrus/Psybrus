#pragma once

#include "Events/EvtEvent.h"

/**
 * Event groups.
 */
#define NS_EVENTGROUP_CORE		EVT_MAKE_ID( 'N', 's', 0 )
#define NS_EVENTGROUP_SESSION	EVT_MAKE_ID( 'N', 's', 1 )
#define NS_EVENTGROUP_PEER		EVT_MAKE_ID( 'N', 's', 2 )

/**
 * @brief Network events.
 */
enum class NsEvent
{
	// All core events.
	CORE_FIRST			= NS_EVENTGROUP_CORE,
	CORE_LAST,
	
	// All session events.
	SESSION_FIRST,
	SESSION_DISCONNECTED,
	SESSION_CONNECTING,
	SESSION_CONNECTED,
	SESSION_DISCONNECTING,
	SESSION_ERROR,
	SESSION_LAST,

	// All peer events.
	PEER_FIRST,
	PEER_LAST,
	
	//
	MAX
};

/**
 * @brief Core network event.
 */
struct NsEventCore: EvtEvent< NsEventCore >
{
	
};

/**
 * @brief Session network event.
 */
struct NsEventSession: EvtEvent< NsEventSession >
{
	
};

