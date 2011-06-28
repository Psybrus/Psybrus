/**************************************************************************
*
* File:		OsCoreImplSDL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSCOREIMPLSDL_H__
#define __OSCOREIMPLSDL_H__

#include "OsCore.h"

#include <SDL/SDL.h>

//////////////////////////////////////////////////////////////////////////
// OsCoreImplSDL
class OsCoreImplSDL:
	public OsCore
{
public:
	OsCoreImplSDL();
	virtual ~OsCoreImplSDL();
	
	virtual void 			open();
	virtual void 			update();
	virtual void 			close();

private:

private:
	typedef void (OsCoreImplSDL::*TSDLEventHandler)( SDL_Event& Event );
	typedef std::map< int, TSDLEventHandler > TSDLEventHandlerMap;
	typedef TSDLEventHandlerMap::iterator TSDLEventHandlerMapIterator;
	
	/**
	* Register handler for message.
	*/
	void					registerSDLEventHandler( int Type, TSDLEventHandler func );
	
	/**
	* Process the SDL message queue.
	*/
	void					processSDLEvents();
	
	/**
	* Process input (keyboard) messages.
	*/
	void					processSDLEvent_InputKeyboard( SDL_Event& Event );

	/**
	* Process input (mouse) messages.
	*/
	void					processSDLEvent_InputMouse( SDL_Event& Event );

	/**
	* Process quit message.
	*/
	void					processSDLEvent_Quit( SDL_Event& Event );
	
private:
	TSDLEventHandlerMap		SDLEventHandlerMap_;				///!< SDL Event handler map.

	SDL_Surface*			pScreenSurface_;					///!< Pointer to screen's surface.
	

	// Events.
	OsEventCore				EventCore_;
	OsEventInputKeyboard	EventInputKeyboard_;
	OsEventInputMouse		EventInputMouse_;
};

#endif

