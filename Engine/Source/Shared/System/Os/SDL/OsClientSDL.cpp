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
	// Setup keycode map.
	KeyCodeMap_[ SDLK_CANCEL ] = OsEventInputKeyboard::KEYCODE_CANCEL;
	KeyCodeMap_[ SDLK_TAB ] = OsEventInputKeyboard::KEYCODE_TAB;
	KeyCodeMap_[ SDLK_CLEAR ] = OsEventInputKeyboard::KEYCODE_CLEAR;
	KeyCodeMap_[ SDLK_RETURN ] = OsEventInputKeyboard::KEYCODE_RETURN;
	KeyCodeMap_[ SDLK_BACKSPACE ] = OsEventInputKeyboard::KEYCODE_BACKSPACE;	
	KeyCodeMap_[ SDLK_LCTRL ] = OsEventInputKeyboard::KEYCODE_CONTROL;	
	KeyCodeMap_[ SDLK_LSHIFT ] = OsEventInputKeyboard::KEYCODE_SHIFT;	
	KeyCodeMap_[ SDLK_RCTRL ] = OsEventInputKeyboard::KEYCODE_CONTROL;	
	KeyCodeMap_[ SDLK_RSHIFT ] = OsEventInputKeyboard::KEYCODE_SHIFT;	
	KeyCodeMap_[ SDLK_MENU ] = OsEventInputKeyboard::KEYCODE_ALT;
	KeyCodeMap_[ SDLK_PAUSE ] = OsEventInputKeyboard::KEYCODE_PAUSE;
	KeyCodeMap_[ SDLK_ESCAPE ] = OsEventInputKeyboard::KEYCODE_ESCAPE;
	KeyCodeMap_[ SDLK_SPACE ] = OsEventInputKeyboard::KEYCODE_SPACE;
	KeyCodeMap_[ SDLK_PRIOR ] = OsEventInputKeyboard::KEYCODE_PGUP;
	KeyCodeMap_[ SDLK_END ] = OsEventInputKeyboard::KEYCODE_END;
	KeyCodeMap_[ SDLK_HOME ] = OsEventInputKeyboard::KEYCODE_HOME;
	KeyCodeMap_[ SDLK_LEFT ] = OsEventInputKeyboard::KEYCODE_LEFT;
	KeyCodeMap_[ SDLK_UP ] = OsEventInputKeyboard::KEYCODE_UP;
	KeyCodeMap_[ SDLK_RIGHT ] = OsEventInputKeyboard::KEYCODE_RIGHT;
	KeyCodeMap_[ SDLK_DOWN ] = OsEventInputKeyboard::KEYCODE_DOWN;
	KeyCodeMap_[ SDLK_SELECT ] = OsEventInputKeyboard::KEYCODE_SELECT;
	KeyCodeMap_[ SDLK_EXECUTE ] = OsEventInputKeyboard::KEYCODE_EXECUTE;
	KeyCodeMap_[ SDLK_INSERT ] = OsEventInputKeyboard::KEYCODE_INSERT;
	KeyCodeMap_[ SDLK_DELETE ] = OsEventInputKeyboard::KEYCODE_DELETE;
	KeyCodeMap_[ SDLK_HELP ] = OsEventInputKeyboard::KEYCODE_HELP;
	KeyCodeMap_[ SDLK_KP_0 ] = OsEventInputKeyboard::KEYCODE_NUMPAD0;
	KeyCodeMap_[ SDLK_KP_1 ] = OsEventInputKeyboard::KEYCODE_NUMPAD1;
	KeyCodeMap_[ SDLK_KP_2 ] = OsEventInputKeyboard::KEYCODE_NUMPAD2;
	KeyCodeMap_[ SDLK_KP_3 ] = OsEventInputKeyboard::KEYCODE_NUMPAD3;
	KeyCodeMap_[ SDLK_KP_4 ] = OsEventInputKeyboard::KEYCODE_NUMPAD4;
	KeyCodeMap_[ SDLK_KP_5 ] = OsEventInputKeyboard::KEYCODE_NUMPAD5;
	KeyCodeMap_[ SDLK_KP_6 ] = OsEventInputKeyboard::KEYCODE_NUMPAD6;
	KeyCodeMap_[ SDLK_KP_7 ] = OsEventInputKeyboard::KEYCODE_NUMPAD7;
	KeyCodeMap_[ SDLK_KP_8 ] = OsEventInputKeyboard::KEYCODE_NUMPAD8;
	KeyCodeMap_[ SDLK_KP_9 ] = OsEventInputKeyboard::KEYCODE_NUMPAD9;
	KeyCodeMap_[ SDLK_SEPARATOR ] = OsEventInputKeyboard::KEYCODE_SEPARATOR;
	KeyCodeMap_[ SDLK_KP_MINUS ] = OsEventInputKeyboard::KEYCODE_SUBTRACT;
	KeyCodeMap_[ SDLK_KP_PERIOD ] = OsEventInputKeyboard::KEYCODE_DECIMAL;
	KeyCodeMap_[ SDLK_KP_DIVIDE ] = OsEventInputKeyboard::KEYCODE_DIVIDE;
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
OsClientSDL::~OsClientSDL()
{

}

