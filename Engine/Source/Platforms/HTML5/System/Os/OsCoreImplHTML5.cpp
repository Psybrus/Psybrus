/**************************************************************************
*
* File:		OsCoreImplHTML5.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Os/OsCoreImplHTML5.h"
#include "System/Os/OsClientHTML5.h"

#include "System/SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplHTML5 );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplHTML5::OsCoreImplHTML5()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCoreImplHTML5::~OsCoreImplHTML5()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void OsCoreImplHTML5::open()
{
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
	{
		PSY_LOG( "SDL_Init Error: %u\n", SDL_GetError() );
		BcBreakpoint;
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplHTML5::update()
{
	OsCore::update();

	SDL_Event Event;
	{
		while( SDL_PollEvent( &Event ) )
		{
			switch( Event.type )
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEMOTION:
			case SDL_WINDOWEVENT:
				for( auto Client : ClientList_ )
				{
					OsClientHTML5* HTML5Client = dynamic_cast< OsClientHTML5* >( Client );
					HTML5Client->handleEvent( Event );
				}
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplHTML5::close()
{
	SDL_Quit();
}
