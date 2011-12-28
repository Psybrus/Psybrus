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

#include "OsClientWindows.h"

#include "OsCore.h"

#include "BcString.h"

//////////////////////////////////////////////////////////////////////////
// Class name generation.
static BcU32 gClassID_ = 0;

//////////////////////////////////////////////////////////////////////////
// Ctor
OsClientWindows::OsClientWindows()
{

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
	::SetWindowLongPtr( hWnd_, GWL_USERDATA, (LONG_PTR)this );

	// Get the device context
	hDC_ = GetDC( hWnd_ );

	// Show the window
	if( Visible )
	{
		::ShowWindow( hWnd_, SW_SHOW );
		::SetForegroundWindow( hWnd_ );
		::SetFocus( hWnd_ );
	}

	return BcTrue;
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
	return WindowSize_.right - WindowSize_.left;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 OsClientWindows::getHeight() const
{
	return WindowSize_.bottom - WindowSize_.top;
}

//////////////////////////////////////////////////////////////////////////
// centreWindow
BcBool OsClientWindows::centreWindow( BcS32 SizeX, BcS32 SizeY )
{
	WindowSize_ = windowSize( SizeX, SizeY );

	BcBool RetValue = BcTrue;

	if( ::AdjustWindowRectEx( &WindowSize_, WindowStyle_, BcFalse, WindowStyleEx_ ) == FALSE )
	{
		RetValue  = BcFalse;
	}
	
	return RetValue; 
}

////////////////////////////////////////////////////////////////////////////////
// windowSize
RECT OsClientWindows::windowSize( BcS32 SizeX, BcS32 SizeY ) const
{
	RECT Rect;

	::SystemParametersInfo( SPI_GETWORKAREA, 0, &Rect, 0 );

	BcS32 Width = Rect.right - Rect.left;
	BcS32 Height = Rect.bottom - Rect.top;

	BcS32 slX = ( ( Width - SizeX ) / 2 ) + Rect.left;	// Default window to middle of screen
	BcS32 slY = ( ( Height - SizeY ) / 2 ) + Rect.top;

	RECT WindowSize;

	WindowSize.left = slX;
	WindowSize.top = slY;
	WindowSize.right = ( SizeX + slX );
	WindowSize.bottom = ( SizeY + slY ); 

	return WindowSize;
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

			// Send resize event.
			OsEventClientResize Event;
			Event.pClient_ = this;
			Event.Width_ = lParam & 0xffff;
			Event.Height_ = lParam >> 16 & 0xffff;
			EvtPublisher::publish( osEVT_CLIENT_RESIZE, Event );
		}
		break;
	case WM_KEYDOWN:
		{
			OsEventInputKeyboard Event;
			Event.DeviceID_ = 0;
			Event.KeyCode_ = static_cast< BcU32 >( wParam ) & 0xff;
			OsCore::pImpl()->publish( osEVT_INPUT_KEYDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_KEYDOWN, Event );
			return 0;
		}
		break;

	case WM_KEYUP:
		{
			OsEventInputKeyboard Event;
			Event.DeviceID_ = 0;
			Event.KeyCode_ = static_cast< BcU32 >( wParam ) & 0xff;
			OsCore::pImpl()->publish( osEVT_INPUT_KEYUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_KEYUP, Event );
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.ButtonCode_ = 0;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEMOVE, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEMOVE, Event );
			return 0;
		}
		break;

	case WM_LBUTTONDOWN:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.ButtonCode_ = 0;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, Event );
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.ButtonCode_ = 0;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEUP, Event );
			return 0;
		}
		break;

	case WM_RBUTTONDOWN:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.ButtonCode_ = 1;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, Event );
			return 0;
		}
		break;

	case WM_RBUTTONUP:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.ButtonCode_ = 1;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEUP, Event );
			return 0;
		}
		break;

	case WM_MBUTTONDOWN:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.ButtonCode_ = 2;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEDOWN, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, Event );
			return 0;
		}
		break;

	case WM_MBUTTONUP:
		{
			OsEventInputMouse Event;
			Event.DeviceID_ = 0;
			Event.MouseX_ = lParam & 0xffff;
			Event.MouseY_ = lParam >> 16 & 0xffff;
			Event.ButtonCode_ = 2;
			OsCore::pImpl()->publish( osEVT_INPUT_MOUSEUP, Event ); // TODO: REMOVE OLD!
			EvtPublisher::publish( osEVT_INPUT_MOUSEUP, Event );
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
LRESULT CALLBACK OsClientWindows::WndProc( HWND hWnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam )
{
	LONG_PTR ptr = ::GetWindowLongPtr( hWnd, GWL_USERDATA );
	OsClientWindows* pWindow = reinterpret_cast< OsClientWindows* >( ptr );

	return pWindow->wndProcInternal( hWnd, uMsg, wParam, lParam );
}