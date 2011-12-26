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
#define OS_EVENTGROUP_CORE		EVT_MAKE_ID( 'O', 's', 0 )
#define OS_EVENTGROUP_INPUT		EVT_MAKE_ID( 'O', 's', 1 )

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
	osEVT_INPUT_TOUCHDOWN,
	osEVT_INPUT_TOUCHUP,
	osEVT_INPUT_TOUCHMOVE,
	osEVT_INPUT_MIDI,
	osEVT_INPUT_LAST,

	// All client events.
	osEVT_CLIENT_RESIZE,
	osEVT_CLIENT_CLOSE,
	osEVT_CLIENT_MINIMIZE,
	osEVT_CLIENT_MAXIMIZE,
	
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

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI
template< typename _Ty >
struct OsEventInputMIDI: OsEventInput< _Ty >
{
	BcU32			FullMessage_;		///!< Full MIDI message.
	BcU32			Timestamp_;			///!< Timestamp for message.
	
	/**
	 * Get message.
	 */
	BcForceInline BcU32 getMessage() const
	{
		return ( ( FullMessage_ >> 4 ) & 0xf );
	}
	
	/**
	 * Get channel.
	 */
	BcForceInline BcU32 getChannel() const
	{
		return ( FullMessage_ & 0xf );
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI_NoteOff
struct OsEventInputMIDI_NoteOff: OsEventInputMIDI< OsEventInputMIDI_NoteOff >
{
	enum
	{
		MESSAGE = 0x8
	};
	
	/**
	 * Get note.
	 */
	BcForceInline BcU32 getNote() const
	{
		return ( FullMessage_ >> 8 ) & 0x7f;
	}

	/**
	 * Get velocity.
	 */
	BcForceInline BcU32 getVelocity() const
	{
		return ( FullMessage_ >> 16 ) & 0x7f;
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI_NoteOn
struct OsEventInputMIDI_NoteOn: OsEventInputMIDI< OsEventInputMIDI_NoteOn >
{
	enum
	{
		MESSAGE = 0x9
	};
	
	/**
	 * Get note.
	 */
	BcForceInline BcU32 getNote() const
	{
		return ( FullMessage_ >> 8 ) & 0x7f;
	}
	
	/**
	 * Get velocity.
	 */
	BcForceInline BcU32 getVelocity() const
	{
		return ( FullMessage_ >> 16 ) & 0x7f;
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI_PolyphonicKeyPressure
struct OsEventInputMIDI_PolyphonicKeyPressure: OsEventInputMIDI< OsEventInputMIDI_PolyphonicKeyPressure >
{
	enum
	{
		MESSAGE = 0xa
	};
	
	/**
	 * Get note.
	 */
	BcForceInline BcU32 getNote() const
	{
		return ( FullMessage_ >> 8 ) & 0x7f;
	}
	
	/**
	 * Get pressure.
	 */
	BcForceInline BcU32 getPressure() const
	{
		return ( FullMessage_ >> 16 ) & 0x7f;
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI_ControlChange
struct OsEventInputMIDI_ControlChange: OsEventInputMIDI< OsEventInputMIDI_ControlChange >
{
	enum
	{
		MESSAGE = 0xb
	};
	
	/**
	 * Get controller.
	 */
	BcForceInline BcU32 getController() const
	{
		return ( FullMessage_ >> 8 ) & 0x7f;
	}
	
	/**
	 * Get value.
	 */
	BcForceInline BcU32 getValue() const
	{
		return ( FullMessage_ >> 16 ) & 0x7f;
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI_ProgramChange
struct OsEventInputMIDI_ProgramChange: OsEventInputMIDI< OsEventInputMIDI_ProgramChange >
{
	enum
	{
		MESSAGE = 0xc
	};
	
	/**
	 * Get program.
	 */
	BcForceInline BcU32 getProgram() const
	{
		return ( FullMessage_ >> 8 ) & 0x7f;
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI_ChannelPressure
struct OsEventInputMIDI_ChannelPressure: OsEventInputMIDI< OsEventInputMIDI_ChannelPressure >
{
	enum
	{
		MESSAGE = 0xd
	};
	
	/**
	 * Get pressure.
	 */
	BcForceInline BcU32 getPressure() const
	{
		return ( FullMessage_ >> 8 ) & 0x7f;
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputMIDI_PitchWheelChange
struct OsEventInputMIDI_PitchWheelChange: OsEventInputMIDI< OsEventInputMIDI_PitchWheelChange >
{
	enum
	{
		MESSAGE = 0xe
	};
	
	/**
	 * Get shift.
	 */
	BcForceInline BcU32 getShift() const
	{
		return ( ( ( FullMessage_ >> 16 ) & 0x7f ) << 7 ) | ( ( FullMessage_ >> 8 ) & 0x7f );
	}
};

////////////////////////////////////////////////////////////////////////////////
// OsEventClientBase
template< typename _Ty >
struct OsEventClientBase: EvtEvent< _Ty >
{
	class OsClient*	pClient_; // Client sending the event.
};
////////////////////////////////////////////////////////////////////////////////
// OsEventClient
struct OsEventClient: OsEventClientBase< OsEventClient >
{
};

////////////////////////////////////////////////////////////////////////////////
// OsEventClientResize
struct OsEventClientResize: OsEventClientBase< OsEventClientResize >
{
	BcU32			Width_;		// Width of client.
	BcU32			Height_;	// Height of client.
};

#endif
