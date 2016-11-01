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

#include "System/Os/SDL/OsCoreImplSDL.h"
#include "System/Os/SDL/OsClientSDL.h"
#include "System/Os/SDL/OsInputDeviceGameControllerSDL.h"

#include "System/SysKernel.h"

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
	int Flags = SDL_INIT_EVENTS | SDL_INIT_VIDEO;
#if !PLATFORM_HTML5
	Flags |= SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER;
#endif

	if ( SDL_Init( Flags ) != 0 )
	{
		PSY_LOG( "SDL_Init Error: %s\n", SDL_GetError() );

		// Try fallback to just timer & events. We may just be running a server build.
		if ( SDL_Init( SDL_INIT_EVENTS ) != 0 )
		{
			PSY_LOG( "SDL_Init Error: %s\n", SDL_GetError() );
			BcBreakpoint;
		}
	}

	// Create default input devices.
	InputKeyboard_.reset( new OsInputDeviceKeyboard() );	
	InputMouse_.reset( new OsInputDeviceMouse() );	
	registerInputDevice( InputKeyboard_.get() );
	registerInputDevice( InputMouse_.get() );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplSDL::update()
{
	OsCore::update();

	SDL_Event Event;
	{
		while( SDL_PollEvent( &Event ) )
		{
			switch( Event.type )
			{
			case SDL_QUIT:
				pKernel()->stop();
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_TEXTINPUT:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
			case SDL_WINDOWEVENT:
				for( auto Client : ClientList_ )
				{
					OsClientSDL* SDLClient = dynamic_cast< OsClientSDL* >( Client );
					if( SDLClient->getWindowId() == Event.window.windowID )
					{
						SDLClient->handleEvent( Event );
					}
				}
				break;

			case SDL_CONTROLLERAXISMOTION:
				for( auto& GameController : InputGameControllers_ )
				{
					if( GameController->getPlayerID() == (BcU32)Event.caxis.which )
					{
						GameController->handleEvent( Event );
					}
				}
				break;
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				for( auto& GameController : InputGameControllers_ )
				{
					if( GameController->getPlayerID() == (BcU32)Event.cbutton.which )
					{
						GameController->handleEvent( Event );
					}
				}
				break;
			case SDL_CONTROLLERDEVICEADDED:
				{
					bool HasFound = false;
					for( auto& GameController : InputGameControllers_ )
					{
						if( GameController->getPlayerID() == (BcU32)Event.cdevice.which )
						{
							HasFound = true;
							GameController->handleEvent( Event );
							break;
						}
					}

					if( HasFound == false )
					{
						auto GameController = new OsInputDeviceGameControllerSDL( Event.cdevice.which );
						registerInputDevice( GameController );
						GameController->handleEvent( Event );
					}
				}
				break;
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				for( auto& GameController : InputGameControllers_ )
				{
					if( GameController->getPlayerID() == (BcU32)Event.cdevice.which )
					{
						GameController->handleEvent( Event );
					}
				}
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplSDL::close()
{
	unregisterInputDevice( InputKeyboard_.get() );
	unregisterInputDevice( InputMouse_.get() );
	InputKeyboard_.reset();
	InputMouse_.reset();

	SDL_Quit();
}
