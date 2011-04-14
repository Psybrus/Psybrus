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

#ifndef __OSEVENTS_H__
#define __OSEVENTS_H__

#include "EvtEvent.h"

////////////////////////////////////////////////////////////////////////////////
// Event groups
#define OS_EVENTGROUP_CORE		EVT_MAKE_ID( 'Os', 0 )
#define OS_EVENTGROUP_INPUT		EVT_MAKE_ID( 'Os', 1 )

////////////////////////////////////////////////////////////////////////////////
// Event enum
enum OsEvents
{
	// All core events.
	osEVT_CORE_FIRST			= OS_EVENTGROUP_CORE,
	osEVT_CORE_QUIT,
	osEVT_CORE_LAST,
	
	// All input events.
	osEVT_INPUT_FIRST			= OS_EVENTGROUP_INPUT,
	osEVT_INPUT_MOUSEMOVE,
	osEVT_INPUT_MOUSEDOWN,		
	osEVT_INPUT_MOUSEUP,
	osEVT_INPUT_KEYDOWN,
	osEVT_INPUT_KEYUP,
	osEVT_INPUT_LAST,
	osEVT_INPUT_TOUCHDOWN,
	osEVT_INPUT_TOUCHUP,
	osEVT_INPUT_TOUCHMOVE,
	
	//
	osEVT_MAX
};

////////////////////////////////////////////////////////////////////////////////
// OsEventCore
struct OsEventCore: EvtEvent< OsEventCore >
{
	
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInput
template< typename _Ty >
struct OsEventInput: EvtEvent< _Ty >
{
	BcU16			DeviceID_;			///!< Device that the input has came from.
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMouse
struct OsEventInputMouse: OsEventInput< OsEventInputMouse >
{
	BcS16			MouseX_;			///!< X coordinate of mouse relative to window.
	BcS16			MouseY_;			///!< Y coordinate of mouse relative to window.
	BcS16			ButtonCode_;		///!< Button code for event (-1 for not used)
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputKeyboard
struct OsEventInputKeyboard: OsEventInput< OsEventInputKeyboard >
{
	BcU16			KeyCode_;			///!< Key code.
};

#endif
