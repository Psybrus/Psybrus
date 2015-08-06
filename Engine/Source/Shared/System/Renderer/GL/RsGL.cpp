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
	SupportDepthTextures_( BcFalse ),
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
	auto* Vendor = (const char*)glGetString( GL_VENDOR );
	auto* Renderer = (const char*)glGetString( GL_RENDERER );
	auto* Version = (const char*)glGetString( GL_VERSION );
	auto Extensions = (const char*)glGetString( GL_EXTENSIONS );
	PSY_LOG( "Vendor: %s", Vendor );
	PSY_LOG( "Renderer: %s", Renderer );
	PSY_LOG( "Version: %s", Version );
	PSY_LOG( "Extensions: %s", Extensions );

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
			SupportDepthTextures_ = BcTrue;
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
		SupportDXTTextures_ |= 
			HaveExtension( "EXT_texture_compression_dxt1" ) &&
			HaveExtension( "EXT_texture_compression_dxt3" ) &&
			HaveExtension( "EXT_texture_compression_dxt5" );
		SupportDXTTextures_ |= 
			HaveExtension( "EXT_texture_compression_dxt1" ) &&
			HaveExtension( "ANGLE_texture_compression_dxt3" ) &&
			HaveExtension( "ANGLE_texture_compression_dxt5" );

		SupportETC1Textures_ |= HaveExtension( "OES_compressed_ETC1_RGB8_texture" );
		SupportETC2Textures_ |= HaveExtension( "OES_compressed_ETC2_RGBA8_texture" );

		SupportATCTextures_ |= HaveExtension( "AMD_compressed_ATC_texture" );

		SupportDepthTextures_ |= HaveExtension( "OES_depth_texture" );
		SupportDepthTextures_ |= HaveExtension( "WEBGL_depth_texture" );
		SupportFloatTextures_ |= HaveExtension( "OES_texture_float" );
		SupportFloatTextures_ |= HaveExtension( "WEBGL_texture_float" );

		SupportHalfFloatTextures_ |= HaveExtension( "OES_texture_half_float" );
		SupportHalfFloatTextures_ |= HaveExtension( "WEBGL_texture_half_float" );

		SupportAnisotropicFiltering_ |= HaveExtension( "EXT_texture_filter_anisotropic" );

		SupportVAOs_ |= HaveExtension( "OES_vertex_array_object" );

		SupportDrawElementsBaseVertex_ |= HaveExtension( "EXT_draw_elements_base_vertex" );

		break;
	}

	glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureSlots_ );
	PSY_LOG( "GL_MAX_TEXTURE_IMAGE_UNITS: %u", MaxTextureSlots_ );
}

////////////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
BcBool RsOpenGLVersion::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	BcU32 CombinedVersion = Major_ << 16 | Minor_;

	switch( CodeType )
	{
	case RsShaderCodeType::GLSL_140:
		if( CombinedVersion >= 0x00030001 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_150:
		if( CombinedVersion >= 0x00030002 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_330:
		if( CombinedVersion >= 0x00030003 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_400:
		if( CombinedVersion >= 0x00040000 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_410:
		if( CombinedVersion >= 0x00040001 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_420:
		if( CombinedVersion >= 0x00040002 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_430:
		if( CombinedVersion >= 0x00040003 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_440:
		if( CombinedVersion >= 0x00040004 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_450:
		if( CombinedVersion >= 0x00040005 &&
			Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_ES_100:
		if( CombinedVersion >= 0x00020000 &&
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
GLuint RsReportGLErrors( const char* File, int Line )
{
	PSY_PROFILER_SECTION( CatchRoot, "RsReportGLErrors" );

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
		case GL_OUT_OF_MEMORY:
			ErrorString = "GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			ErrorString = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
#if !PLATFORM_ANDROID
		case GL_TABLE_TOO_LARGE:
			ErrorString = "GL_TABLE_TOO_LARGE";
			break;
		case GL_STACK_OVERFLOW:
			ErrorString = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			ErrorString = "GL_STACK_UNDERFLOW";
			break;
#endif
		}

		if( Error != 0 )
		{
			PSY_LOG( "RsGL: %s:%u: Error: %s\n", File, Line, ErrorString.c_str() );
			auto Result = BcBacktrace();
			BcPrintBacktrace( Result );
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
