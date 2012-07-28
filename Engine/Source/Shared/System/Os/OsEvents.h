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

#include "Events/EvtEvent.h"

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
	BcReal			MouseDX_;			///!< Delta X.
	BcReal			MouseDY_;			///!< Delta Y.
	BcReal			NormalisedX_;		///!< X in -1.0 to 1.0
	BcReal			NormalisedY_;		///!< Y in -1.0 to 1.0
	BcS16			ButtonCode_;		///!< Button code for event (-1 for not used)
};

////////////////////////////////////////////////////////////////////////////////
// OsEventInputKeyboard
struct OsEventInputKeyboard: OsEventInput< OsEventInputKeyboard >
{
	enum
	{
		KEYCODE_FIRST = 255,
		KEYCODE_LBUTTON,
		KEYCODE_RBUTTON,
		KEYCODE_CANCEL,
		KEYCODE_MBUTTON,
		KEYCODE_BACKSPACE,
		KEYCODE_TAB,
		KEYCODE_CLEAR,
		KEYCODE_RETURN,
		KEYCODE_SHIFT,
		KEYCODE_CONTROL,
		KEYCODE_ALT,
		KEYCODE_PAUSE,
		KEYCODE_CAPSLOCK,
		KEYCODE_ESCAPE,
		KEYCODE_SPACE,
		KEYCODE_PGUP,
		KEYCODE_PGDN,
		KEYCODE_END,
		KEYCODE_HOME,
		KEYCODE_LEFT,
		KEYCODE_UP,
		KEYCODE_RIGHT,
		KEYCODE_DOWN,
		KEYCODE_SELECT,
		KEYCODE_PRINT,
		KEYCODE_EXECUTE,
		KEYCODE_PRINT_SCREEN,
		KEYCODE_INSERT,
		KEYCODE_DELETE,
		KEYCODE_HELP,
		KEYCODE_NUMPAD0,
		KEYCODE_NUMPAD1,
		KEYCODE_NUMPAD2,
		KEYCODE_NUMPAD3,
		KEYCODE_NUMPAD4,
		KEYCODE_NUMPAD5,
		KEYCODE_NUMPAD6,
		KEYCODE_NUMPAD7,
		KEYCODE_NUMPAD8,
		KEYCODE_NUMPAD9,
		KEYCODE_SEPARATOR,
		KEYCODE_SUBTRACT,
		KEYCODE_DECIMAL,
		KEYCODE_DIVIDE,
		KEYCODE_F1,
		KEYCODE_F2,
		KEYCODE_F3,
		KEYCODE_F4,
		KEYCODE_F5,
		KEYCODE_F6,
		KEYCODE_F7,
		KEYCODE_F8,
		KEYCODE_F9,
		KEYCODE_F10,
		KEYCODE_F11,
		KEYCODE_F12,
		KEYCODE_F13,
		KEYCODE_F14,
		KEYCODE_F15,
		KEYCODE_F16,
		KEYCODE_F17,
		KEYCODE_F18,
		KEYCODE_F19,
		KEYCODE_F20,
		KEYCODE_F21,
		KEYCODE_F22,
		KEYCODE_F23,
		KEYCODE_F24,
		KEYCODE_NUMLOCK,
		KEYCODE_SCROLL,
		KEYCODE_LSHIFT,
		KEYCODE_RSHIFT,
		KEYCODE_LCONTROL,
		KEYCODE_RCONTROL,
		KEYCODE_LMENU,
		KEYCODE_RMENU,
		KEYCODE_PLAY,
		KEYCODE_ZOOM
	};

	BcU16			KeyCode_;			///!< Key code.
	BcU16			ScanCode_;			///!< Scan code.
	BcU16			AsciiCode_;			///!< ASCII code.
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
