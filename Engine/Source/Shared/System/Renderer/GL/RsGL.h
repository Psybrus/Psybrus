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

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "System/Renderer/RsTypes.h"

#include <tuple>

////////////////////////////////////////////////////////////////////////////////
// HTML5
#if PLATFORM_HTML5
#  include "GL/glew.h"

////////////////////////////////////////////////////////////////////////////////
// Linux
#elif PLATFORM_LINUX
#  include "GL/glew.h"

////////////////////////////////////////////////////////////////////////////////
// Mac
#elif PLATFORM_OSX
#  include <OpenGL/OpenGL.h>
#  include <AGL/agl.h>

extern AGLContext GAGLContext;

////////////////////////////////////////////////////////////////////////////////
// Win32
#elif PLATFORM_WINDOWS
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
#  include "GL/glew.h"
#  include "GL/wglew.h"
#endif

////////////////////////////////////////////////////////////////////////////////
// RsGLCatchError
void RsGLCatchError();

////////////////////////////////////////////////////////////////////////////////
// RsOpenGLType
enum class RsOpenGLType
{
	CORE = 0,
	COMPATIBILITY,
	ES,
	WEB
};

////////////////////////////////////////////////////////////////////////////////
// RsOpenGLVersion
struct RsOpenGLVersion
{
	RsOpenGLVersion()
	{
	}

	RsOpenGLVersion( BcU32 Major, BcU32 Minor, RsOpenGLType Type, RsShaderCodeType MaxCodeType ):
		Major_( Major ),
		Minor_( Minor ),
		Type_( Type ),
		MaxCodeType_( MaxCodeType )
	{
	}

	bool operator < ( const RsOpenGLVersion& Other ) const 
	{
		return std::make_tuple( Major_, Minor_, Type_, MaxCodeType_ ) < std::make_tuple( Other.Major_, Minor_, Type_, MaxCodeType_ );
	}

	BcU32 Major_;
	BcU32 Minor_;
	RsOpenGLType Type_;
	RsShaderCodeType MaxCodeType_;
};



#endif

