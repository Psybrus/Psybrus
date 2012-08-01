/**************************************************************************
*
* File:		EvtEvent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Events.
*		
*		
*
* 
**************************************************************************/

#ifndef __EVTEVENT_H__
#define __EVTEVENT_H__

#include "Base/BcDelegate.h"
#include "Base/BcMisc.h"

////////////////////////////////////////////////////////////////////////////////
// Typedefs
typedef BcU32 EvtID;

////////////////////////////////////////////////////////////////////////////////
/**	\brief Make event ID
* 
*	In an attempt to group up IDs, this macro is provided to generate an ID
*	for a particular system/subsystem within the engine and game.							<br/>
*
*	The first parameters, _prefixh & _prefixl, should be characters single quoted.			<br/>
*	Examples are: 'R' & 's' (Render system), 'S' & 's' (Sound system),
*   'O' & 's' (Operating system).															<br/><br/>
*	
*	The second parameter, _group, should be a value no greater than 0xff, and
*	is used to split off IDs into groups of 256 values.										<br/><br/>
*
*	Practical example of usage:																<br/><br/>
*
*	#define OS_EVENTGROUP_CORE		EVT_MAKE_ID( 'O', 's', 0 )								<br/>
*	#define OS_EVENTGROUP_INPUT		EVT_MAKE_ID( 'O', 's', 1 )								<br/>
*																							<br/>
*	enum OsEvents																			<br/>
*	{																						<br/>
*		// All core events.																	<br/>
*		osEVT_CORE_FIRST			= OS_EVENTGROUP_CORE,									<br/>
*		osEVT_CORE_QUIT_SIGNALLED,															<br/>
*		osEVT_CORE_LAST,																	<br/>
*																							<br/>
*		// All input events.																<br/>
*		osEVT_INPUT_FIRST			= OS_EVENTGROUP_INPUT,									<br/>
*		osEVT_INPUT_MOUSEMOVE,																<br/>
*		osEVT_INPUT_MOUSEDOWN,																<br/>
*		osEVT_INPUT_MOUSEUP,																<br/>
*		osEVT_INPUT_KEYDOWN,																<br/>
*		osEVT_INPUT_KEYUP,																	<br/>
*		osEVT_INPUT_LAST,																	<br/>
*																							<br/>
*		//																					<br/>
*		osEVT_MAX																			<br/>
*	};																						<br/>
*/
#define EVT_MAKE_ID( _prefixh, _prefixl, _group ) EvtID( ( BcU32( _prefixh & 0xff ) << 24 ) | ( BcU32( _prefixl & 0xff ) << 16 ) | ( BcU32( _group & 0xff ) << 8 ) )

////////////////////////////////////////////////////////////////////////////////
// Event Return
enum eEvtReturn
{
	evtRET_PASS = 0,			///!< Pass the event on to next handler.
	evtRET_BLOCK,				///!< Block event from reaching next handler.
	evtRET_REMOVE				///!< Unsubscribe from any further calls from current event ID
};

////////////////////////////////////////////////////////////////////////////////
/**	\class EvtBaseEvent
*	\brief Used to allow events to be passed through without templating.
*
*/
struct EvtBaseEvent
{

};

////////////////////////////////////////////////////////////////////////////////
/**	\class EvtEvent
*	\brief Derive from this with sub class as template parameter to implement an event of any size.
*	       
*	Events of almost any size (depending on publisher's internals) can be published.
*	An event should only contain basic types that can be shallow copied, i.e, are pure binary data.
*
*	Example of a VALID event:
*
*	struct MyStringEvent: EvtEvent< MyStringEvent >
*	{
*		BcChar String_[ 64 ];			///!< Copies safely via memcpy.
*	};
*
*	Now, an example of an INVALID event:
*
*	struct MyStringEvent: EvtEvent< MyStringEvent >
*	{
*		std::string String_;			///!< Requires a call to it's copy constructor.
*	};
*/
template< typename _Ty >
struct EvtEvent: EvtBaseEvent
{
public:
	typedef BcDelegate< eEvtReturn(*)( EvtID, const _Ty& ) > Delegate;

};

////////////////////////////////////////////////////////////////////////////////
/**	\class EvtNullEvent
*	\brief Null event implementation
*
*/
struct EvtNullEvent: EvtEvent< EvtNullEvent >
{
	
};

#endif
