/**************************************************************************
*
* File:		OsClientSDL.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#include "System/Os/SDL/OsClientSDL.h"

#include "System/Os/OsCore.h"

#include "Base/BcString.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Class name generation.
static BcU32 gClassID_ = 0;

//////////////////////////////////////////////////////////////////////////
// Ctor
OsClientSDL::OsClientSDL()
{
#if 0
	// Setup keycode map.
	KeyCodeMap_[ SDLK_LBUTTON ] = OsEventInputKeyboard::KEYCODE_LBUTTON;
	KeyCodeMap_[ SDLK_RBUTTON ] = OsEventInputKeyboard::KEYCODE_RBUTTON;
	KeyCodeMap_[ SDLK_CANCEL ] = OsEventInputKeyboard::KEYCODE_CANCEL;
	KeyCodeMap_[ SDLK_MBUTTON ] = OsEventInputKeyboard::KEYCODE_MBUTTON;
	KeyCodeMap_[ SDLK_BACK ] = OsEventInputKeyboard::KEYCODE_BACKSPACE;
	KeyCodeMap_[ SDLK_TAB ] = OsEventInputKeyboard::KEYCODE_TAB;
	KeyCodeMap_[ SDLK_CLEAR ] = OsEventInputKeyboard::KEYCODE_CLEAR;
	KeyCodeMap_[ SDLK_RETURN ] = OsEventInputKeyboard::KEYCODE_RETURN;
	KeyCodeMap_[ SDLK_SHIFT ] = OsEventInputKeyboard::KEYCODE_SHIFT;
	KeyCodeMap_[ SDLK_CONTROL ] = OsEventInputKeyboard::KEYCODE_CONTROL;
	KeyCodeMap_[ SDLK_MENU ] = OsEventInputKeyboard::KEYCODE_ALT;
	KeyCodeMap_[ SDLK_PAUSE ] = OsEventInputKeyboard::KEYCODE_PAUSE;
	KeyCodeMap_[ SDLK_CAPITAL ] = OsEventInputKeyboard::KEYCODE_CAPSLOCK;
	KeyCodeMap_[ SDLK_ESCAPE ] = OsEventInputKeyboard::KEYCODE_ESCAPE;
	KeyCodeMap_[ SDLK_SPACE ] = OsEventInputKeyboard::KEYCODE_SPACE;
	KeyCodeMap_[ SDLK_PRIOR ] = OsEventInputKeyboard::KEYCODE_PGUP;
	KeyCodeMap_[ SDLK_NEXT ] = OsEventInputKeyboard::KEYCODE_PGDN;
	KeyCodeMap_[ SDLK_END ] = OsEventInputKeyboard::KEYCODE_END;
	KeyCodeMap_[ SDLK_HOME ] = OsEventInputKeyboard::KEYCODE_HOME;
	KeyCodeMap_[ SDLK_LEFT ] = OsEventInputKeyboard::KEYCODE_LEFT;
	KeyCodeMap_[ SDLK_UP ] = OsEventInputKeyboard::KEYCODE_UP;
	KeyCodeMap_[ SDLK_RIGHT ] = OsEventInputKeyboard::KEYCODE_RIGHT;
	KeyCodeMap_[ SDLK_DOWN ] = OsEventInputKeyboard::KEYCODE_DOWN;
	KeyCodeMap_[ SDLK_SELECT ] = OsEventInputKeyboard::KEYCODE_SELECT;
	KeyCodeMap_[ SDLK_PRINT ] = OsEventInputKeyboard::KEYCODE_PRINT;
	KeyCodeMap_[ SDLK_EXECUTE ] = OsEventInputKeyboard::KEYCODE_EXECUTE;
	KeyCodeMap_[ SDLK_SNAPSHOT ] = OsEventInputKeyboard::KEYCODE_PRINT_SCREEN;
	KeyCodeMap_[ SDLK_INSERT ] = OsEventInputKeyboard::KEYCODE_INSERT;
	KeyCodeMap_[ SDLK_DELETE ] = OsEventInputKeyboard::KEYCODE_DELETE;
	KeyCodeMap_[ SDLK_HELP ] = OsEventInputKeyboard::KEYCODE_HELP;
	KeyCodeMap_[ SDLK_NUMPAD0 ] = OsEventInputKeyboard::KEYCODE_NUMPAD0;
	KeyCodeMap_[ SDLK_NUMPAD1 ] = OsEventInputKeyboard::KEYCODE_NUMPAD1;
	KeyCodeMap_[ SDLK_NUMPAD2 ] = OsEventInputKeyboard::KEYCODE_NUMPAD2;
	KeyCodeMap_[ SDLK_NUMPAD3 ] = OsEventInputKeyboard::KEYCODE_NUMPAD3;
	KeyCodeMap_[ SDLK_NUMPAD4 ] = OsEventInputKeyboard::KEYCODE_NUMPAD4;
	KeyCodeMap_[ SDLK_NUMPAD5 ] = OsEventInputKeyboard::KEYCODE_NUMPAD5;
	KeyCodeMap_[ SDLK_NUMPAD6 ] = OsEventInputKeyboard::KEYCODE_NUMPAD6;
	KeyCodeMap_[ SDLK_NUMPAD7 ] = OsEventInputKeyboard::KEYCODE_NUMPAD7;
	KeyCodeMap_[ SDLK_NUMPAD8 ] = OsEventInputKeyboard::KEYCODE_NUMPAD8;
	KeyCodeMap_[ SDLK_NUMPAD9 ] = OsEventInputKeyboard::KEYCODE_NUMPAD9;
	KeyCodeMap_[ SDLK_SEPARATOR ] = OsEventInputKeyboard::KEYCODE_SEPARATOR;
	KeyCodeMap_[ SDLK_SUBTRACT ] = OsEventInputKeyboard::KEYCODE_SUBTRACT;
	KeyCodeMap_[ SDLK_DECIMAL ] = OsEventInputKeyboard::KEYCODE_DECIMAL;
	KeyCodeMap_[ SDLK_DIVIDE ] = OsEventInputKeyboard::KEYCODE_DIVIDE;
	KeyCodeMap_[ SDLK_F1 ] = OsEventInputKeyboard::KEYCODE_F1;
	KeyCodeMap_[ SDLK_F2 ] = OsEventInputKeyboard::KEYCODE_F2;
	KeyCodeMap_[ SDLK_F3 ] = OsEventInputKeyboard::KEYCODE_F3;
	KeyCodeMap_[ SDLK_F4 ] = OsEventInputKeyboard::KEYCODE_F4;
	KeyCodeMap_[ SDLK_F5 ] = OsEventInputKeyboard::KEYCODE_F5;
	KeyCodeMap_[ SDLK_F6 ] = OsEventInputKeyboard::KEYCODE_F6;
	KeyCodeMap_[ SDLK_F7 ] = OsEventInputKeyboard::KEYCODE_F7;
	KeyCodeMap_[ SDLK_F8 ] = OsEventInputKeyboard::KEYCODE_F8;
	KeyCodeMap_[ SDLK_F9 ] = OsEventInputKeyboard::KEYCODE_F9;
	KeyCodeMap_[ SDLK_F10 ] = OsEventInputKeyboard::KEYCODE_F10;
	KeyCodeMap_[ SDLK_F11 ] = OsEventInputKeyboard::KEYCODE_F11;
	KeyCodeMap_[ SDLK_F12 ] = OsEventInputKeyboard::KEYCODE_F12;
	KeyCodeMap_[ SDLK_F13 ] = OsEventInputKeyboard::KEYCODE_F13;
	KeyCodeMap_[ SDLK_F14 ] = OsEventInputKeyboard::KEYCODE_F14;
	KeyCodeMap_[ SDLK_F15 ] = OsEventInputKeyboard::KEYCODE_F15;
	KeyCodeMap_[ SDLK_F16 ] = OsEventInputKeyboard::KEYCODE_F16;
	KeyCodeMap_[ SDLK_F17 ] = OsEventInputKeyboard::KEYCODE_F17;
	KeyCodeMap_[ SDLK_F18 ] = OsEventInputKeyboard::KEYCODE_F18;
	KeyCodeMap_[ SDLK_F19 ] = OsEventInputKeyboard::KEYCODE_F19;
	KeyCodeMap_[ SDLK_F20 ] = OsEventInputKeyboard::KEYCODE_F20;
	KeyCodeMap_[ SDLK_F21 ] = OsEventInputKeyboard::KEYCODE_F21;
	KeyCodeMap_[ SDLK_F22 ] = OsEventInputKeyboard::KEYCODE_F22;
	KeyCodeMap_[ SDLK_F23 ] = OsEventInputKeyboard::KEYCODE_F23;
	KeyCodeMap_[ SDLK_F24 ] = OsEventInputKeyboard::KEYCODE_F24;
	KeyCodeMap_[ SDLK_NUMLOCK ] = OsEventInputKeyboard::KEYCODE_NUMLOCK;
	KeyCodeMap_[ SDLK_SCROLL ] = OsEventInputKeyboard::KEYCODE_SCROLL;
	KeyCodeMap_[ SDLK_LSHIFT ] = OsEventInputKeyboard::KEYCODE_LSHIFT;
	KeyCodeMap_[ SDLK_RSHIFT ] = OsEventInputKeyboard::KEYCODE_RSHIFT;
	KeyCodeMap_[ SDLK_LCONTROL ] = OsEventInputKeyboard::KEYCODE_LCONTROL;
	KeyCodeMap_[ SDLK_RCONTROL ] = OsEventInputKeyboard::KEYCODE_RCONTROL;
	KeyCodeMap_[ SDLK_LMENU ] = OsEventInputKeyboard::KEYCODE_LMENU;
	KeyCodeMap_[ SDLK_RMENU ] = OsEventInputKeyboard::KEYCODE_RMENU;
	KeyCodeMap_[ SDLK_PLAY ] = OsEventInputKeyboard::KEYCODE_PLAY;
	KeyCodeMap_[ SDLK_ZOOM ] = OsEventInputKeyboard::KEYCODE_ZOOM;
#endif
	PrevMouseX_ = 0;
	PrevMouseY_ = 0;
	MouseLocked_ = BcFalse;

	MousePrevDelta_ = MaVec2d( 0.0f, 0.0f );
	MouseDelta_ = MaVec2d( 0.0f, 0.0f );
	MousePos_ = MaVec2d( 0.0f, 0.0f );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
OsClientSDL::~OsClientSDL()
{

}

//////////////////////////////////////////////////////////////////////////
// create
BcBool OsClientSDL::create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen, BcBool Visible )
{
	SDLWindow_ = SDL_CreateWindow( pTitle, 100, 100, Width, Height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if ( SDLWindow_ == nullptr )
	{
		return BcFalse;
	}
	
	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// update
void OsClientSDL::update()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
void OsClientSDL::destroy()
{	
	SDL_DestroyWindow( SDLWindow_ );
	SDLWindow_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getDeviceHandle
//virtual
BcHandle OsClientSDL::getDeviceHandle()
{
	return (BcHandle)SDLWindow_;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 OsClientSDL::getWidth() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 OsClientSDL::getHeight() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setMouseLock
void OsClientSDL::setMouseLock( BcBool Enabled )
{
	MouseLocked_ = Enabled;
}
