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

#include "System/Renderer/GL/RsGL.h"

#include "Base/BcProfiler.h"

////////////////////////////////////////////////////////////////////////////////
// Utility.
namespace
{
	BcBool HaveExtension( const char* ExtensionName )
	{
		auto Extensions = (const char*)glGetString( GL_EXTENSIONS );
		auto RetVal = BcStrStr( Extensions, ExtensionName ) != nullptr;
		PSY_LOG( "RsGL: HaveExtension \"%s\"? %s\n", ExtensionName, RetVal ? "YES!" : "no" );
		return RetVal;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Ctor
RsOpenGLVersion::RsOpenGLVersion()
{
}

////////////////////////////////////////////////////////////////////////////////
// RsGLCatchError
RsOpenGLVersion::RsOpenGLVersion( BcS32 Major, BcS32 Minor, RsOpenGLType Type, RsShaderCodeType MaxCodeType ):
	Major_( Major ),
	Minor_( Minor ),
	Type_( Type ),
	MaxCodeType_( MaxCodeType ),
	SupportMRT_( BcFalse ),
	SupportSeparateBlendState_( BcFalse ),
	SupportDXTTextures_( BcFalse ),
	SupportNpotTextures_( BcFalse ),
	SupportFloatTextures_( BcFalse ),
	SupportHalfFloatTextures_( BcFalse ),
	SupportAnisotropicFiltering_( BcFalse ),
	SupportPolygonMode_( BcFalse ),
	SupportVAOs_( BcFalse ),
	SupportSamplerStates_( BcFalse ),
	SupportUniformBuffers_( BcFalse ),
	SupportGeometryShaders_( BcFalse ),
	SupportTesselationShaders_( BcFalse ),
	SupportComputeShaders_( BcFalse ),
	SupportAntialiasedLines_( BcFalse ),
	SupportDrawElementsBaseVertex_( BcFalse )
{

}

////////////////////////////////////////////////////////////////////////////////
// setupFeatureSupport
void RsOpenGLVersion::setupFeatureSupport()
{
	switch( Type_ )
	{
	case RsOpenGLType::COMPATIBILITY:
		BcBreakpoint;
		break;

	case RsOpenGLType::CORE:
		// 3.0
		if( Major_ >= 3 &&
			Minor_ >= 0 )
		{
			SupportMRT_ = BcTrue;
			SupportDXTTextures_ = BcTrue; // Ubiquous.
			SupportNpotTextures_ = BcTrue;
			SupportFloatTextures_ = BcTrue;
			SupportHalfFloatTextures_ = BcTrue;
			SupportAnisotropicFiltering_ = BcTrue;
			SupportPolygonMode_ = BcTrue;
			SupportVAOs_ = BcTrue;
			SupportAntialiasedLines_ = BcTrue;
		}

		// 3.1
		if( Major_ >= 3 &&
			Minor_ >= 1 )
		{
			SupportUniformBuffers_ = BcTrue;
			SupportGeometryShaders_ = BcTrue;
		}

		// 3.2
		if( Major_ >= 3 &&
			Minor_ >= 2 )
		{
			SupportDrawElementsBaseVertex_ = BcTrue;
		}

		// 3.3
		if( Major_ >= 3 &&
			Minor_ >= 3 )
		{
			SupportSamplerStates_ = BcTrue;
		}

		// 4.0
		if( Major_ >= 4 &&
			Minor_ >= 0 )
		{
			SupportSeparateBlendState_ = BcTrue;
		}

		// 4.2
		if( Major_ >= 4 &&
			Minor_ >= 2 )
		{
			// TODO: double check this.
			SupportTesselationShaders_ = BcTrue;
			SupportComputeShaders_ = BcTrue;
		}

		break;

	case RsOpenGLType::ES:
		SupportDXTTextures_ |= HaveExtension( "WEBGL_compressed_texture_s3tc" );
		SupportDXTTextures_ |= HaveExtension( "EXT_texture_compression_s3tc" );

		SupportFloatTextures_ |= HaveExtension( "OES_texture_float" );

		SupportHalfFloatTextures_ |= HaveExtension( "OES_texture_half_float" );

		SupportAnisotropicFiltering_ |= HaveExtension( "EXT_texture_filter_anisotropic" );

		SupportVAOs_ |= HaveExtension( "OES_vertex_array_object" );

		SupportDrawElementsBaseVertex_ |= HaveExtension( "EXT_draw_elements_base_vertex" );

		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
BcBool RsOpenGLVersion::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	switch( CodeType )
	{
	case RsShaderCodeType::GLSL_140:
		if( Major_ >= 3 &&
			Minor_ >= 1 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_150:
		if( Major_ >= 3 &&
			Minor_ >= 2 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_330:
		if( Major_ >= 3 &&
			Minor_ >= 3 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_400:
		if( Major_ >= 4 &&
			Minor_ >= 0 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_410:
		if( Major_ >= 4 &&
			Minor_ >= 1 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_420:
		if( Major_ >= 4 &&
			Minor_ >= 2 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_430:
		if( Major_ >= 4 &&
			Minor_ >= 3 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_440:
		if( Major_ >= 4 &&
			Minor_ >= 4 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_450:
		if( Major_ >= 4 &&
			Minor_ >= 5 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_ES_100:
		if( Major_ >= 2 &&
			Minor_ >= 0 &&
			Type_ == RsOpenGLType::ES )
		{
			return BcTrue;
		}
		break;
	default:
		break;
	}

	return BcFalse;
}

////////////////////////////////////////////////////////////////////////////////
// RsGLCatchError
#if PSY_GL_CATCH_ERRORS
GLuint RsGLCatchError()
{
	PSY_PROFILER_SECTION( CatchRoot, "RsGLCatchError" );

	BcU32 TotalErrors = 0;
	GLuint Error;
	do
	{
		Error = glGetError();
#if PSY_DEBUG
		std::string ErrorString = "UNKNOWN";
		switch( Error )
		{
		case GL_INVALID_ENUM:
			ErrorString = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			ErrorString = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			ErrorString = "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			ErrorString = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			ErrorString = "GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			ErrorString = "GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			ErrorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_TABLE_TOO_LARGE:
			ErrorString = "GL_TABLE_TOO_LARGE";
			break;
		}

		if( Error != 0 )
		{
			PSY_LOG( "RsGL: Error: %s\n", ErrorString.c_str() );
			++TotalErrors;
		}
#endif
	}
	while( Error != 0 );

	if( TotalErrors > 0 )
	{
		BcBreakpoint;
	}

	return Error;
}
#endif
