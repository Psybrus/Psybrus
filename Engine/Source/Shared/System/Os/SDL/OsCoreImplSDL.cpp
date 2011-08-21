/**************************************************************************
*
* File:		OsCoreImplSDL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "OsCoreImplSDL.h"

#include "SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplSDL );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplSDL::OsCoreImplSDL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCoreImplSDL::~OsCoreImplSDL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void OsCoreImplSDL::open()
{
	BcPrintf( "Open SDL\n" );
	// Initialise SDL.
	if ( SDL_Init( SDL_INIT_VIDEO ) == 0 )
	{	
		// Setup video mode. Renderer crossover here...but OS tells renderer how big it's source window is.
		pScreenSurface_ = SDL_SetVideoMode( 1280, 720, 32, SDL_HWSURFACE | SDL_OPENGLBLIT /*| SDL_FULLSCREEN*/ );	
		
		// If we've created the surface, continue on.
		if( pScreenSurface_ != NULL )
		{
			// Setup depth buffer.
			SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
			SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );

			SDL_WM_SetCaption( "Lost Psyche", NULL );
				
			// Register SDL event handlers.
			registerSDLEventHandler( SDL_MOUSEMOTION,		&OsCoreImplSDL::processSDLEvent_InputMouse );
			registerSDLEventHandler( SDL_MOUSEBUTTONDOWN,	&OsCoreImplSDL::processSDLEvent_InputMouse );
			registerSDLEventHandler( SDL_MOUSEBUTTONUP,		&OsCoreImplSDL::processSDLEvent_InputMouse );
			registerSDLEventHandler( SDL_KEYDOWN,			&OsCoreImplSDL::processSDLEvent_InputKeyboard );
			registerSDLEventHandler( SDL_KEYUP,				&OsCoreImplSDL::processSDLEvent_InputKeyboard );
			registerSDLEventHandler( SDL_QUIT, 				&OsCoreImplSDL::processSDLEvent_Quit );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplSDL::update()
{
	// Process SDL events.
	processSDLEvents();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplSDL::close()
{
	// Tell SDL to quit.
	SDL_Quit();
}

//////////////////////////////////////////////////////////////////////////
// registerSDLEventHandler
void OsCoreImplSDL::registerSDLEventHandler( int Type, TSDLEventHandler func )
{
	SDLEventHandlerMap_[ Type ] = func;
}

//////////////////////////////////////////////////////////////////////////
// processSDLEvents
void OsCoreImplSDL::processSDLEvents()
{
	SDL_Event Event;
	
	// Throttle events a bit.
	BcU32 NoofEventsHandled = 0;
	
	// Poll SDL for an event.
	while( SDL_PollEvent( &Event ) /*&& NoofEventsHandled < 1*/ )
	{
		++NoofEventsHandled;
		
		// Find handler for event.
		TSDLEventHandlerMapIterator Iter = SDLEventHandlerMap_.find( Event.type );
		
		// If we've found a handler, call it.
		if( Iter != SDLEventHandlerMap_.end() )
		{
			TSDLEventHandler func = Iter->second;
			(this->*func)( Event );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// processSDLEvent_InputMouse
void OsCoreImplSDL::processSDLEvent_InputMouse( SDL_Event& Event )
{
	//
	switch( Event.type )
	{
	case SDL_MOUSEMOTION:
		{
			EventInputMouse_.DeviceID_ = Event.motion.which;
			EventInputMouse_.MouseX_ = Event.motion.x;
			EventInputMouse_.MouseY_ = Event.motion.y;
			EventInputMouse_.ButtonCode_ = -1;
			EvtPublisher::publish( osEVT_INPUT_MOUSEMOVE, EventInputMouse_ );
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
		{
			EventInputMouse_.DeviceID_ = Event.button.which;
			EventInputMouse_.MouseX_ = Event.motion.x;
			EventInputMouse_.MouseY_ = Event.motion.y;
			EventInputMouse_.ButtonCode_ = Event.button.button;
			EvtPublisher::publish( osEVT_INPUT_MOUSEUP, EventInputMouse_ );
		}
		break;

	case SDL_MOUSEBUTTONUP:
		{
			EventInputMouse_.DeviceID_ = Event.button.which;
			EventInputMouse_.MouseX_ = Event.motion.x;
			EventInputMouse_.MouseY_ = Event.motion.y;
			EventInputMouse_.ButtonCode_ = Event.button.button;
			EvtPublisher::publish( osEVT_INPUT_MOUSEDOWN, EventInputMouse_ );
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// processSDLEvent_InputKeyboard
void OsCoreImplSDL::processSDLEvent_InputKeyboard( SDL_Event& Event )
{
	//
	switch( Event.type )
	{
	case SDL_KEYDOWN:
		{
			EventInputKeyboard_.DeviceID_ = 0;
			EventInputKeyboard_.KeyCode_ = Event.key.keysym.sym;
			EvtPublisher::publish( osEVT_INPUT_KEYDOWN, EventInputKeyboard_ );
		}
		break;

	case SDL_KEYUP:
		{
			EventInputKeyboard_.DeviceID_ = 0;
			EventInputKeyboard_.KeyCode_ = Event.key.keysym.sym;
			EvtPublisher::publish( osEVT_INPUT_KEYUP, EventInputKeyboard_ );
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// processSDLEvent_Quit
void OsCoreImplSDL::processSDLEvent_Quit( SDL_Event& Event )
{
	EvtPublisher::publish( osEVT_CORE_QUIT, OsEventCore() );
}
