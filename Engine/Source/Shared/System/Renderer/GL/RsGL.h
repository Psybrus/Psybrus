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
#if 1
inline void RsGLCatchError(){};
#else
void RsGLCatchError();
#endif

////////////////////////////////////////////////////////////////////////////////
// RsOpenGLType
enum class RsOpenGLType
{
	CORE = 0,
	COMPATIBILITY,
	ES
};

////////////////////////////////////////////////////////////////////////////////
// RsOpenGLVersion
struct RsOpenGLVersion
{
	RsOpenGLVersion();
	RsOpenGLVersion( BcS32 Major, BcS32 Minor, RsOpenGLType Type, RsShaderCodeType MaxCodeType );

	/**
	 * Will setuo feature support + query extensions for active context and setup all the features supported.
	 */
	void setupFeatureSupport();

	/**
	 * Is shader code type supported?
	 */
	BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const;

	bool operator < ( const RsOpenGLVersion& Other ) const 
	{
		return std::make_tuple( Major_, Minor_, Type_, MaxCodeType_ ) < std::make_tuple( Other.Major_, Minor_, Type_, MaxCodeType_ );
	}

	// Overall information.
	BcS32 Major_;
	BcS32 Minor_;
	RsOpenGLType Type_;
	RsShaderCodeType MaxCodeType_;

	// Features.
	BcBool SupportMRT_;
	BcBool SupportSeparateBlendState_;
	BcBool SupportDXTTextures_;
	BcBool SupportNpotTextures_;
	BcBool SupportFloatTextures_;
	BcBool SupportHalfFloatTextures_;
	BcBool SupportAnisotropicFiltering_;
	BcBool SupportPolygonMode_;
	BcBool SupportVAOs_;
	BcBool SupportSamplerStates_;
	BcBool SupportUniformBuffers_;
	BcBool SupportGeometryShaders_;
	BcBool SupportTesselationShaders_;
	BcBool SupportComputeShaders_;
	BcBool SupportAntialiasedLines_;
	BcBool SupportDrawElementsBaseVertex_;
};

#endif
