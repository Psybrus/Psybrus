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

	BcBool create( const BcChar* pTitle );
	void destroy();

	void update() override;
	BcHandle getDeviceHandle() override;
	BcHandle getWindowHandle() override;
	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	bool isActive() const override;
	bool isFocused() const override;
	void setMouseLock( BcBool Enabled ) override;
	void maximise() override;

public:
	void setSize( BcU32 Width, BcU32 Height );

private:
	void pollLooper();

	BcBool handleInput( struct AInputEvent* Event );

private:
	struct android_app* App_;
	struct ANativeWindow* Window_;
	bool IsActive_;

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
