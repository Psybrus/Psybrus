/**************************************************************************
*
* File:		OsClientWindows.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#include "System/Os/OsClientWindows.h"

#include "System/Os/OsCore.h"

#include "Base/BcString.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Class name generation.
static BcU32 gClassID_ = 0;

//////////////////////////////////////////////////////////////////////////
// Ctor
OsClientWindows::OsClientWindows()
{
	// Setup keycode map.
	KeyCodeMap_[ VK_LBUTTON ] = OsEventInputKeyboard::KEYCODE_LBUTTON;
	KeyCodeMap_[ VK_RBUTTON ] = OsEventInputKeyboard::KEYCODE_RBUTTON;
	KeyCodeMap_[ VK_CANCEL ] = OsEventInputKeyboard::KEYCODE_CANCEL;
	KeyCodeMap_[ VK_MBUTTON ] = OsEventInputKeyboard::KEYCODE_MBUTTON;
	KeyCodeMap_[ VK_BACK ] = OsEventInputKeyboard::KEYCODE_BACKSPACE;
	KeyCodeMap_[ VK_TAB ] = OsEventInputKeyboard::KEYCODE_TAB;
	KeyCodeMap_[ VK_CLEAR ] = OsEventInputKeyboard::KEYCODE_CLEAR;
	KeyCodeMap_[ VK_RETURN ] = OsEventInputKeyboard::KEYCODE_RETURN;
	KeyCodeMap_[ VK_SHIFT ] = OsEventInputKeyboard::KEYCODE_SHIFT;
	KeyCodeMap_[ VK_CONTROL ] = OsEventInputKeyboard::KEYCODE_CONTROL;
	KeyCodeMap_[ VK_MENU ] = OsEventInputKeyboard::KEYCODE_ALT;
	KeyCodeMap_[ VK_PAUSE ] = OsEventInputKeyboard::KEYCODE_PAUSE;
	KeyCodeMap_[ VK_CAPITAL ] = OsEventInputKeyboard::KEYCODE_CAPSLOCK;
	KeyCodeMap_[ VK_ESCAPE ] = OsEventInputKeyboard::KEYCODE_ESCAPE;
	KeyCodeMap_[ VK_SPACE ] = OsEventInputKeyboard::KEYCODE_SPACE;
	KeyCodeMap_[ VK_PRIOR ] = OsEventInputKeyboard::KEYCODE_PGUP;
	KeyCodeMap_[ VK_NEXT ] = OsEventInputKeyboard::KEYCODE_PGDN;
	KeyCodeMap_[ VK_END ] = OsEventInputKeyboard::KEYCODE_END;
	KeyCodeMap_[ VK_HOME ] = OsEventInputKeyboard::KEYCODE_HOME;
	KeyCodeMap_[ VK_LEFT ] = OsEventInputKeyboard::KEYCODE_LEFT;
	KeyCodeMap_[ VK_UP ] = OsEventInputKeyboard::KEYCODE_UP;
	KeyCodeMap_[ VK_RIGHT ] = OsEventInputKeyboard::KEYCODE_RIGHT;
	KeyCodeMap_[ VK_DOWN ] = OsEventInputKeyboard::KEYCODE_DOWN;
	KeyCodeMap_[ VK_SELECT ] = OsEventInputKeyboard::KEYCODE_SELECT;
	KeyCodeMap_[ VK_PRINT ] = OsEventInputKeyboard::KEYCODE_PRINT;
	KeyCodeMap_[ VK_EXECUTE ] = OsEventInputKeyboard::KEYCODE_EXECUTE;
	KeyCodeMap_[ VK_SNAPSHOT ] = OsEventInputKeyboard::KEYCODE_PRINT_SCREEN;
	KeyCodeMap_[ VK_INSERT ] = OsEventInputKeyboard::KEYCODE_INSERT;
	KeyCodeMap_[ VK_DELETE ] = OsEventInputKeyboard::KEYCODE_DELETE;
	KeyCodeMap_[ VK_HELP ] = OsEventInputKeyboard::KEYCODE_HELP;
	KeyCodeMap_[ VK_NUMPAD0 ] = OsEventInputKeyboard::KEYCODE_NUMPAD0;
	KeyCodeMap_[ VK_NUMPAD1 ] = OsEventInputKeyboard::KEYCODE_NUMPAD1;
	KeyCodeMap_[ VK_NUMPAD2 ] = OsEventInputKeyboard::KEYCODE_NUMPAD2;
	KeyCodeMap_[ VK_NUMPAD3 ] = OsEventInputKeyboard::KEYCODE_NUMPAD3;
	KeyCodeMap_[ VK_NUMPAD4 ] = OsEventInputKeyboard::KEYCODE_NUMPAD4;
	KeyCodeMap_[ VK_NUMPAD5 ] = OsEventInputKeyboard::KEYCODE_NUMPAD5;
	KeyCodeMap_[ VK_NUMPAD6 ] = OsEventInputKeyboard::KEYCODE_NUMPAD6;
	KeyCodeMap_[ VK_NUMPAD7 ] = OsEventInputKeyboard::KEYCODE_NUMPAD7;
	KeyCodeMap_[ VK_NUMPAD8 ] = OsEventInputKeyboard::KEYCODE_NUMPAD8;
	KeyCodeMap_[ VK_NUMPAD9 ] = OsEventInputKeyboard::KEYCODE_NUMPAD9;
	KeyCodeMap_[ VK_SEPARATOR ] = OsEventInputKeyboard::KEYCODE_SEPARATOR;
	KeyCodeMap_[ VK_SUBTRACT ] = OsEventInputKeyboard::KEYCODE_SUBTRACT;
	KeyCodeMap_[ VK_DECIMAL ] = OsEventInputKeyboard::KEYCODE_DECIMAL;
	KeyCodeMap_[ VK_DIVIDE ] = OsEventInputKeyboard::KEYCODE_DIVIDE;
	KeyCodeMap_[ VK_F1 ] = OsEventInputKeyboard::KEYCODE_F1;
	KeyCodeMap_[ VK_F2 ] = OsEventInputKeyboard::KEYCODE_F2;
	KeyCodeMap_[ VK_F3 ] = OsEventInputKeyboard::KEYCODE_F3;
	KeyCodeMap_[ VK_F4 ] = OsEventInputKeyboard::KEYCODE_F4;
	KeyCodeMap_[ VK_F5 ] = OsEventInputKeyboard::KEYCODE_F5;
	KeyCodeMap_[ VK_F6 ] = OsEventInputKeyboard::KEYCODE_F6;
	KeyCodeMap_[ VK_F7 ] = OsEventInputKeyboard::KEYCODE_F7;
	KeyCodeMap_[ VK_F8 ] = OsEventInputKeyboard::KEYCODE_F8;
	KeyCodeMap_[ VK_F9 ] = OsEventInputKeyboard::KEYCODE_F9;
	KeyCodeMap_[ VK_F10 ] = OsEventInputKeyboard::KEYCODE_F10;
	KeyCodeMap_[ VK_F11 ] = OsEventInputKeyboard::KEYCODE_F11;
	KeyCodeMap_[ VK_F12 ] = OsEventInputKeyboard::KEYCODE_F12;
	KeyCodeMap_[ VK_F13 ] = OsEventInputKeyboard::KEYCODE_F13;
	KeyCodeMap_[ VK_F14 ] = OsEventInputKeyboard::KEYCODE_F14;
	KeyCodeMap_[ VK_F15 ] = OsEventInputKeyboard::KEYCODE_F15;
	KeyCodeMap_[ VK_F16 ] = OsEventInputKeyboard::KEYCODE_F16;
	KeyCodeMap_[ VK_F17 ] = OsEventInputKeyboard::KEYCODE_F17;
	KeyCodeMap_[ VK_F18 ] = OsEventInputKeyboard::KEYCODE_F18;
	KeyCodeMap_[ VK_F19 ] = OsEventInputKeyboard::KEYCODE_F19;
	KeyCodeMap_[ VK_F20 ] = OsEventInputKeyboard::KEYCODE_F20;
	KeyCodeMap_[ VK_F21 ] = OsEventInputKeyboard::KEYCODE_F21;
	KeyCodeMap_[ VK_F22 ] = OsEventInputKeyboard::KEYCODE_F22;
	KeyCodeMap_[ VK_F23 ] = OsEventInputKeyboard::KEYCODE_F23;
	KeyCodeMap_[ VK_F24 ] = OsEventInputKeyboard::KEYCODE_F24;
	KeyCodeMap_[ VK_NUMLOCK ] = OsEventInputKeyboard::KEYCODE_NUMLOCK;
	KeyCodeMap_[ VK_SCROLL ] = OsEventInputKeyboard::KEYCODE_SCROLL;
	KeyCodeMap_[ VK_LSHIFT ] = OsEventInputKeyboard::KEYCODE_LSHIFT;
	KeyCodeMap_[ VK_RSHIFT ] = OsEventInputKeyboard::KEYCODE_RSHIFT;
	KeyCodeMap_[ VK_LCONTROL ] = OsEventInputKeyboard::KEYCODE_LCONTROL;
	KeyCodeMap_[ VK_RCONTROL ] = OsEventInputKeyboard::KEYCODE_RCONTROL;
	KeyCodeMap_[ VK_LMENU ] = OsEventInputKeyboard::KEYCODE_LMENU;
	KeyCodeMap_[ VK_RMENU ] = OsEventInputKeyboard::KEYCODE_RMENU;
	KeyCodeMap_[ VK_PLAY ] = OsEventInputKeyboard::KEYCODE_PLAY;
	KeyCodeMap_[ VK_ZOOM ] = OsEventInputKeyboard::KEYCODE_ZOOM;

	PrevMouseX_ = 0;
	PrevMouseY_ = 0;
	MouseLocked_ = BcFalse;

	MousePrevDelta_ = MaVec2d( 0.0f, 0.0f );
	MouseDelta_ = MaVec2d( 0.0f, 0.0f );
	MousePos_ = MaVec2d( 0.0f, 0.0f );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
OsClientWindows::~OsClientWindows()
{

}

//////////////////////////////////////////////////////////////////////////
// create
BcBool OsClientWindows::create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen, BcBool Visible )
{
	WNDCLASSEX	wc;
	RECT		WindowRect;

	WindowRect.left = (long)0;
	WindowRect.right = (long)Width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)Height;

	hInstance_ = Instance;


	// Enumerate device guff.
	BOOL EnumSuccess = FALSE;
	BcU32 ModeIdx = 0;
	do
	{
		DEVMODEA DevMode;
		ZeroMemory( &DevMode, sizeof( DevMode ) );
		EnumSuccess = ::EnumDisplaySettingsA( 
			nullptr, 
			ModeIdx++,
			&DevMode );

		if( EnumSuccess )
		{
			DeviceModes_.push_back( DevMode );
		}
	}
	while( EnumSuccess );

	BcU32 DeviceIdx = 0;
	do
	{
		DISPLAY_DEVICEA DisplayDevice;
		ZeroMemory( &DisplayDevice, sizeof( DisplayDevice ) );
		EnumSuccess = ::EnumDisplayDevicesA( 
			nullptr, 
			DeviceIdx++,
			&DisplayDevice,
			EDD_GET_DEVICE_INTERFACE_NAME );

		if( EnumSuccess )
		{
			DisplayDevices_.push_back( DisplayDevice );
		}
	}
	while( EnumSuccess );

	BcU32 MonitorIdx = 0;
	do
	{
		MONITORINFO MonitorInfo;
		ZeroMemory( &MonitorInfo, sizeof( MonitorInfo ) );
		EnumSuccess = ::GetMonitorInfoA(
			(HMONITOR)MonitorIdx,
			&MonitorInfo );

		if( EnumSuccess )
		{
			MonitorInfos_.push_back( MonitorInfo );
		}
	}
	while( EnumSuccess );

	// Generate class name.
	BcSPrintf( ClassName_, "PsybrusWindow_0x%x", gClassID_++ );
	
	wc.cbSize			= sizeof( WNDCLASSEX );
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= (WNDPROC) WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= (HINSTANCE)Instance;
	wc.hIcon			= ::LoadIcon( NULL, IDI_WINLOGO );
	wc.hCursor			= ::LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground	= (HBRUSH)( COLOR_WINDOW );
	wc.lpszMenuName		= NULL;
	wc.lpszClassName	= ClassName_;
	wc.hIconSm			= NULL;

	if ( !RegisterClassEx( &wc ) )
	{
		return BcFalse;
	}

	if( Fullscreen == BcFalse )
	{
		WindowStyleEx_ = WS_EX_WINDOWEDGE;
		WindowStyle_ = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;
	}
	else
	{
		WindowStyleEx_ = WS_EX_APPWINDOW;
		WindowStyle_ = WS_POPUP;	
	}

	::AdjustWindowRectEx( &WindowRect, WindowStyle_, FALSE, WindowStyleEx_);

	centreWindow( Width, Height );

	// Create The Window
	if ( !( hWnd_=::CreateWindowEx(	WindowStyleEx_,
		ClassName_,
		pTitle,
		WindowStyle_,
		WindowSize_.left, WindowSize_.top,
		WindowRect.right-WindowRect.left,
		WindowRect.bottom-WindowRect.top,
		NULL,
		NULL,
		(HINSTANCE)Instance,
		NULL)))	
	{
		return BcFalse;
	}

	// Set user data.
	::SetWindowLongPtr( hWnd_, GWLP_USERDATA, (LONG_PTR)this );

	// Get the device context
	hDC_ = GetDC( hWnd_ );

	// Show the window
	if( Visible )
	{
		::ShowWindow( hWnd_, SW_SHOWMAXIMIZED );
		::SetForegroundWindow( hWnd_ );
		::SetFocus( hWnd_ );
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// update
void OsClientWindows::update()
{
	// Update mouse if we're in focus.
	if( ::GetActiveWindow() == hWnd_ )
	{
		POINT MousePosition;
		POINT WindowPosition;
		RECT Rect;

		// Get window rect in screen space.
		::GetWindowRect( hWnd_, &Rect );
		
		// Screen space cood of the client area.
		WindowPosition.x = 0;
		WindowPosition.y = 0;
		::ClientToScreen( hWnd_, &WindowPosition );
		
		// Get the cursor position
		::GetCursorPos( &MousePosition );

		const BcS32 WX = ( Rect.right - Rect.left );
		const BcS32 WY = ( Rect.bottom - Rect.top );
		MouseDelta_.x( BcF32( MousePosition.x - ( Rect.left + ( WX / 2 ) ) ) );
		MouseDelta_.y( BcF32( MousePosition.y - ( Rect.top + ( WY / 2 ) ) ) );

		MousePos_ += MouseDelta_;
		MousePos_.x( BcClamp( MousePos_.x(), 0.0f, BcF32( WX ) ) );
		MousePos_.y( BcClamp( MousePos_.y(), 0.0f, BcF32( WY ) ) );

		// Smooth out delta
		const MaVec2d TempOld = MouseDelta_;
		MouseDelta_ = ( MousePrevDelta_ + MouseDelta_ ) * 0.5f;
		MousePrevDelta_ = TempOld;

		// Lock to centre of screen if we're in focus.
		if( MouseLocked_ )
		{
			::SetCursorPos( Rect.left + ( WX / 2 ), Rect.top + ( WY / 2 ) );
		}

		// Send event if moved.
		if( MouseDelta_.magnitude() > 0.5f )
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = (BcS16)MousePosition.x - (BcS16)WindowPosition.x;
			Event.MouseY_ = (BcS16)MousePosition.y - (BcS16)WindowPosition.y;
			Event.MouseDX_ = MouseDelta_.x();
			Event.MouseDY_ = MouseDelta_.y();
			Event.NormalisedX_ = BcF32( (BcS32)Event.MouseX_ - ( (BcS32)getWidth() / 2 ) ) / BcF32( (BcS32)getWidth() / 2 );
			Event.NormalisedY_ = BcF32( (BcS32)Event.MouseY_ - ( (BcS32)getHeight() / 2 ) ) / BcF32( (BcS32)getHeight() / 2 );

			// Legacy...
			PrevMouseX_ = Event.MouseX_;
			PrevMouseY_ = Event.MouseY_;

			Event.ButtonCode_ = 0;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEMOVE, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEMOVE, Event );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// destroy
void OsClientWindows::destroy()
{	
	// Destroy.
	if ( hDC_ && !::ReleaseDC( hWnd_, hDC_ ) )
	{
		hDC_ = NULL;
	}

	if ( hWnd_ && !::DestroyWindow( hWnd_ ) )
	{
		hWnd_ = NULL;
	}

	::UnregisterClass( ClassName_, (HINSTANCE)hInstance_ );

	hDC_ = NULL;
	hWnd_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// getDeviceHandle
//virtual
BcHandle OsClientWindows::getDeviceHandle()
{
	return (BcHandle)hDC_;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 OsClientWindows::getWidth() const
{
	return ClientSize_.right - ClientSize_.left;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 OsClientWindows::getHeight() const
{
	return ClientSize_.bottom - ClientSize_.top;
}

//////////////////////////////////////////////////////////////////////////
// centreWindow
BcBool OsClientWindows::centreWindow( BcS32 SizeX, BcS32 SizeY )
{
	RECT Rect;

	::SystemParametersInfo( SPI_GETWORKAREA, 0, &Rect, 0 );

	BcS32 Width = Rect.right - Rect.left;
	BcS32 Height = Rect.bottom - Rect.top;
	BcS32 slX = ( ( Width - SizeX ) / 2 ) + Rect.left;
	BcS32 slY = ( ( Height - SizeY ) / 2 ) + Rect.top;

	WindowSize_.left = slX;
	WindowSize_.top = slY;
	WindowSize_.right = ( SizeX + slX );
	WindowSize_.bottom = ( SizeY + slY ); 
	BcBool RetValue = BcTrue;

	if( ::AdjustWindowRectEx( &WindowSize_, WindowStyle_, BcFalse, WindowStyleEx_ ) == FALSE )
	{
		RetValue  = BcFalse;
	}
	
	return RetValue;
}

//////////////////////////////////////////////////////////////////////////
// setMouseLock
void OsClientWindows::setMouseLock( BcBool Enabled )
{
	// Hide cursor too.
	CURSORINFO CursorInfo;
	CursorInfo.cbSize = sizeof(CursorInfo);
	GetCursorInfo( &CursorInfo );

	if( Enabled )
	{
		if( MouseLocked_ == BcFalse )
		{
			POINT MousePosition;
			POINT WindowPosition;
			RECT Rect;

			// Get window rect in screen space.
			::GetWindowRect( hWnd_, &Rect );
		
			// Screen space cood of the client area.
			WindowPosition.x = 0;
			WindowPosition.y = 0;
			::ClientToScreen( hWnd_, &WindowPosition );
		
			// Get the cursor position
			::GetCursorPos( &MousePosition );

			const BcS32 WX = ( Rect.right - Rect.left );
			const BcS32 WY = ( Rect.bottom - Rect.top );

			// Reset delta.
			MouseDelta_ = MaVec2d( 0.0f, 0.0f );
			MousePrevDelta_ = MaVec2d( 0.0f, 0.0f );

			// Position in center of screen.
			::SetCursorPos( Rect.left + ( WX / 2 ), Rect.top + ( WY / 2 ) );

			if ( CursorInfo.flags == CURSOR_SHOWING )
			{
				while ( ShowCursor( FALSE ) >= 0 )
				{
				}
			}
		}
	}
	else
	{
		if ( CursorInfo.flags != CURSOR_SHOWING )
		{
			while ( ShowCursor( TRUE ) < 0 )
			{
			}
		}
	}

	MouseLocked_ = Enabled;
}

//////////////////////////////////////////////////////////////////////////
// mapKeyEvent
void OsClientWindows::mapKeyEvent( OsEventInputKeyboard& Event, WORD wParam )
{
	Event.KeyCode_ = static_cast< BcU16 >( wParam ) & 0xff;
	Event.ScanCode_ = ::MapVirtualKey( Event.KeyCode_, 0 );
	WORD AsciiOut = 0;
	BYTE KeyState[ 256 ];
	GetKeyboardState( KeyState );
	if( ::ToAscii( Event.KeyCode_, Event.ScanCode_, KeyState, &AsciiOut, 0 ) > 0 )
	{
		Event.AsciiCode_ = static_cast< BcU16 >( AsciiOut );
	}
	else
	{
		Event.AsciiCode_ = 0;
	}

	// Get key code, or pass through virtual.
	TKeyCodeMapIterator It( KeyCodeMap_.find( Event.KeyCode_ ) );
	if( It != KeyCodeMap_.end() )
	{
		Event.KeyCode_ = (*It).second;
	}
}

//////////////////////////////////////////////////////////////////////////
// centreMouse
void OsClientWindows::centreMouse()
{

}

//////////////////////////////////////////////////////////////////////////
// wndProcInternal
LRESULT OsClientWindows::wndProcInternal( HWND hWnd,
                                          UINT uMsg,
                                          WPARAM wParam,
                                          LPARAM lParam )
{
	// Handle messages
	switch (uMsg)
	{
	case WM_SYSCOMMAND:
		{
			switch ( wParam )
			{
				// Disable the screensaver and monitor power off events.
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				return 0;
				break;

			case SC_MINIMIZE:
				{
					// Get new window rect.
					::GetWindowRect( hWnd, &WindowSize_ );
					::GetClientRect( hWnd_, &ClientSize_ );

					// Send event.
					OsEventClient Event;
					Event.pClient_ = this;
					EvtPublisher::publish( osEVT_CLIENT_MINIMIZE, Event );
				}
				break;

			case SC_MAXIMIZE:
				{
					// Get new window rect.
					::GetWindowRect( hWnd, &WindowSize_ );
					::GetClientRect( hWnd_, &ClientSize_ );

					// Send event.
					OsEventClient Event;
					Event.pClient_ = this;
					EvtPublisher::publish( osEVT_CLIENT_MAXIMIZE, Event );
				}
				break;
			}
		}
		break;

	case WM_SIZE:
		{
			// Get new window rect.
			::GetWindowRect( hWnd, &WindowSize_ );
			::GetClientRect( hWnd_, &ClientSize_ );

			// Send resize event.
			OsEventClientResize Event;
			Event.pClient_ = this;
			Event.Width_ = getWidth();
			Event.Height_ = getHeight();
			EvtPublisher::publish( osEVT_CLIENT_RESIZE, Event );
		}
		break;
	case WM_KEYDOWN:
		{
			OsEventInputKeyboard Event;
			Event.DeviceID_ = 0;
			mapKeyEvent( Event, wParam );
			OsCore::pImpl()->publish( osEVT_INPUT_KEYDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_KEYDOWN, Event );
			return 0;
		}
		break;

	case WM_KEYUP:
		{
			OsEventInputKeyboard Event;
			Event.DeviceID_ = 0;
			mapKeyEvent( Event, wParam );			
			OsCore::pImpl()->publish( osEVT_INPUT_KEYUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_KEYUP, Event );
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		{
			/*
			if( MouseLocked_ == BcFalse )
			{
				OsEventInputMouse Event;
				Event.DeviceID_ = 0;
				Event.MouseX_ = lParam & 0xffff;
				Event.MouseY_ = lParam >> 16 & 0xffff;
				Event.MouseDX_ = Event.MouseX_ - PrevMouseX_;
				Event.MouseDY_ = Event.MouseY_ - PrevMouseY_;
				Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
				Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;

				Event.ButtonCode_ = 0;
				OsCore::pImpl()->publish( osEVT_INPUT_MOUSEMOVE, Event ); // TODO: REMOVE OLD!
				EvtPublisher::publish( osEVT_INPUT_MOUSEMOVE, Event );
			}
			*/
			return 0;
		}
		break;

	case WM_LBUTTONDOWN:
		{
			SetCapture( hWnd );
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
			Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
			//if( MouseLocked_ == BcFalse )
			{
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;
			}

			Event.ButtonCode_ = 0;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, Event );
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		{
			ReleaseCapture();
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
			Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
			//if( MouseLocked_ == BcFalse )
			{
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;
			}

			Event.ButtonCode_ = 0;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEUP, Event );
			return 0;
		}
		break;

	case WM_RBUTTONDOWN:
		{
			SetCapture( hWnd );
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
			Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
			//if( MouseLocked_ == BcFalse )
			{
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;
			}

			Event.ButtonCode_ = 1;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, Event );
			return 0;
		}
		break;

	case WM_RBUTTONUP:
		{
			ReleaseCapture();
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
			Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
			//if( MouseLocked_ == BcFalse )
			{
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;
			}

			Event.ButtonCode_ = 1;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEUP, Event );
			return 0;
		}
		break;

	case WM_MBUTTONDOWN:
		{
			SetCapture( hWnd );
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
			Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
			//if( MouseLocked_ == BcFalse )
			{
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;
			}

			Event.ButtonCode_ = 2;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, Event );
			return 0;
		}
		break;

	case WM_MBUTTONUP:
		{
			ReleaseCapture();
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
			Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
			//if( MouseLocked_ == BcFalse )
			{
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;
			}

			Event.ButtonCode_ = 2;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEUP, Event );
			return 0;
		}
		break;

	case WM_MOUSEWHEEL:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.MouseDX_ = (BcF32)(Event.MouseX_ - PrevMouseX_);
			Event.MouseDY_ = (BcF32)(Event.MouseY_ - PrevMouseY_);
			Event.NormalisedX_ = BcF32( Event.MouseX_ - getWidth() / 2 ) / BcF32( getWidth() );
			Event.NormalisedY_ = BcF32( Event.MouseY_ - getHeight() / 2 ) / BcF32( getHeight() );
			//if( MouseLocked_ == BcFalse )
			{
				PrevMouseX_ = Event.MouseX_;
				PrevMouseY_ = Event.MouseY_;
			}

			BcS16 WheelDirection = wParam >> 16 & 0xffff;

			Event.ButtonCode_ = WheelDirection > 0 ? 3 : 4;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEWHEEL, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEWHEEL, Event );
			return 0;
		}
		break;

	case WM_DESTROY:
	case WM_CLOSE:
		{
			// Send the close event.
			OsEventClient Event;
			Event.pClient_ = this;
			EvtPublisher::publish( osEVT_CLIENT_CLOSE, Event );

			// Post windows quit message.
			::PostQuitMessage( 0 );
			return 0;
		}
		break;
	}
	
	return ::DefWindowProc( hWnd, uMsg, wParam, lParam );
}

//////////////////////////////////////////////////////////////////////////
// WndProc
//static
LRESULT CALLBACK OsClientWindows::WndProc( 
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam )
{
	LONG_PTR ptr = ::GetWindowLongPtr( hWnd, GWLP_USERDATA );
	OsClientWindows* pWindow = reinterpret_cast< OsClientWindows* >( ptr );

	return pWindow->wndProcInternal( hWnd, uMsg, wParam, lParam );
}

//////////////////////////////////////////////////////////////////////////
// WndProc
//static
BOOL CALLBACK OsClientWindows::MonitorEnumProc(
	HMONITOR hMonitor, 
	HDC hDC, 
	LPRECT Rect, 
	LPARAM lParam )
{
	return TRUE;
}