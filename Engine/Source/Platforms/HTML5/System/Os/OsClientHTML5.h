/**************************************************************************
*
* File:		OsClientHTML5.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#ifndef __OsClientHTML5_H__
#define __OsClientHTML5_H__

#include "Base/BcTypes.h"
#include "Math/MaVec2d.h"
#include "System/Os/OsHTML5.h"
#include "System/Os/OsClient.h"

//////////////////////////////////////////////////////////////////////////
// OsClientHTML5
class OsClientHTML5:
	public OsClient
{
public:
	OsClientHTML5();
	~OsClientHTML5();

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

	/**
	 * Handle event. Called from OsCoreImplSDL.
	 */
	void handleEvent( const SDL_Event& SDLEvent );

private:
	void handleKeyEvent( const SDL_Event& SDLEvent );
	void handleTextInputEvent( const SDL_Event& SDLEvent );
	void handleMouseEvent( const SDL_Event& SDLEvent );
	void handleWindowEvent( const SDL_Event& SDLEvent );
	void setWindowSize();

	typedef std::map< BcU64, BcU16 > TKeyCodeMap;
	typedef TKeyCodeMap::iterator TKeyCodeMapIterator;
	TKeyCodeMap KeyCodeMap_;

	BcBool MouseLocked_;
	
	BcS16 PrevMouseX_;
	BcS16 PrevMouseY_;

	MaVec2d MousePrevDelta_;
	MaVec2d MouseDelta_;
	MaVec2d MousePos_;

	SDL_Surface* SDLSurface_;
	BcU32 Width_;
	BcU32 Height_;
	int IsFullScreen_;
};

#endif
