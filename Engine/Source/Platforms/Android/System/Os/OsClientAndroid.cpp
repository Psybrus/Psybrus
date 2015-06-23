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

#include "Base/BcString.h"
#include "Base/BcMath.h"

#include <android/input.h>


//////////////////////////////////////////////////////////////////////////
// Ctor
OsClientAndroid::OsClientAndroid()
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
}

//////////////////////////////////////////////////////////////////////////
// Dtor
OsClientAndroid::~OsClientAndroid()
{

}

//////////////////////////////////////////////////////////////////////////
// create
BcBool OsClientAndroid::create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen, BcBool Visible )
{
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// update
void OsClientAndroid::update()
{

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
	return static_cast< BcHandle >( 0 );
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
