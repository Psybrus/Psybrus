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
	bool HaveExtension( const char* ExtensionName )
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
	SupportPolygonMode_( BcFalse ),
	SupportVAOs_( BcFalse ),
	SupportSamplerStates_( BcFalse ),
	SupportUniformBuffers_( BcFalse ),
	SupportGeometryShaders_( BcFalse ),
	SupportTesselationShaders_( BcFalse ),
	SupportComputeShaders_( BcFalse ),
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

	// RT origin is bottom left in GL.
	Features_.RTOrigin_ = RsFeatureRenderTargetOrigin::BOTTOM_LEFT;

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
			Features_.MRT_ = true;

			Features_.Texture1D_ = true;
			Features_.Texture2D_ = true;
			Features_.Texture3D_ = true;
			Features_.TextureCube_ = true;

			Features_.TextureFormat_[ (int)RsTextureFormat::R8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8B8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8B8A8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::DXT1 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::DXT3 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::DXT5 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D16 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D24 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D32 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D24S8 ] = true;

			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8B8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8B8A8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;

			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D16 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D32 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24S8 ] = true;

			Features_.MRT_= BcTrue;
			Features_.DepthTextures_ = BcTrue;
			Features_.NPOTTextures_ = BcTrue;
			Features_.AnisotropicFiltering_ = BcTrue;
			Features_.AntialiasedLines_ = BcTrue;

			SupportPolygonMode_ = BcTrue;
			SupportVAOs_ = BcTrue;
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
			Features_.SeparateBlendState_ = BcTrue;
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
		if( Major_ >= 2 &&
			Minor_ >= 0 )
		{
			Features_.Texture2D_ = true;
			Features_.Texture3D_ |= 
				HaveExtension( "OES_texture_3D" );
			Features_.TextureCube_ |= 
				HaveExtension( "OES_texture_cube_map" );

			Features_.TextureFormat_[ (int)RsTextureFormat::R8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8B8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8B8A8 ] = true;

			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8B8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8B8A8 ] = true;

			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D16 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D32 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24S8 ] = true;

			Features_.AnisotropicFiltering_ = 
				HaveExtension( "EXT_texture_filter_anisotropic" );

			bool SupportDXTTextures = false;
			SupportDXTTextures |= 
				HaveExtension( "texture_compression_s3tc" );
			SupportDXTTextures |= 
				HaveExtension( "texture_compression_dxt1" ) &&
				HaveExtension( "texture_compression_dxt3" ) &&
				HaveExtension( "texture_compression_dxt5" );

			bool SupportETC1Textures = 
				HaveExtension( "OES_compressed_ETC1_RGB8_texture" );
			bool SupportETC2Textures = 
				HaveExtension( "OES_compressed_ETC2_RGBA8_texture" );

			bool SupportATCTextures = 
				HaveExtension( "AMD_compressed_ATC_texture" );

			bool SupportDepthTextures = 
				HaveExtension( "OES_depth_texture" ) |
				HaveExtension( "WEBGL_depth_texture" );

			bool SupportFloatTextures =
				HaveExtension( "OES_texture_float" ) |
				HaveExtension( "WEBGL_texture_float" );

			bool SupportHalfFloatTextures = 
				HaveExtension( "OES_texture_half_float" ) |
				HaveExtension( "WEBGL_texture_half_float" );

			if( SupportDXTTextures )
			{
				Features_.TextureFormat_[ (int)RsTextureFormat::DXT1 ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::DXT3 ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::DXT5 ] = true;
			}

			if( SupportETC1Textures )
			{
				Features_.TextureFormat_[ (int)RsTextureFormat::ETC1 ] = true;
			}

			if( SupportHalfFloatTextures )
			{
				Features_.TextureFormat_[ (int)RsTextureFormat::R16F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;

				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;
			}

			if( SupportFloatTextures )
			{
				Features_.TextureFormat_[ (int)RsTextureFormat::R32F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;

				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;

				if( SupportDepthTextures )
				{
					Features_.TextureFormat_[ (int)RsTextureFormat::D32F ] = true;
					Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D32F ] = true;
				}
			}

			if( SupportDepthTextures )
			{
				Features_.TextureFormat_[ (int)RsTextureFormat::D16 ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::D24 ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::D32 ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::D24S8 ] = true;
			}

			SupportVAOs_ |= HaveExtension( "OES_vertex_array_object" );

			SupportDrawElementsBaseVertex_ |= HaveExtension( "EXT_draw_elements_base_vertex" );
		}

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
	case RsShaderCodeType::GLSL_ES_300:
		if( CombinedVersion >= 0x00030000 &&
			Type_ == RsOpenGLType::ES )
		{
			return BcTrue;
		}
		break;
	case RsShaderCodeType::GLSL_ES_310:
		if( CombinedVersion >= 0x00030001 &&
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
GLuint RsReportGLErrors( const char* File, int Line, const char* CallString )
{
	PSY_PROFILER_SECTION( CatchRoot, "RsReportGLErrors" );
	BcAssert( File );
	BcAssert( Line > 0 );
	BcAssert( CallString );

#if 0
	PSY_LOG( "GL: %s", CallString );
#endif

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
			PSY_LOG( "RsGL: %s:%u", File, Line );
			PSY_LOG( " - Call: %s\n", CallString );
			PSY_LOG( " - Error: %s", ErrorString.c_str() );
			auto Result = BcBacktrace();
			BcPrintBacktrace( Result );
			++TotalErrors;
		}
#endif
	}
	while( Error != 0 );

	if( TotalErrors > 0 )
	{
#if PLATFORM_WINDOWS
		if( ::IsDebuggerPresent() )
		{
			BcBreakpoint;
		}
#else
		BcBreakpoint;
#endif
	}

	return Error;
}
#endif
