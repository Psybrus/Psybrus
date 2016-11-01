/**************************************************************************
*
* File:		OsSDL.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __OSSDL_H__
#define __OSSDL_H__

#if LOCAL_SDL_LIBRARY
#  include <SDL.h>
#  include <SDL_syswm.h>
#else
#  include <SDL2/SDL.h>
#  include <SDL2/SDL_syswm.h>
#endif

#endif

