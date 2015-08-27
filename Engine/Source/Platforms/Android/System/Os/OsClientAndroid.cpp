/**************************************************************************
*
* File:		OsClientAndroid.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#include "System/Os/OsClientAndroid.h"

#include "System/Os/OsCore.h"
#include "System/SysKernel.h"

#include "Base/BcString.h"
#include "Base/BcMath.h"

#include <android_native_app_glue.h>
#include <android/input.h>
#include <android/keycodes.h>
#include <android/window.h>


//////////////////////////////////////////////////////////////////////////
// Ctor
OsClientAndroid::OsClientAndroid( android_app* App ):
	App_( App ),
	Window_( nullptr )
{
	// Setup keycode map.
	KeyCodeMap_[ AKEYCODE_TAB ] = OsEventInputKeyboard::KEYCODE_TAB;
	KeyCodeMap_[ AKEYCODE_ENTER ] = OsEventInputKeyboard::KEYCODE_RETURN;
	KeyCodeMap_[ AKEYCODE_CTRL_LEFT ] = OsEventInputKeyboard::KEYCODE_CONTROL;	
	KeyCodeMap_[ AKEYCODE_SHIFT_LEFT ] = OsEventInputKeyboard::KEYCODE_SHIFT;	
	KeyCodeMap_[ AKEYCODE_CTRL_RIGHT ] = OsEventInputKeyboard::KEYCODE_CONTROL;	
	KeyCodeMap_[ AKEYCODE_SHIFT_RIGHT ] = OsEventInputKeyboard::KEYCODE_SHIFT;	
	KeyCodeMap_[ AKEYCODE_ALT_LEFT ] = OsEventInputKeyboard::KEYCODE_ALT;
	KeyCodeMap_[ AKEYCODE_ALT_RIGHT ] = OsEventInputKeyboard::KEYCODE_ALT;

	KeyCodeMap_[ AKEYCODE_ESCAPE ] = OsEventInputKeyboard::KEYCODE_ESCAPE;
	KeyCodeMap_[ AKEYCODE_SPACE ] = OsEventInputKeyboard::KEYCODE_SPACE;
	KeyCodeMap_[ AKEYCODE_HOME ] = OsEventInputKeyboard::KEYCODE_HOME;
	KeyCodeMap_[ AKEYCODE_INSERT ] = OsEventInputKeyboard::KEYCODE_INSERT;
	KeyCodeMap_[ AKEYCODE_NUMPAD_0 ] = OsEventInputKeyboard::KEYCODE_NUMPAD0;
	KeyCodeMap_[ AKEYCODE_NUMPAD_1 ] = OsEventInputKeyboard::KEYCODE_NUMPAD1;
	KeyCodeMap_[ AKEYCODE_NUMPAD_2 ] = OsEventInputKeyboard::KEYCODE_NUMPAD2;
	KeyCodeMap_[ AKEYCODE_NUMPAD_3 ] = OsEventInputKeyboard::KEYCODE_NUMPAD3;
	KeyCodeMap_[ AKEYCODE_NUMPAD_4 ] = OsEventInputKeyboard::KEYCODE_NUMPAD4;
	KeyCodeMap_[ AKEYCODE_NUMPAD_5 ] = OsEventInputKeyboard::KEYCODE_NUMPAD5;
	KeyCodeMap_[ AKEYCODE_NUMPAD_6 ] = OsEventInputKeyboard::KEYCODE_NUMPAD6;
	KeyCodeMap_[ AKEYCODE_NUMPAD_7 ] = OsEventInputKeyboard::KEYCODE_NUMPAD7;
	KeyCodeMap_[ AKEYCODE_NUMPAD_8 ] = OsEventInputKeyboard::KEYCODE_NUMPAD8;
	KeyCodeMap_[ AKEYCODE_NUMPAD_9 ] = OsEventInputKeyboard::KEYCODE_NUMPAD9;
	KeyCodeMap_[ AKEYCODE_NUMPAD_SUBTRACT ] = OsEventInputKeyboard::KEYCODE_SUBTRACT;
	KeyCodeMap_[ AKEYCODE_NUMPAD_DOT ] = OsEventInputKeyboard::KEYCODE_DECIMAL;
	KeyCodeMap_[ AKEYCODE_NUMPAD_DIVIDE ] = OsEventInputKeyboard::KEYCODE_DIVIDE;

	KeyCodeMap_[ AKEYCODE_F1 ] = OsEventInputKeyboard::KEYCODE_F1;
	KeyCodeMap_[ AKEYCODE_F2 ] = OsEventInputKeyboard::KEYCODE_F2;
	KeyCodeMap_[ AKEYCODE_F3 ] = OsEventInputKeyboard::KEYCODE_F3;
	KeyCodeMap_[ AKEYCODE_F4 ] = OsEventInputKeyboard::KEYCODE_F4;
	KeyCodeMap_[ AKEYCODE_F5 ] = OsEventInputKeyboard::KEYCODE_F5;
	KeyCodeMap_[ AKEYCODE_F6 ] = OsEventInputKeyboard::KEYCODE_F6;
	KeyCodeMap_[ AKEYCODE_F7 ] = OsEventInputKeyboard::KEYCODE_F7;
	KeyCodeMap_[ AKEYCODE_F8 ] = OsEventInputKeyboard::KEYCODE_F8;
	KeyCodeMap_[ AKEYCODE_F9 ] = OsEventInputKeyboard::KEYCODE_F9;
	KeyCodeMap_[ AKEYCODE_F10 ] = OsEventInputKeyboard::KEYCODE_F10;
	KeyCodeMap_[ AKEYCODE_F11 ] = OsEventInputKeyboard::KEYCODE_F11;
	KeyCodeMap_[ AKEYCODE_F12 ] = OsEventInputKeyboard::KEYCODE_F12;

	PrevMouseX_ = 0;
	PrevMouseY_ = 0;
	MouseLocked_ = BcFalse;
	Width_ = 0;
	Height_ = 0;

	MousePrevDelta_ = MaVec2d( 0.0f, 0.0f );
	MouseDelta_ = MaVec2d( 0.0f, 0.0f );
	MousePos_ = MaVec2d( 0.0f, 0.0f );

	Width_ = 0;
	Height_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
OsClientAndroid::~OsClientAndroid()
{

}

//////////////////////////////////////////////////////////////////////////
// create
BcBool OsClientAndroid::create( const BcChar* pTitle )
{
	// Setup input.
	App_->userData = this;
	App_->onAppCmd = []( struct android_app* App, int32_t Cmd )->void
		{
			OsClientAndroid* Client = static_cast< OsClientAndroid* >( App->userData );
			switch( Cmd )
			{
			case APP_CMD_TERM_WINDOW:
				{
					SysKernel::pImpl()->flushAllJobQueues();
					Client->Window_ = nullptr;
				}
				break;

			case APP_CMD_INIT_WINDOW:
				{
					SysKernel::pImpl()->flushAllJobQueues();
					Client->Window_ = App->window;
				}
				break;
			}
		};

	App_->onInputEvent = []( struct android_app* App, AInputEvent* Event )->int32_t
		{
			OsClientAndroid* Client = static_cast< OsClientAndroid* >( App->userData ); 
			return Client->handleInput( Event ) ? 1 : 0;
		};


	// Wait until window init event is picked up.
	while( Window_ == nullptr )
	{
		pollLooper();

	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// update
void OsClientAndroid::update()
{
	pollLooper();
}

//////////////////////////////////////////////////////////////////////////
// destroy
void OsClientAndroid::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// getDeviceHandle
//virtual
BcHandle OsClientAndroid::getDeviceHandle()
{
	return static_cast< BcHandle >( 0 );
}

//////////////////////////////////////////////////////////////////////////
// getWindowHandle
//virtual
BcHandle OsClientAndroid::getWindowHandle()
{
	return static_cast< BcHandle >( Window_ );
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 OsClientAndroid::getWidth() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 OsClientAndroid::getHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// setMouseLock
void OsClientAndroid::setMouseLock( BcBool Enabled )
{
	MouseLocked_ = Enabled;
}

//////////////////////////////////////////////////////////////////////////
// handleEvent
BcU32 OsClientAndroid::getWindowId() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setSize
void OsClientAndroid::setSize( BcU32 Width, BcU32 Height )
{
	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// pollLooper
void OsClientAndroid::pollLooper()
{
	int EventId = 0;
	int NoofEvents = 0;
	struct android_poll_source* PollSource = nullptr;
	
	while ( ( EventId = ALooper_pollAll( 0, nullptr, &NoofEvents, (void**)&PollSource) ) >= 0 )
	{
		// Process this event.
		if( PollSource != nullptr )
		{
			PollSource->process( App_, PollSource );
		}
	}
}	

//////////////////////////////////////////////////////////////////////////
// handleInput
BcBool OsClientAndroid::handleInput( struct AInputEvent* Event )
{
	auto Source = AInputEvent_getSource( Event );
	auto Type = AInputEvent_getType( Event );

	BcAssert( Event );

	switch( Type )
	{
	case AINPUT_EVENT_TYPE_MOTION:
		{
			auto Action = AMotionEvent_getAction( Event );
			auto ActionType = Action & AMOTION_EVENT_ACTION_MASK;
			auto TouchID = ( Action & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK ) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

			// Regular touch event.
			OsEventInputTouch TouchEvent;
			TouchEvent.TouchID_ = TouchID;
			TouchEvent.TouchX_ = AMotionEvent_getX( Event, 0 );
			TouchEvent.TouchY_ = AMotionEvent_getY( Event, 0 );

			if( ActionType == AMOTION_EVENT_ACTION_DOWN )
			{
				OsCore::pImpl()->publish( osEVT_INPUT_TOUCHDOWN, TouchEvent ); // TODO: REMOVE OLD!
				EvtPublisher::publish( osEVT_INPUT_TOUCHDOWN, TouchEvent );
			}
			else if( ActionType == AMOTION_EVENT_ACTION_UP )
			{
				OsCore::pImpl()->publish( osEVT_INPUT_TOUCHUP, TouchEvent ); // TODO: REMOVE OLD!
				EvtPublisher::publish( osEVT_INPUT_TOUCHUP, TouchEvent );
			}
			else
			{
				OsCore::pImpl()->publish( osEVT_INPUT_TOUCHMOVE, TouchEvent ); // TODO: REMOVE OLD!
				EvtPublisher::publish( osEVT_INPUT_TOUCHMOVE, TouchEvent );
			}

			// Emulated mouse event.
			if( TouchID == 0 )
			{
				OsEventInputMouse MouseEvent;
				MouseEvent.MouseX_ = AMotionEvent_getX( Event, 0 );
				MouseEvent.MouseY_ = AMotionEvent_getY( Event, 0 );
				MouseEvent.MouseDX_ = (BcF32)(MouseEvent.MouseX_ - PrevMouseX_);
				MouseEvent.MouseDY_ = (BcF32)(MouseEvent.MouseY_ - PrevMouseY_);
				MouseEvent.NormalisedX_ = ( BcF32( MouseEvent.MouseX_ ) - BcF32( getWidth() ) * 0.5f ) / BcF32( getWidth() * 0.5f );
				MouseEvent.NormalisedY_ = ( BcF32( MouseEvent.MouseY_ ) - BcF32( getHeight() ) * 0.5f ) / BcF32( getHeight() * 0.5f );
				MouseEvent.ButtonCode_ = 0;
				PrevMouseX_ = MouseEvent.MouseX_;
				PrevMouseY_ = MouseEvent.MouseY_;

				if( ActionType == AMOTION_EVENT_ACTION_DOWN )
				{
					OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, MouseEvent ); // TODO: REMOVE OLD!
					EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, MouseEvent );
				}
				else if( ActionType == AMOTION_EVENT_ACTION_UP )
				{
					OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, MouseEvent ); // TODO: REMOVE OLD!
					EvtPublisher::publish( osEVT_INPUT_MOUSEUP, MouseEvent );
				}
				else
				{
					OsCore::pImpl()->publish( osEVT_INPUT_MOUSEMOVE, MouseEvent ); // TODO: REMOVE OLD!
					EvtPublisher::publish( osEVT_INPUT_MOUSEMOVE, MouseEvent );
				}
			}
			return BcTrue;
		}
		break;
	}
	return BcFalse;
}
