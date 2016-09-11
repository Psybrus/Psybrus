/**************************************************************************
*
* File:		OsClientSDL.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*			Window.
*		
*		
*		
* 
**************************************************************************/

#ifndef __OsClientSDL_H__
#define __OsClientSDL_H__

#include "Base/BcTypes.h"
#include "Math/MaVec2d.h"
#include "System/Os/SDL/OsSDL.h"
#include "System/Os/OsClient.h"

//////////////////////////////////////////////////////////////////////////
// OsClientSDL
class OsClientSDL:
	public OsClient
{
public:
	OsClientSDL();
	~OsClientSDL();

	BcBool create( const BcChar* pTitle, BcHandle Instance, BcU32 Width, BcU32 Height, BcBool Fullscreen, BcBool Visible );
	void destroy();

	void update() override;
	BcHandle getDeviceHandle() override;
	BcHandle getWindowHandle() override;
	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	BcU32 getDPI() const override;
	bool isActive() const override;
	bool isFocused() const override;
	void setMouseLock( BcBool Enabled ) override;
	void maximise() override;

	/**
	 * Get Window ID.
	 */
	BcU32 getWindowId() const;

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

	BcBool IsFocused_;
	BcBool MouseLocked_;	
	BcS16 PrevMouseX_;
	BcS16 PrevMouseY_;

	MaVec2d MousePrevDelta_;
	MaVec2d MouseDelta_;
	MaVec2d MousePos_;

	SDL_Window* SDLWindow_;
	BcU32 Width_;
	BcU32 Height_;
};

#endif
