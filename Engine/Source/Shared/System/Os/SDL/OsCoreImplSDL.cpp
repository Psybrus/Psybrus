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
	if ( SDL_Init( SDL_INIT_EVERYTHING ) != 0 )
	{
		BcPrintf( "SDL_Init Error: %u\n", SDL_GetError() );
		BcBreakpoint;
	}


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
	SDL_Quit();
}
