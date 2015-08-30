/**************************************************************************
*
* File:		OsClientWindows.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#ifndef __OsClientWindows_H__
#define __OsClientWindows_H__

#include "Base/BcTypes.h"
#include "Math/MaVec2d.h"
#include "System/Os/OsWindows.h"
#include "System/Os/OsClient.h"

//////////////////////////////////////////////////////////////////////////
// OsClientWindows
class OsClientWindows:
	public OsClient
{
public:
	OsClientWindows();
	~OsClientWindows();

	BcBool create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen, BcBool Visible );
	void destroy();

	void update() override;
	BcHandle getDeviceHandle() override;
	BcHandle getWindowHandle() override;
	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	bool isActive() const override;
	bool isFocused() const override;
	void setMouseLock( BcBool Enabled ) override;

	BcBool centreWindow( BcS32 SizeX, BcS32 SizeY );
	MaVec2d getWindowCentre() const;

private:
	void mapKeyEvent( OsEventInputKeyboard& Event, WPARAM wParam );

	void centreMouse();

	LRESULT wndProcInternal( HWND, UINT, WPARAM, LPARAM );
	static LRESULT	CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

	static BOOL CALLBACK MonitorEnumProc( HMONITOR, HDC, LPRECT, LPARAM );

private:
	std::vector< DEVMODEA > DeviceModes_;
	std::vector< DISPLAY_DEVICEA > DisplayDevices_;
	std::vector< MONITORINFO > MonitorInfos_;

	HDC				hDC_;
	HWND			hWnd_;
	BcHandle		hInstance_;
	BcChar			ClassName_[ 32 ];

	RECT			WindowSize_;
	RECT			ClientSize_;
	DWORD			WindowStyleEx_;
	DWORD			WindowStyle_;

	typedef std::map< BcU32, BcU16 > TKeyCodeMap;
	typedef TKeyCodeMap::iterator TKeyCodeMapIterator;
	TKeyCodeMap		KeyCodeMap_;

	BcBool			IsFocused_;
	BcBool			MouseLocked_;
	
	BcS16			PrevMouseX_;
	BcS16			PrevMouseY_;

	MaVec2d			MousePrevDelta_;
	MaVec2d			MouseDelta_;
	MaVec2d			MousePos_;
};

#endif
