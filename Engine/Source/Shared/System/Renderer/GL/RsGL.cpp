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
	SupportPolygonMode_( false ),
	SupportVAOs_( false ),
	SupportSamplerStates_( false ),
	SupportUniformBuffers_( false ),
	SupportUniformBufferOffset_( false ),
	SupportImageLoadStore_( false ),
	SupportShaderStorageBufferObjects_( false ),
	SupportProgramInterfaceQuery_( false ),
	SupportGeometryShaders_( false ),
	SupportTesselationShaders_( false ),
	SupportComputeShaders_( false ),
	SupportDrawElementsBaseVertex_( false ),
	SupportDrawInstanced_( false ),
	SupportDrawInstancedBaseInstance_( false ),
	SupportBlitFrameBuffer_( false ),
	SupportCopyImageSubData_( false ),
	MaxTextureSlots_( 0 ),
	MaxTextureAnisotropy_( 0.0f )
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
		if( getCombinedVersion() >= 0x00030000 )
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
			Features_.TextureFormat_[ (int)RsTextureFormat::R10G10B10A2 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R11G11B10F ] = true;
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
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R10G10B10A2 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R11G11B10F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;

			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D16 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D32 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24S8 ] = true;

			Features_.MRT_= true;
			Features_.DepthTextures_ = true;
			Features_.NPOTTextures_ = true;
			Features_.AnisotropicFiltering_ = true;
			Features_.AntialiasedLines_ = true;

			SupportPolygonMode_ = true;
			SupportVAOs_ = true;
			SupportBindBufferRange_ = true;
		}

		// 3.1
		if( getCombinedVersion() >= 0x00030001 )
		{
			SupportUniformBuffers_ = true;
			SupportGeometryShaders_ = true;
			SupportDrawInstanced_ = true;
		}

		// 3.2
		if( getCombinedVersion() >= 0x00030002 )
		{
			SupportDrawElementsBaseVertex_ = true;
		}

		// 3.3
		if( getCombinedVersion() >= 0x00030003 )
		{
			SupportUniformBufferOffset_ = true;
			SupportSamplerStates_ = true;
			SupportBlitFrameBuffer_ = true;
		}

		// 4.0
		if( getCombinedVersion() >= 0x00040000 )
		{
			Features_.SeparateBlendState_ = true;
			SupportTesselationShaders_ = true;
		}

		// 4.1
		if( getCombinedVersion() >= 0x00040001 )
		{
		}

		// 4.2
		if( getCombinedVersion() >= 0x00040002 )
		{
			SupportImageLoadStore_ = true;
			SupportDrawInstancedBaseInstance_ = true;
		}

		// 4.3
		if( getCombinedVersion() >= 0x00040003 )
		{
			SupportComputeShaders_ = true;
			SupportShaderStorageBufferObjects_ = true;
			SupportProgramInterfaceQuery_ = true;
			SupportCopyImageSubData_ = true;
		}

		break;

	case RsOpenGLType::ES:
		if( getCombinedVersion() >= 0x00020000 )
		{
			Features_.MRT_ |=
				HaveExtension( "WEBGL_draw_buffers" ) |
				HaveExtension( "NV_draw_buffers" );
			Features_.Texture2D_ = true;
			Features_.Texture3D_ |= 
				HaveExtension( "OES_texture_3D" );
			Features_.TextureCube_ |= true;


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
				HaveExtension( "texture_compression_s3tc" ) ||
				HaveExtension( "compressed_texture_s3tc" );

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
			Features_.DepthTextures_ = SupportDepthTextures;

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
				//Features_.TextureFormat_[ (int)RsTextureFormat::R16F ] = true;
				//Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;

				//Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16F ] = true;
				//Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;
			}

			if( SupportFloatTextures )
			{
				//Features_.TextureFormat_[ (int)RsTextureFormat::R32F ] = true;
				//Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;

				//Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32F ] = true;
				//Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
				Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;

				if( SupportDepthTextures )
				{
					Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D32 ] = true;
				}
			}

			if( SupportDepthTextures )
			{
				Features_.TextureFormat_[ (int)RsTextureFormat::D16 ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::D24 ] = true;
				Features_.TextureFormat_[ (int)RsTextureFormat::D24S8 ] = true;
			}

			SupportVAOs_ |= HaveExtension( "OES_vertex_array_object" );

			SupportDrawElementsBaseVertex_ |= HaveExtension( "EXT_draw_elements_base_vertex" );

			SupportDrawInstanced_ |= 
				HaveExtension( "ANGLE_instanced_arrays" ) ||
				HaveExtension( "OES_instanced_arrays" );

			SupportDrawInstancedBaseInstance_ |= 
				HaveExtension( "ARB_base_instance" ) ||
				HaveExtension( "ARB_base_instance" );
		}

		break;
	}

	// General shared.
	Features_.ComputeShaders_ = SupportComputeShaders_;


	glGetIntegerv( GL_MAX_TEXTURE_IMAGE_UNITS, &MaxTextureSlots_ );
	PSY_LOG( "GL_MAX_TEXTURE_IMAGE_UNITS: %u", MaxTextureSlots_ );

	glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &MaxTextureAnisotropy_ );
	PSY_LOG( "GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT: %f", MaxTextureAnisotropy_ );
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
	GLuint LastError = 0;
	GLuint Error = 0;
	do
	{
		Error = glGetError();
#if !PSY_PRODUCTION
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
#if !defined( RENDER_USE_GLES )
		case GL_TABLE_TOO_LARGE:
			ErrorString = "GL_TABLE_TOO_LARGE";
			break;
		case GL_STACK_OVERFLOW:
			ErrorString = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			ErrorString = "GL_STACK_UNDERFLOW";
			break;
#endif // !defined( RENDER_USE_GLES )
		}

		if( Error != 0 )
		{
			PSY_LOG( "RsGL: %s:%u", File, Line );
			PSY_LOG( " - Call: %s\n", CallString );
			PSY_LOG( " - Error: %s", ErrorString.c_str() );
			auto Result = BcBacktrace();
			BcPrintBacktrace( Result );
			++TotalErrors;
			LastError = Error;
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
		//BcBreakpoint;
#endif
	}

	return LastError;
}
