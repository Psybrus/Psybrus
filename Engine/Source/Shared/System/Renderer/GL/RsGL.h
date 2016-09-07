#pragma once

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "System/Renderer/RsFeatures.h"

#include <tuple>

////////////////////////////////////////////////////////////////////////////////
// HTML5
#if PLATFORM_HTML5
#  include "GLES2/gl2.h"
#  include "GLES2/gl2ext.h"
#  include "GLES3/gl3.h"
#  include "GLES3/gl3ext.h"
#  include "GLES3/gl31.h"

#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_1D_SHADOW 0x8B61

#define GL_IMAGE_1D 0x904C
#define GL_IMAGE_2D 0x904D
#define GL_IMAGE_3D 0x904E
#define GL_IMAGE_2D_RECT 0x904F
#define GL_IMAGE_CUBE 0x9050
#define GL_IMAGE_BUFFER 0x9051
#define GL_IMAGE_1D_ARRAY 0x9052
#define GL_IMAGE_2D_ARRAY 0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY 0x9054
#define GL_IMAGE_2D_MULTISAMPLE 0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY 0x9056

#define GL_UNSIGNED_INT_10_10_10_2 0x8036

#  define RENDER_USE_GLES

////////////////////////////////////////////////////////////////////////////////
// Android
#elif PLATFORM_ANDROID
#  include "GLES2/gl2.h"
#  include "GLES2/gl2ext.h"
#  include "GLES3/gl3.h"
#  include "GLES3/gl3ext.h"
#  include "GLES3/gl31.h"

#  include <EGL/egl.h>

#define GL_SAMPLER_1D 0x8B5D
#define GL_SAMPLER_1D_SHADOW 0x8B61

#define GL_IMAGE_1D 0x904C
#define GL_IMAGE_2D 0x904D
#define GL_IMAGE_3D 0x904E
#define GL_IMAGE_2D_RECT 0x904F
#define GL_IMAGE_CUBE 0x9050
#define GL_IMAGE_BUFFER 0x9051
#define GL_IMAGE_1D_ARRAY 0x9052
#define GL_IMAGE_2D_ARRAY 0x9053
#define GL_IMAGE_CUBE_MAP_ARRAY 0x9054
#define GL_IMAGE_2D_MULTISAMPLE 0x9055
#define GL_IMAGE_2D_MULTISAMPLE_ARRAY 0x9056

#define GL_UNSIGNED_INT_10_10_10_2 0x8036

#define GL_R16 0x822A
#define GL_R16_SNORM 0x8F98
#define GL_RG16 0x822C
#define GL_RG16_SNORM 0x822C
#define GL_RGBA16_SNORM 0x8F9B
#define GL_RGBA16 0x805B
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT 0x8C4D
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT 0x8C4E
#define GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT 0x8C4F
#define GL_COMPRESSED_RED_RGTC1 0x8DBB
#define GL_COMPRESSED_SIGNED_RED_RGTC1 0x8DBC
#define GL_COMPRESSED_RG_RGTC2 0x8DBD
#define GL_COMPRESSED_SIGNED_RG_RGTC2 0x8DBE
#define GL_COMPRESSED_RGBA_BPTC_UNORM 0x8E8C
#define GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM 0x8E8D
#define GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT 0x8E8E
#define GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT 0x8E8F


#  define RENDER_USE_GLES

#if ANDROID_NDK_VERSION >= 20
#  define RENDER_USE_GLES3
#endif

////////////////////////////////////////////////////////////////////////////////
// Linux
#elif PLATFORM_LINUX
#  include "GL/glew.h"

////////////////////////////////////////////////////////////////////////////////
// Mac
#elif PLATFORM_OSX
#  include "GL/glew.h"

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
// GLES defines.
#if !defined( RENDER_USE_GLES )
#define GL_ETC1_RGB8_OES           0x8D64
#define GL_TEXTURE_MAX_LEVEL       0x813D
#endif

////////////////////////////////////////////////////////////////////////////////
// RsGLCatchError
#define PSY_GL_CATCH_ERRORS ( 1 && !PSY_PRODUCTION && !PLATFORM_HTML5 )

GLuint RsReportGLErrors( const char* File, int Line, const char* CallString );

#if PSY_GL_CATCH_ERRORS
#  define GL( _call ) \
	gl##_call; RsReportGLErrors( __FILE__, __LINE__, #_call  )

#else
#  define GL( _call ) \
	gl##_call
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
	 * Will setup feature support + query extensions for active context and setup all the features supported.
	 */
	void setupFeatureSupport();

	/**
	 * Is shader code type supported?
	 */
	BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const;

	/**
	 * Get combined shader version.
	 */
	BcU32 getCombinedVersion() const { return Major_ << 16 | Minor_; }

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
	RsFeatures Features_;

	// GL specific features.
	bool SupportPolygonMode_;
	bool SupportVAOs_;
	bool SupportSamplerStates_;
	bool SupportBindBufferRange_;
	bool SupportUniformBuffers_;
	bool SupportUniformBufferOffset_;
	bool SupportImageLoadStore_;
	bool SupportShaderStorageBufferObjects_;
	bool SupportProgramInterfaceQuery_;
	bool SupportGeometryShaders_;
	bool SupportTesselationShaders_;
	bool SupportComputeShaders_;
	bool SupportDrawElementsBaseVertex_;
	bool SupportDrawInstanced_;
	bool SupportDrawInstancedBaseInstance_;
	bool SupportBlitFrameBuffer_;
	bool SupportCopyImageSubData_;
	GLint MaxVaryingFloats_;
	GLint MaxTextureSlots_;
	GLfloat MaxTextureAnisotropy_;

};

