/**************************************************************************
*
* File:		RsGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		GL includes.
*		
*
*
* 
**************************************************************************/

#ifndef __RSGL_H__
#define __RSGL_H__

#include "BcTypes.h"

////////////////////////////////////////////////////////////////////////////////
// Linux
#if defined( PLATFORM_LINUX )
#  include <SDL/SDL.h>
#  include <GL/gl.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Mac
#if defined( PLATFORM_OSX )
#  include <SDL.h>
#  include <OpenGL/OpenGL.h>
#  include <AGL/agl.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Win32
#if defined( PLATFORM_WINDOWS )
#  define WIN32_LEAN_AND_MEAN
#  define NOGDICAPMASKS    
#  define NOMENUS         
#  define NORASTEROPS     
#  define OEMRESOURCE     
#  define NOATOM          
#  define NOCLIPBOARD     
#  define NODRAWTEXT      
#  define NONLS           
#  define NOMEMMGR        
#  define NOMETAFILE      
#  define NOMINMAX        
#  define NOOPENFILE      
#  define NOSCROLL        
#  define NOSERVICE       
#  define NOSOUND         
#  define NOTEXTMETRIC    
#  define NOWINOFFSETS    
#  define NOKANJI         
#  define NOHELP          
#  define NOPROFILER      
#  define NODEFERWINDOWPOS
#  define NOMCX
#  include "GLee.h"
#  include <SDL/SDL.h>
#endif

//
#define RsGLCatchError									\
	{													\
		GLuint Error = glGetError();					\
		if( Error != 0 )								\
		{												\
			BcPrintf( "RsGL: Error: 0x%x", Error );		\
			BcBreakpoint;								\
		}												\
	}

#endif

