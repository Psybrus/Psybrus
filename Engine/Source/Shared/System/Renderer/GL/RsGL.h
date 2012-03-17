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
#include "BcDebug.h"

////////////////////////////////////////////////////////////////////////////////
// Linux
#if PLATFORM_LINUX
#  include <GL/gl.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Mac
#if PLATFORM_OSX
#  include <OpenGL/OpenGL.h>
#  include <AGL/agl.h>

extern AGLContext GAGLContext;

#endif

////////////////////////////////////////////////////////////////////////////////
// Win32
#if PLATFORM_WINDOWS
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
#endif

//
#define RsGLCatchError									\
	{													\
		GLuint Error = glGetError();					\
		BcAssertMsg( Error == 0, "RsGL: Error (%s:%u): 0x%x\n", __FILE__, __LINE__, Error );		\
	}

#endif

