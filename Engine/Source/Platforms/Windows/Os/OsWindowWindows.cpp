/**************************************************************************
*
* File:		OsWindowWindows.cpp
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#include "OsWindowWindows.h"

#include "BcString.h"

//////////////////////////////////////////////////////////////////////////
// Class name generation.
static BcU32 gClassID_ = 0;

//////////////////////////////////////////////////////////////////////////
// Ctor
OsWindowWindows::OsWindowWindows()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
OsWindowWindows::~OsWindowWindows()
{

}

//////////////////////////////////////////////////////////////////////////
// create
BcBool OsWindowWindows::create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen )
{
	WNDCLASSEX	wc;
	DWORD		dwExStyle;
	DWORD		dwStyle;
	RECT		WindowRect;

	WindowRect.left = (long)0;
	WindowRect.right = (long)Width;
	WindowRect.top = (long)0;
	WindowRect.bottom = (long)Height;

	hInstance_ = Instance;

	// Generate class name.
	BcSPrintf( ClassName_, "PsybrusWindow%u", gClassID_++ );
	
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
		dwExStyle = WS_EX_WINDOWEDGE;
		dwStyle = WS_CAPTION | WS_SYSMENU | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP;	
	}

	::AdjustWindowRectEx( &WindowRect, dwStyle, FALSE, dwExStyle);

	// Create The Window
	if ( !( hWnd_=::CreateWindowEx(	dwExStyle,
		ClassName_,
		pTitle,
		dwStyle,
		0, 0,
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
	::ShowWindow( hWnd_, SW_SHOW );
	::SetForegroundWindow( hWnd_ );
	::SetFocus( hWnd_ );

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// destroy
void OsWindowWindows::destroy()
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
// getDC
BcHandle OsWindowWindows::getDC()
{
	return (BcHandle)hDC_;
}

//////////////////////////////////////////////////////////////////////////
// wndProcInternal
LRESULT OsWindowWindows::wndProcInternal( HWND hWnd,
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
			}
		}
		break;

	case WM_KEYDOWN:
		{
			//BcU32 KeyCode = static_cast< BcU32 >( wParam ) & 0xff;
			//pOsCoreImpl->setKey( KeyCode, BcTrue );

			//if( KeyCode == VK_F11 )
			//{
			//	pOsCoreImpl->reinitDisplay();
			//}
			return 0;
		}
		break;

	case WM_KEYUP:
		{
			//BcU32 KeyCode = static_cast< BcU32 >( wParam ) & 0xff;
			//pOsCoreImpl->setKey( KeyCode, BcFalse );
			return 0;
		}
		break;

	case WM_MOUSEMOVE:
		{
			return 0;
		}
		break;

	case WM_LBUTTONDOWN:
		{
			//pOsCoreImpl->setMouseButton( 0, BcTrue );
			return 0;
		}
		break;

	case WM_LBUTTONUP:
		{
			//pOsCoreImpl->setMouseButton( 0, BcFalse );
			return 0;
		}
		break;

	case WM_RBUTTONDOWN:
		{
			//pOsCoreImpl->setMouseButton( 1, BcTrue );
			return 0;
		}
		break;

	case WM_RBUTTONUP:
		{
			//pOsCoreImpl->setMouseButton( 1, BcFalse );
			return 0;
		}
		break;

	case WM_MBUTTONDOWN:
		{
			//pOsCoreImpl->setMouseButton( 2, BcTrue );
			return 0;
		}
		break;

	case WM_MBUTTONUP:
		{
			//pOsCoreImpl->setMouseButton( 2, BcFalse );
			return 0;
		}
		break;

	case WM_DESTROY:
	case WM_CLOSE:
		{
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
LRESULT CALLBACK OsWindowWindows::WndProc( HWND hWnd,
                                    UINT uMsg,
                                    WPARAM wParam,
                                    LPARAM lParam )
{
	LONG_PTR ptr = ::GetWindowLongPtr( hWnd, GWL_USERDATA );
	OsWindowWindows* pWindow = reinterpret_cast< OsWindowWindows* >( ptr );

	return pWindow->wndProcInternal( hWnd, uMsg, wParam, lParam );
}