//////////////////////////////////////////////////////////////////////////
// create
BcBool OsClientSDL::create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen, BcBool Visible )
{
	SDLWindow_ = SDL_CreateWindow( 
		pTitle,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		Width, Height, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN );
	if ( SDLWindow_ == nullptr )
	{
		return BcFalse;
	}

	// Get window size.
	int W = 0;
	int H = 0;
	SDL_GetWindowSize( SDLWindow_, &W, &H );
	Width_ = W;
	Height_ = H;
	
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
// getWindowHandle
//virtual
BcHandle OsClientSDL::getWindowHandle()
{
	return (BcHandle)SDLWindow_;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 OsClientSDL::getWidth() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 OsClientSDL::getHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// setMouseLock
void OsClientSDL::setMouseLock( BcBool Enabled )
{
	MouseLocked_ = Enabled;
}

//////////////////////////////////////////////////////////////////////////
// handleEvent
BcU32 OsClientSDL::getWindowId() const
{
	return SDL_GetWindowID( SDLWindow_ );
}

//////////////////////////////////////////////////////////////////////////
// handleEvent
void OsClientSDL::handleEvent( const SDL_Event& SDLEvent )
{
	switch( SDLEvent.type )
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		handleKeyEvent( SDLEvent );
		break;

	case SDL_TEXTINPUT:
		handleTextInputEvent( SDLEvent );
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
	case SDL_MOUSEMOTION:
		handleMouseEvent( SDLEvent );
		break;
	
	case SDL_WINDOWEVENT:
		handleWindowEvent( SDLEvent );
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// handleKeyEvent
void OsClientSDL::handleKeyEvent( const SDL_Event& SDLEvent )
{
	OsEventInputKeyboard Event;
	Event.DeviceID_ = 0;
	Event.KeyCode_ = SDLEvent.key.keysym.sym;
	Event.ScanCode_ = SDLEvent.key.keysym.scancode;
	Event.AsciiCode_ = SDLEvent.key.keysym.sym; // TODO.

	// Get key code, or pass through virtual.
	TKeyCodeMapIterator It( KeyCodeMap_.find( Event.KeyCode_ ) );
	if( It != KeyCodeMap_.end() )
	{
		Event.KeyCode_ = (*It).second;
	}

	if( SDLEvent.key.state == SDL_PRESSED )
	{
		OsCore::pImpl()->publish( osEVT_INPUT_KEYDOWN, Event ); // TODO: REMOVE OLD!
		EvtPublisher::publish( osEVT_INPUT_KEYDOWN, Event );
	}
	else if( SDLEvent.key.state == SDL_RELEASED )
	{
		OsCore::pImpl()->publish( osEVT_INPUT_KEYUP, Event ); // TODO: REMOVE OLD!
		EvtPublisher::publish( osEVT_INPUT_KEYUP, Event );
	}
}

//////////////////////////////////////////////////////////////////////////
// handleTextInputEvent
void OsClientSDL::handleTextInputEvent( const SDL_Event& SDLEvent )
{
	OsEventInputText Event;
	Event.DeviceID_ = 0;
	memcpy( Event.Text_, SDLEvent.text.text, sizeof( SDLEvent.text.text ) );

	OsCore::pImpl()->publish( osEVT_INPUT_TEXT, Event ); // TODO: REMOVE OLD!
	EvtPublisher::publish( osEVT_INPUT_TEXT, Event );
}

//////////////////////////////////////////////////////////////////////////
// handleMouseEvent
void OsClientSDL::handleMouseEvent( const SDL_Event& SDLEvent )
{
	OsEventInputMouse Event;
	Event.DeviceID_ = 0;
	
	switch( SDLEvent.type )
	{
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		{
			Event.MouseX_ = SDLEvent.button.x;
			Event.MouseY_ = SDLEvent.button.y;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = ( BcF32( Event.MouseX_ ) - BcF32( getWidth() ) * 0.5f ) / BcF32( getWidth() * 0.5f );
			Event.NormalisedY_ = ( BcF32( Event.MouseY_ ) - BcF32( getHeight() ) * 0.5f ) / BcF32( getHeight() * 0.5f );

			switch( SDLEvent.button.button )
			{
			case SDL_BUTTON_LEFT:
				Event.ButtonCode_ = 0;
				break;
			case SDL_BUTTON_RIGHT:
				Event.ButtonCode_ = 1;
				break;
			case SDL_BUTTON_MIDDLE:
				Event.ButtonCode_ = 2;
				break;
			case SDL_BUTTON_X1:
				Event.ButtonCode_ = 3;
				break;
			case SDL_BUTTON_X2:
				Event.ButtonCode_ = 4;
				break;
			}

			PrevMouseX_ = Event.MouseX_;
			PrevMouseY_ = Event.MouseY_;
			if( SDLEvent.type == SDL_MOUSEBUTTONDOWN )
			{
				OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, Event ); // TODO: REMOVE OLD!
				EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, Event );
			}
			else if( SDLEvent.type == SDL_MOUSEBUTTONUP )
			{
				OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, Event ); // TODO: REMOVE OLD!
				EvtPublisher::publish( osEVT_INPUT_MOUSEUP, Event );
			}
		}
		break;

	case SDL_MOUSEMOTION:
		{
			Event.MouseX_ = SDLEvent.motion.x;
			Event.MouseY_ = SDLEvent.motion.y;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = ( BcF32( Event.MouseX_ ) - BcF32( getWidth() ) * 0.5f ) / BcF32( getWidth() * 0.5f );
			Event.NormalisedY_ = ( BcF32( Event.MouseY_ ) - BcF32( getHeight() ) * 0.5f ) / BcF32( getHeight() * 0.5f );
			Event.ButtonCode_ = 0;

			PrevMouseX_ = Event.MouseX_;
			PrevMouseY_ = Event.MouseY_;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEMOVE, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEMOVE, Event );
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// handleWindowEvent
void OsClientSDL::handleWindowEvent( const SDL_Event& SDLEvent )
{
	switch( SDLEvent.window.event )
	{
	case SDL_WINDOWEVENT_SHOWN:
	case SDL_WINDOWEVENT_RESIZED:
	case SDL_WINDOWEVENT_MINIMIZED:
	case SDL_WINDOWEVENT_MAXIMIZED:
		{
			setWindowSize();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setWindowSize
void OsClientSDL::setWindowSize()
{
	int W = 0;
	int H = 0;
	SDL_GetWindowSize( SDLWindow_, &W, &H );
	Width_ = W;
	Height_ = H;

	// Send event.
	OsEventClientResize Event;
	Event.pClient_ = this;
	Event.Width_ = Width_;
	Event.Height_ = Height_;
	OsCore::pImpl()->publish( osEVT_CLIENT_RESIZE, Event ); // TODO: REMOVE OLD!
	EvtPublisher::publish( osEVT_CLIENT_RESIZE, Event );
}
