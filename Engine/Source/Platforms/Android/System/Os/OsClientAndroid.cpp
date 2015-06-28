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
	App_( App )
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
	// Wait until window init event is picked up.
	while( App_->window == nullptr )
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
	return static_cast< BcHandle >( App_->window );
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

		// Check if we are exiting.
		if( App_->destroyRequested != 0 )
		{
			PSY_LOG( "Engine thread destroy requested!" );
			SysKernel::pImpl()->stop();
			return;
		}
	}
}

