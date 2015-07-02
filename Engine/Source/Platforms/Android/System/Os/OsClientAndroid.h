/**************************************************************************
*
* File:		OsClientAndroid.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#ifndef __OsClientAndroid_H__
#define __OsClientAndroid_H__

#include "Base/BcTypes.h"
#include "Math/MaVec2d.h"
#include "System/Os/OsClient.h"

//////////////////////////////////////////////////////////////////////////
// OsClientAndroid
class OsClientAndroid:
	public OsClient
{
public:
	OsClientAndroid( struct android_app* App );
	~OsClientAndroid();

	/**
	*	Create window.
	*/
	BcBool create( const BcChar* pTitle );

	/**
	 *	Update.
	 */
	virtual void update();


	/**
	*	Destroy window.
	*/
	void destroy();

	/**
	 * Get device handle.
	 */
	virtual BcHandle getDeviceHandle();

	/**
	 * Get window handle.
	 */
	virtual BcHandle getWindowHandle();
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
	 * Get window center.
	 */
	MaVec2d getWindowCentre() const;

	/**
	 * Set mouse lock.
	 */
	void setMouseLock( BcBool Enabled );

	/**
	 * Get Window ID.
	 */
	BcU32 getWindowId() const;

public:
	void setSize( BcU32 Width, BcU32 Height );

private:
	void pollLooper();

	void handleInput( struct AInputEvent* Event );

private:
	struct android_app* App_;

	typedef std::map< BcU64, BcU16 > TKeyCodeMap;
	typedef TKeyCodeMap::iterator TKeyCodeMapIterator;
	TKeyCodeMap KeyCodeMap_;

	BcBool MouseLocked_;
	
	BcS16 PrevMouseX_;
	BcS16 PrevMouseY_;

	MaVec2d MousePrevDelta_;
	MaVec2d MouseDelta_;
	MaVec2d MousePos_;

	BcU32 Width_;
	BcU32 Height_;
};

#endif
