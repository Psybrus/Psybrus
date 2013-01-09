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
#include "Base/BcVectors.h"
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

	/**
	*	Create window.
	*/
	BcBool			create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen, BcBool Visible );

	/**
	 *	Update.
	 */
	virtual void	update();


	/**
	*	Destroy window.
	*/
	void			destroy();

	/**
	 * Get device handle.
	 */
	virtual BcHandle getDeviceHandle();

	/**
 	 * Get width.
	 */
	virtual BcU32 getWidth() const;

	/**
 	 * Get height.
	 */
	virtual BcU32 getHeight() const;

	/**
	 * Centre the window.
	 */
	BcBool centreWindow( BcS32 SizeX, BcS32 SizeY );

	/**
	 * Set mouse lock.
	 */
	void setMouseLock( BcBool Enabled );

private:
	void mapKeyEvent( OsEventInputKeyboard& Event, WORD wParam );

	void centreMouse();

	LRESULT wndProcInternal( HWND, UINT, WPARAM, LPARAM );
	static LRESULT	CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

private:
	HDC				hDC_;
	HWND			hWnd_;
	BcHandle		hInstance_;
	BcChar			ClassName_[ 32 ];

	RECT			WindowSize_;
	RECT			ClientSize_;
	DWORD			WindowStyleEx_;
	DWORD			WindowStyle_;

	typedef std::map< WORD, BcU16 > TKeyCodeMap;
	typedef TKeyCodeMap::iterator TKeyCodeMapIterator;
	TKeyCodeMap		KeyCodeMap_;

	BcBool			MouseLocked_;
	
	BcS16			PrevMouseX_;
	BcS16			PrevMouseY_;

	BcVec2d			MousePrevDelta_;
	BcVec2d			MouseDelta_;
	BcVec2d			MousePos_;
};

#endif
