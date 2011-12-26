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

#include "BcTypes.h"
#include "OsWindows.h"
#include "OsClient.h"

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
	BcBool			create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen );

	/**
	*	Destroy window.
	*/
	void			destroy();

	/**
	 *  Get DC.
	 */
	BcHandle		getDC();

	BcBool centreWindow( BcS32 SizeX, BcS32 SizeY );
	RECT windowSize( BcS32 SizeX, BcS32 SizeY ) const;

private:
	LRESULT wndProcInternal( HWND, UINT, WPARAM, LPARAM );
	static LRESULT	CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

private:
	HDC				hDC_;
	HWND			hWnd_;
	BcHandle		hInstance_;
	BcChar			ClassName_[ 32 ];

	RECT			WindowSize_;
	DWORD			WindowStyleEx_;
	DWORD			WindowStyle_;
};

#endif
