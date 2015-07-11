/**************************************************************************
*
* File:		RsContextGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/GL/RsContextGL.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsVertexDeclaration.h"

#include "System/Renderer/RsViewport.h"

#include "System/Os/OsClient.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

#include <memory>

#include "Import/Img/Img.h"

#include "System/SysKernel.h"

#if PLATFORM_HTML5
#include "System/Os/OsHTML5.h"
#endif

#if PLATFORM_ANDROID
#include "System/Os/OsClientAndroid.h"

#include <android_native_app_glue.h>
#endif

#include <algorithm>

#define ENABLE_DEBUG_OUTPUT ( 0 && !defined( PSY_PRODUCTION ) && !PLATFORM_HTML5 )

//////////////////////////////////////////////////////////////////////////
// Debug output.
#if ENABLE_DEBUG_OUTPUT
#if PLATFORM_WINDOWS
static void APIENTRY debugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam)
#else
static void debugOutput( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam )
#endif
{
	const char* SeverityStr = "";
	switch( severity )
	{
	case GL_DEBUG_SEVERITY_HIGH:
		SeverityStr = "GL_DEBUG_SEVERITY_HIGH";
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		SeverityStr = "GL_DEBUG_SEVERITY_MEDIUM";
		break;
	case GL_DEBUG_SEVERITY_LOW:
		SeverityStr = "GL_DEBUG_SEVERITY_LOW";
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		SeverityStr = "GL_DEBUG_SEVERITY_NOTIFICATION";
		break;
	}

	static bool ShowNotifications = false;

	if( severity != GL_DEBUG_SEVERITY_NOTIFICATION || ShowNotifications )
	{
		PSY_LOG( "Source: %x, Type: %x, Id: %x, Severity: %s\n - %s\n",
			source, type, id, SeverityStr, message );
	}
}
#endif // !defined( PSY_PRODUCTION )

//////////////////////////////////////////////////////////////////////////
// State value translation.
static GLenum gCompareMode[] = 
{
	GL_NEVER,
	GL_LESS,
	GL_EQUAL,
	GL_LEQUAL,
	GL_GREATER,
	GL_NOTEQUAL,
	GL_GEQUAL,
	GL_ALWAYS
};

static GLenum gBlendOp[] = 
{
	GL_FUNC_ADD,
	GL_FUNC_SUBTRACT,
	GL_FUNC_REVERSE_SUBTRACT,
	GL_MIN,
	GL_MAX,
};

static GLenum gBlendType[] = 
{
	GL_ZERO,
	GL_ONE,
	GL_SRC_COLOR,
	GL_ONE_MINUS_SRC_COLOR,
	GL_SRC_ALPHA,
	GL_ONE_MINUS_SRC_ALPHA,
	GL_DST_COLOR,
	GL_ONE_MINUS_DST_COLOR,
	GL_DST_ALPHA,
	GL_ONE_MINUS_DST_ALPHA,
};

static GLenum gStencilOp[] =
{
	GL_KEEP,
	GL_ZERO,
	GL_REPLACE,
	GL_INCR,
	GL_INCR_WRAP,
	GL_DECR,
	GL_DECR_WRAP,
	GL_INVERT
};

static GLenum gTextureFiltering[] = 
{
	// No mipmapping.
	GL_NEAREST,
	GL_LINEAR,
	
#if defined( RENDER_USE_GLES )
	// TODO: Figure out why mipmapping was a problem.
	// Mipmapping nearest
	GL_LINEAR, // GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR, // GL_LINEAR_MIPMAP_NEAREST,
	
	// Mipmapping linear
	GL_LINEAR, // GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR, // GL_LINEAR_MIPMAP_LINEAR
#else
	// Mipmapping nearest
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	
	// Mipmapping linear
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
#endif
};

static GLenum gTextureSampling[] = 
{
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_CLAMP_TO_EDGE,
#if !defined( RENDER_USE_GLES )
	GL_DECAL
#else
	GL_CLAMP_TO_EDGE
#endif
};

static GLenum gTextureTypes[] = 
{
	0,
#if !defined( RENDER_USE_GLES )
	GL_TEXTURE_1D,
#else
	0,
#endif
	GL_TEXTURE_2D,
	GL_TEXTURE_3D,
	GL_TEXTURE_CUBE_MAP
};

static GLenum gVertexDataTypes[] = 
{
	GL_FLOAT,			// RsVertexDataType::FLOAT32 = 0,
	GL_HALF_FLOAT,		// RsVertexDataType::FLOAT16,
	GL_FIXED,			// RsVertexDataType::FIXED,
	GL_BYTE,			// RsVertexDataType::BYTE,
	GL_BYTE,			// RsVertexDataType::BYTE_NORM,
	GL_UNSIGNED_BYTE,	// RsVertexDataType::UBYTE,
	GL_UNSIGNED_BYTE,	// RsVertexDataType::UBYTE_NORM,
	GL_SHORT,			// RsVertexDataType::SHORT,
	GL_SHORT,			// RsVertexDataType::SHORT_NORM,
	GL_UNSIGNED_SHORT,	// RsVertexDataType::USHORT,
	GL_UNSIGNED_SHORT,	// RsVertexDataType::USHORT_NORM,
	GL_INT,				// RsVertexDataType::INT,
	GL_INT,				// RsVertexDataType::INT_NORM,
	GL_UNSIGNED_INT,	// RsVertexDataType::UINT,
	GL_UNSIGNED_INT		// RsVertexDataType::UINT_NORM,
};

static GLboolean gVertexDataNormalised[] = 
{
	GL_FALSE,			// RsVertexDataType::FLOAT32 = 0,
	GL_FALSE,			// RsVertexDataType::FLOAT16,
	GL_FALSE,			// RsVertexDataType::FIXED,
	GL_FALSE,			// RsVertexDataType::BYTE,
	GL_TRUE,			// RsVertexDataType::BYTE_NORM,
	GL_FALSE,			// RsVertexDataType::UBYTE,
	GL_TRUE,			// RsVertexDataType::UBYTE_NORM,
	GL_FALSE,			// RsVertexDataType::SHORT,
	GL_TRUE,			// RsVertexDataType::SHORT_NORM,
	GL_FALSE,			// RsVertexDataType::USHORT,
	GL_TRUE,			// RsVertexDataType::USHORT_NORM,
	GL_FALSE,			// RsVertexDataType::INT,
	GL_TRUE,			// RsVertexDataType::INT_NORM,
	GL_FALSE,			// RsVertexDataType::UINT,
	GL_TRUE				// RsVertexDataType::UINT_NORM,
};

static GLuint gVertexDataSize[] = 
{
	4,					// RsVertexDataType::FLOAT32 = 0,
	2,					// RsVertexDataType::FLOAT16,
	4,					// RsVertexDataType::FIXED,
	1,					// RsVertexDataType::BYTE,
	1,					// RsVertexDataType::BYTE_NORM,
	1,					// RsVertexDataType::UBYTE,
	1,					// RsVertexDataType::UBYTE_NORM,
	2,					// RsVertexDataType::SHORT,
	2,					// RsVertexDataType::SHORT_NORM,
	2,					// RsVertexDataType::USHORT,
	2,					// RsVertexDataType::USHORT_NORM,
	4,					// RsVertexDataType::INT,
	4,					// RsVertexDataType::INT_NORM,
	4,					// RsVertexDataType::UINT,
	4					// RsVertexDataType::UINT_NORM,
};

static GLenum gTopologyType[] =
{
	GL_POINTS,						// RsTopologyType::POINTLIST = 0,
	GL_LINES,						// RsTopologyType::LINE_LIST,
	GL_LINE_STRIP,					// RsTopologyType::LINE_STRIP,
#if !defined( RENDER_USE_GLES )
	GL_LINES_ADJACENCY,				// RsTopologyType::LINE_LIST_ADJACENCY,
	GL_LINE_STRIP_ADJACENCY,		// RsTopologyType::LINE_STRIP_ADJACENCY,
#else
	0, 0,
#endif
	GL_TRIANGLES,					// RsTopologyType::TRIANGLE_LIST,
	GL_TRIANGLE_STRIP,				// RsTopologyType::TRIANGLE_STRIP,
#if !defined( RENDER_USE_GLES )
	GL_TRIANGLES_ADJACENCY,			// RsTopologyType::TRIANGLE_LIST_ADJACENCY,
	GL_TRIANGLE_STRIP_ADJACENCY,	// RsTopologyType::TRIANGLE_STRIP_ADJACENCY,
	GL_TRIANGLE_FAN,				// RsTopologyType::TRIANGLE_FAN,
	GL_PATCHES						// RsTopologyType::PATCHES,
#else
	0, 0, 0, 0
#endif
};

static GLenum gBufferType[] =
{
	0,								// RsBufferType::UNKNOWN
	GL_ARRAY_BUFFER,				// RsBufferType::VERTEX
	GL_ELEMENT_ARRAY_BUFFER,		// RsBufferType::INDEX
	GL_UNIFORM_BUFFER,				// RsBufferType::UNIFORM
#if !defined( RENDER_USE_GLES )
	GL_IMAGE_BUFFER,				// RsBufferType::UNORDERED_ACCESS
	GL_DRAW_INDIRECT_BUFFER,		// RsBufferType::DRAW_INDIRECT
	GL_TRANSFORM_FEEDBACK_BUFFER,	// RsBufferType::STREAM_OUT
#else
	0, 0, 0
#endif
};

static GLenum gTextureType[] =
{
	0,								// RsTextureType::UNKNOWN
#if !defined( RENDER_USE_GLES )
	GL_TEXTURE_1D,					// RsTextureType::TEX1D
#else
	0,
#endif
	GL_TEXTURE_2D,					// RsTextureType::TEX2D
	GL_TEXTURE_3D,					// RsTextureType::TEX3D
	GL_TEXTURE_CUBE_MAP,			// RsTextureType::TEXCUBE
};

struct RsTextureFormatGL
{
	BcBool Compressed_;
	BcBool DepthStencil_;
	GLint InternalFormat_;
	GLenum Format_;
	GLenum Type_;
};

static RsTextureFormatGL gTextureFormats[] =
{
	{ BcFalse, BcFalse, GL_NONE, GL_NONE, GL_NONE },			// RsTextureFormat::UNKNOWN,

	// Colour.	
	{ BcFalse, BcFalse, GL_RED, GL_RED, GL_UNSIGNED_BYTE },		// RsTextureFormat::R8,
	{ BcFalse, BcFalse, GL_RG, GL_RG, GL_UNSIGNED_BYTE },		// RsTextureFormat::R8G8,
	{ BcFalse, BcFalse, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE },		// RsTextureFormat::R8G8B8,
	{ BcFalse, BcFalse, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE },	// RsTextureFormat::R8G8B8A8,
	{ BcFalse, BcFalse, GL_R16F, GL_RED, GL_HALF_FLOAT },		// RsTextureFormat::R16F,
	{ BcFalse, BcFalse, GL_RG16F, GL_RG, GL_HALF_FLOAT },		// RsTextureFormat::R16FG16F,
	{ BcFalse, BcFalse, GL_RGB16F, GL_RGB, GL_HALF_FLOAT },		// RsTextureFormat::R16FG16FB16F,
	{ BcFalse, BcFalse, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT },	// RsTextureFormat::R16FG16FB16FA16F,
	{ BcFalse, BcFalse, GL_R32F, GL_RED, GL_FLOAT },			// RsTextureFormat::R32F,
	{ BcFalse, BcFalse, GL_RG32F, GL_RG, GL_FLOAT },			// RsTextureFormat::R32FG32F,
	{ BcFalse, BcFalse, GL_RGB32F, GL_RGB, GL_FLOAT },			// RsTextureFormat::R32FG32FB32F,
	{ BcFalse, BcFalse, GL_RGBA32F, GL_RGBA, GL_FLOAT },		// RsTextureFormat::R32FG32FB32FA32F,
	{ BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0 }, // RsTextureFormat::DXT1,
	{ BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0 }, // RsTextureFormat::DXT3,
	{ BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0 }, // RsTextureFormat::DXT5,
	// Depth stencil.
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT },	// RsTextureFormat::D16,
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT },		// RsTextureFormat::D24,
#if !defined( RENDER_USE_GLES )
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT },		// RsTextureFormat::D32,
#else
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT },		// RsTextureFormat::D32,
#endif
	{ BcFalse, BcTrue, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 },	// RsTextureFormat::D24S8,
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT },			// RsTextureFormat::D32F,
};

static GLenum gShaderType[] = 
{
	GL_VERTEX_SHADER,											// RsShaderType::VERTEX
	GL_FRAGMENT_SHADER,											// RsShaderType::PIXEL
#if !defined( RENDER_USE_GLES )
	GL_TESS_CONTROL_SHADER,										// RsShaderType::HULL
	GL_TESS_EVALUATION_SHADER,									// RsShaderType::DOMAIN
	GL_GEOMETRY_SHADER,											// RsShaderType::GEOMETRY
	GL_COMPUTE_SHADER,											// RsShaderType::COMPUTE
#else
	0, 0, 0, 0
#endif
};

namespace
{
	bool IsDepthFormat( RsTextureFormat Format )
	{
		switch( Format )
		{
		case RsTextureFormat::D16:
		case RsTextureFormat::D24:
		case RsTextureFormat::D32:
		case RsTextureFormat::D24S8:
		case RsTextureFormat::D32F:
			return true;
		default:
			break;
		}
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextGL::RsContextGL( OsClient* pClient, RsContextGL* pParent ):
	RsContext( pParent ),
	pParent_( pParent ),
	pClient_( pClient ),
	ScreenshotRequested_( BcFalse ),
	OwningThread_( BcErrorCode ),
	FrameCount_( 0 ),
	DirtyFrameBuffer_( BcTrue ),
	FrameBuffer_( nullptr ),
	DirtyViewport_( BcTrue ),
	Viewport_( 0, 0, 0, 0 ),
	DirtyScissor_( BcTrue ),
	ScissorX_( 0 ),
	ScissorY_( 0 ),
	ScissorW_( 0 ),
	ScissorH_( 0 ),
	GlobalVAO_( 0 ),
	ProgramDirty_( BcTrue ),
	BindingsDirty_( BcTrue ),
	Program_( nullptr ),
	IndexBuffer_( nullptr ),
	VertexDeclaration_( nullptr )
{
	BcMemZero( &TextureStateValues_[ 0 ], sizeof( TextureStateValues_ ) );
	BcMemZero( &TextureStateBinds_[ 0 ], sizeof( TextureStateBinds_ ) );
	BcMemZero( &VertexBuffers_[ 0 ], sizeof( VertexBuffers_ ) );
	BcMemZero( &UniformBuffers_[ 0 ], sizeof( UniformBuffers_ ) );
	RenderState_ = nullptr;
	LastRenderStateHandle_ = 0;

	NoofTextureStateBinds_ = 0;

	// Stats.
	NoofDrawCalls_ = 0;
	NoofRenderStateFlushes_ = 0;
	NoofRenderStates_ = 0;
	NoofSamplerStates_ = 0;
	NoofBuffers_ = 0;
	NoofTextures_ = 0;
	NoofShaders_ = 0;
	NoofPrograms_ = 0;

	TransferFBOs_[ 0 ] = 0;
	TransferFBOs_[ 1 ] = 0;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsContextGL::~RsContextGL()
{

}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 RsContextGL::getWidth() const
{
	return pClient_->getWidth();
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextGL::getHeight() const
{
	return pClient_->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextGL::getClient() const
{
	return pClient_;
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextGL::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	return Version_.isShaderCodeTypeSupported( CodeType );
}

//////////////////////////////////////////////////////////////////////////
// maxShaderCodeType
//virtual
RsShaderCodeType RsContextGL::maxShaderCodeType( RsShaderCodeType CodeType ) const
{
	return Version_.MaxCodeType_;
}

//////////////////////////////////////////////////////////////////////////
// presentBackBuffer
void RsContextGL::presentBackBuffer()
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsFrame::presentBackBuffer" );
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//PSY_LOG( "Draw calls: %u\n", NoofDrawCalls_ );
	//PSY_LOG( "Render state flushes: %u\n", NoofRenderStateFlushes_ );
	NoofDrawCalls_ = 0;
	NoofRenderStateFlushes_ = 0;

	if( ScreenshotRequested_ == BcFalse )
	{
		GL( Flush() );
		
	}
	else
	{
		setFrameBuffer( nullptr );
		flushState();

		// Finish all rendering.
		GL( Finish() );

#if !defined( RENDER_USE_GLES )
		BcU32 W = getWidth();
		BcU32 H = getHeight();

		// Read the back buffer.
		GL( ReadBuffer( GL_BACK ) );
		BcU32* pImageData = new BcU32[ W * H ];
		BcU32* pReadImageData = pImageData;
		GL( ReadPixels( 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, pImageData ) );
		
		// Convert to image.
		ImgImage* pImage = new ImgImage();
		pImage->create( W, H, NULL );
		
		for( BcU32 Y = 0; Y < H; ++Y )
		{
			BcU32 RealY = ( H - Y ) - 1;
			for( BcU32 X = 0; X < W; ++X )
			{
				ImgColour* pColour = (ImgColour*)pReadImageData++;
				pImage->setPixel( X, RealY, *pColour );
			}
		}
	
		PSY_LOG( "Took screenshot: %u x %u", W, H );

		// Save out image.				
		// NEILO TODO: Generate an automatic filename.
		Img::save( "screenshot.png", pImage );

		// Free image.
		delete pImage;

		// Free image data.
		delete [] pImageData;
#endif // !defined( RENDER_USE_GLES )

		// No more screenshot requested.
		ScreenshotRequested_ = BcFalse;
	}

	

#if PLATFORM_WINDOWS
	{
		PSY_PROFILER_SECTION( UpdateRoot, "::SwapBuffers" );
		::SwapBuffers( WindowDC_ );
	}
#endif

#if PLATFORM_LINUX || PLATFORM_OSX
	{
		PSY_PROFILER_SECTION( UpdateRoot, "SDL_GL_SwapWindow" );
		SDL_GL_SwapWindow( reinterpret_cast< SDL_Window* >( pClient_->getDeviceHandle() ) );
	}
#endif

#if PLATFORM_HTML5
	{
		PSY_PROFILER_SECTION( UpdateRoot, "SDL_GL_SwapBuffers" );
		SDL_GL_SwapBuffers();
	}
#endif

#if PLATFORM_ANDROID
	{
		PSY_PROFILER_SECTION( UpdateRoot, "SDL_eglSwapBuffers" );
		eglSwapBuffers( EGLDisplay_, EGLSurface_ );
	}
#endif
	// Advance frame.
	FrameCount_++;
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextGL::takeScreenshot()
{
	ScreenshotRequested_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsContextGL::create()
{
	bool WantGLES = SysArgs_.find( "-gles" ) != std::string::npos;

#if PLATFORM_WINDOWS
	// Get client device handle.
	WindowDC_ = (HDC)pClient_->getDeviceHandle();

	// Pixel format.
	static  PIXELFORMATDESCRIPTOR pfd =                 // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),                  // Size Of This Pixel Format Descriptor
		2,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		32,												// Select Our Color Depth
		0, 0, 0, 0, 0, 0,								// Color Bits Ignored
		0,												// No Alpha Buffer
		0,												// Shift Bit Ignored
		0,												// No Accumulation Buffer
		0, 0, 0, 0,										// Accumulation Bits Ignored
		24,												// 24 bit Z-Buffer (Depth Buffer)
		0,												// No Stencil Buffer
		0,												// No Auxiliary Buffer
		PFD_MAIN_PLANE,									// Main Drawing Layer
		0,												// Reserved
		0, 0, 0											// Layer Masks Ignored
	};
	
	GLuint PixelFormat = 0;
	if ( !(PixelFormat = ::ChoosePixelFormat( WindowDC_, &pfd ) ) )
	{
		PSY_LOG( "Can't create pixel format.\n" );
	}
	
	if( !::SetPixelFormat( WindowDC_, PixelFormat, &pfd ) )               // Are We Able To Set The Pixel Format?
	{
		PSY_LOG( "Can't Set The PixelFormat." );
	}

	// Create a rendering context to start with.
	WindowRC_ = wglCreateContext( WindowDC_ );
	BcAssertMsg( WindowRC_ != NULL, "RsCoreImplGL: Render context is NULL!" );

	// Make current.
	wglMakeCurrent( WindowDC_, WindowRC_ );

	// Init GLEW.
	glewExperimental = 1;
	GL( ewInit() );
	
	// Attempt to create core profile.
	RsOpenGLVersion Versions[] = 
	{
		RsOpenGLVersion( 4, 5, RsOpenGLType::CORE, RsShaderCodeType::GLSL_450 ),
		RsOpenGLVersion( 4, 4, RsOpenGLType::CORE, RsShaderCodeType::GLSL_440 ),
		RsOpenGLVersion( 4, 3, RsOpenGLType::CORE, RsShaderCodeType::GLSL_430 ),
		RsOpenGLVersion( 4, 2, RsOpenGLType::CORE, RsShaderCodeType::GLSL_420 ),
		RsOpenGLVersion( 4, 1, RsOpenGLType::CORE, RsShaderCodeType::GLSL_410 ),
		RsOpenGLVersion( 4, 0, RsOpenGLType::CORE, RsShaderCodeType::GLSL_400 ),
		RsOpenGLVersion( 3, 3, RsOpenGLType::CORE, RsShaderCodeType::GLSL_330 ),
		RsOpenGLVersion( 3, 2, RsOpenGLType::CORE, RsShaderCodeType::GLSL_150 ),
		RsOpenGLVersion( 2, 0, RsOpenGLType::ES, RsShaderCodeType::GLSL_ES_100 ),
	};

	HGLRC ParentContext = pParent_ != NULL ? pParent_->WindowRC_ : NULL;
	for( auto Version : Versions )
	{
		if( WantGLES && Version.Type_ != RsOpenGLType::ES )
		{
			continue;
		}

		if( createProfile( Version, ParentContext ) )
		{
			Version_ = Version;
			Version_.setupFeatureSupport();
			PSY_LOG( "RsContextGL: Created OpenGL %s %u.%u Profile.\n", 
				Version.Type_ == RsOpenGLType::CORE ? "Core" : ( Version.Type_ == RsOpenGLType::COMPATIBILITY ? "Compatibility" : "ES" ),
				Version.Major_, 
				Version.Minor_ );
			break;
		}
	}

	// If we have a parent, we need to share lists.
	if( pParent_ != NULL )
	{
		// Make parent current.
		wglMakeCurrent( pParent_->WindowDC_, WindowRC_ );

		// Share parent's lists with this context.
		BOOL Result = wglShareLists( pParent_->WindowRC_, WindowRC_ );
		BcAssertMsg( Result != BcFalse, "Unable to share lists." );
		BcUnusedVar( Result );

		// Make current.
		wglMakeCurrent( WindowDC_, WindowRC_ );
	}

	// Clear current errors.
	glGetError();
#endif

#if PLATFORM_LINUX	|| PLATFORM_OSX
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DEPTH_SIZE, 24 );
	SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	// Attempt to create core profile.
	RsOpenGLVersion Versions[] = 
	{
		RsOpenGLVersion( 4, 5, RsOpenGLType::CORE, RsShaderCodeType::GLSL_450 ),
		RsOpenGLVersion( 4, 4, RsOpenGLType::CORE, RsShaderCodeType::GLSL_440 ),
		RsOpenGLVersion( 4, 3, RsOpenGLType::CORE, RsShaderCodeType::GLSL_430 ),
		RsOpenGLVersion( 4, 2, RsOpenGLType::CORE, RsShaderCodeType::GLSL_420 ),
		RsOpenGLVersion( 4, 1, RsOpenGLType::CORE, RsShaderCodeType::GLSL_410 ),
		RsOpenGLVersion( 4, 0, RsOpenGLType::CORE, RsShaderCodeType::GLSL_400 ),
		RsOpenGLVersion( 3, 3, RsOpenGLType::CORE, RsShaderCodeType::GLSL_330 ),
		RsOpenGLVersion( 3, 2, RsOpenGLType::CORE, RsShaderCodeType::GLSL_150 ),
		RsOpenGLVersion( 2, 0, RsOpenGLType::ES, RsShaderCodeType::GLSL_ES_100 ),
	};

	BcAssert( pParent_ == nullptr );
	SDL_Window* Window = reinterpret_cast< SDL_Window* >( pClient_->getDeviceHandle() );
	bool Success = false;
	for( auto Version : Versions )
	{
		if( WantGLES && Version.Type_ != RsOpenGLType::ES )
		{
			continue;
		}

		if( createProfile( Version, Window ) )
		{
			Version_ = Version;
			Version_.setupFeatureSupport();
			PSY_LOG( "RsContextGL: Created OpenGL %s %u.%u Profile.\n", 
				Version.Type_ == RsOpenGLType::CORE ? "Core" : ( Version.Type_ == RsOpenGLType::COMPATIBILITY ? "Compatibility" : "ES" ),
				Version.Major_, 
				Version.Minor_ );
			break;
		}
	}

	BcAssert( SDLGLContext_ != nullptr );

	// Init GLEW.
	glewExperimental = 1;
	glewInit();
	glGetError();
#endif

#if PLATFORM_ANDROID
	// Use EGL to setup client.
	const EGLint EGLConfigAttribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 16,
		EGL_STENCIL_SIZE, 8,
		EGL_NONE
	};

	const EGLint EGContextAttribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	if ( ( EGLDisplay_ = eglGetDisplay( EGL_DEFAULT_DISPLAY ) ) == EGL_NO_DISPLAY )
	{
		BcAssertMsg( false, "eglGetDisplay() returned error %d", eglGetError() );
		return;
	}
	else
	{
		BcPrintf( "eglGetDisplay() success" );
	}

	if ( !eglInitialize( EGLDisplay_, 0, 0 ) )
	{
		BcAssertMsg( false, "eGL( Initialize() returned error %d", eglGetError() );
		return;
	}
	else
	{
		BcPrintf( "eglInitialize() success" );
	}

	if ( !eglChooseConfig( EGLDisplay_, EGLConfigAttribs, &EGLConfig_, 1, &EGLNumConfigs_ ) )
	{
		BcAssertMsg( false, "eGL( ChooseConfig() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglChooseConfig() success" );
	}

	if ( !eglGetConfigAttrib( EGLDisplay_, EGLConfig_, EGL_NATIVE_VISUAL_ID, &EGLFormat_ ) ) 
	{
		BcAssertMsg( false, "eGL( GetConfigAttrib() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglGetConfigAttrib() success" );
	}

	ANativeWindow* Window = static_cast< ANativeWindow* >( pClient_->getWindowHandle() );
	BcAssert( Window != nullptr );

	ANativeWindow_setBuffersGeometry( Window, 0, 0, EGLFormat_ );
	
	PSY_LOG( "ANativeWindow_setBuffersGeometry() success" );

	if ( !( EGLSurface_ = eglCreateWindowSurface( EGLDisplay_, EGLConfig_, Window, 0 ) ) )
	{
		PSY_LOG( "eglCreateWindowSurface() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglCreateWindowSurface() success" );
	}
	
	if ( !( EGLContext_ = eglCreateContext( EGLDisplay_, EGLConfig_, 0, EGContextAttribs ) ) )
	{
		PSY_LOG( "eglCreateContext() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglCreateContext() success" );
	}
	
	if ( !eglMakeCurrent( EGLDisplay_, EGLSurface_, EGLSurface_, EGLContext_ ) )
	{
		PSY_LOG( "eglMakeCurrent() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglMakeCurrent() success" );
	}

	if ( !eglQuerySurface( EGLDisplay_, EGLSurface_, EGL_WIDTH, &EGLWidth_ ) ||
		 !eglQuerySurface( EGLDisplay_, EGLSurface_, EGL_HEIGHT, &EGLHeight_ ) ) 
	{
		PSY_LOG( "eglQuerySurface() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglQuerySurface() success" );
		OsClientAndroid* Client = static_cast< OsClientAndroid* >( pClient_ );
		Client->setSize( EGLWidth_, EGLHeight_ );
	}

#endif

#if defined( RENDER_USE_GLES )
	Version_ = RsOpenGLVersion( 2, 0, RsOpenGLType::ES, RsShaderCodeType::GLSL_ES_100 );
	Version_.setupFeatureSupport();
	PSY_LOG( "RsContextGL: Created OpenGL %s %u.%u Profile.\n", 
		Version_.Type_ == RsOpenGLType::CORE ? "Core" : ( Version_.Type_ == RsOpenGLType::COMPATIBILITY ? "Compatibility" : "ES" ),
		Version_.Major_, 
		Version_.Minor_ );

#  if PLATFORM_HTML5
	// Init GLEW.
	glewExperimental = 1;
	glewInit();
	glGetError();
#  endif
#endif

	// Debug output extension.	
#if ENABLE_DEBUG_OUTPUT
	if( GLEW_ARB_debug_output )
	{
		GL( DebugMessageCallbackARB( debugOutput, nullptr ) );
		GL( GetError() );
	}
#endif // ENABLE_DEBUG_OUTPUT

	

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	
	// Create + bind global VAO.
	if( Version_.SupportVAOs_ )
	{
		PSY_LOG( "Create global VBO" );
		BcAssert( glGenVertexArrays != nullptr );
		BcAssert( glBindVertexArray != nullptr );
		GL( GenVertexArrays( 1, &GlobalVAO_ ) );
		GL( BindVertexArray( GlobalVAO_ ) );
		
	}

	// Create transfer FBO.
	PSY_LOG( "Create transfer FBOs" );
	GL( GenFramebuffers( 2, TransferFBOs_ ) );
	

	// Force set render state to the default.
	// Initialises redundant state caching.
	PSY_LOG( "Set default render state" );
	RsRenderStateDesc RenderStateDesc = BoundRenderStateDesc_;
	setRenderStateDesc( RenderStateDesc, BcTrue );

	// Set default state.
	// BINDING, not state..
		setDefaultState();

	// Clear screen and flip.
	clear( RsColour( 0.0f, 0.0f, 0.0f, 0.0f ), BcTrue, BcTrue, BcTrue );

	// Present back buffer.
	presentBackBuffer();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsContextGL::update()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsContextGL::destroy()
{
	// Destroy transfer FBO.
	GL( DeleteFramebuffers( 2, TransferFBOs_ ) );

	// Destroy global VAO.
	GL( BindVertexArray( 0 ) );
	GL( DeleteVertexArrays( 1, &GlobalVAO_ ) );

#if PLATFORM_WINDOWS
	// Destroy rendering context.
	wglMakeCurrent( WindowDC_, NULL );
	wglDeleteContext( WindowRC_ );
#endif

#if PLATFORM_LINUX || PLATFORM_OSX
	SDL_GL_DeleteContext( SDLGLContext_ );
#endif

	// Dump stats.
	PSY_LOG( "Number of render states left: %u\n", NoofRenderStates_ );
	PSY_LOG( "Number of sampler states left: %u\n", NoofSamplerStates_ );
	PSY_LOG( "Number of buffers left: %u\n", NoofBuffers_ );
	PSY_LOG( "Number of textures left: %u\n", NoofTextures_ );
	PSY_LOG( "Number of shaders left: %u\n", NoofShaders_ );
	PSY_LOG( "Number of programs left: %u\n", NoofPrograms_ );
}

//////////////////////////////////////////////////////////////////////////
// createProfile
#if PLATFORM_WINDOWS
bool RsContextGL::createProfile( RsOpenGLVersion Version, HGLRC ParentContext )
{
	int ContextAttribs[] = 
	{
		WGL_CONTEXT_PROFILE_MASK_ARB, 0,
		WGL_CONTEXT_MAJOR_VERSION_ARB, Version.Major_,
		WGL_CONTEXT_MINOR_VERSION_ARB, Version.Minor_,
		NULL
	};

	switch( Version.Type_ )
	{
	case RsOpenGLType::CORE:
		ContextAttribs[ 1 ] = WGL_CONTEXT_CORE_PROFILE_BIT_ARB;
		break;
	case RsOpenGLType::COMPATIBILITY:
		ContextAttribs[ 1 ] = WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB;
		break;
	case RsOpenGLType::ES:
		if( Version.Major_ == 2 )
		{
			ContextAttribs[ 1 ] = WGL_CONTEXT_ES2_PROFILE_BIT_EXT;
		}
		else if( Version.Major_ = 3 )
		{
			BcBreakpoint;
#if 0 
			ContextAttribs[ 1 ] = WGL_CONTEXT_ES3_PROFILE_BIT_EXT;
#endif
		}
		else
		{
			BcBreakpoint;
		}
		break;
	}
	
	BcAssert( WGL_ARB_create_context );
	BcAssert( WGL_ARB_create_context_profile );

	auto func = wglCreateContextAttribsARB;
	BcUnusedVar( func );

	HGLRC CoreProfile = wglCreateContextAttribsARB( WindowDC_, ParentContext, ContextAttribs );
	if( CoreProfile != NULL )
	{
		// release old context.
		wglMakeCurrent( WindowDC_, NULL );
		wglDeleteContext( WindowRC_ );

		// make new current.
		wglMakeCurrent( WindowDC_, CoreProfile );

		// Assign new.
		WindowRC_ = CoreProfile;

		return true;
	}
	return false;
}
#endif

//////////////////////////////////////////////////////////////////////////
// createProfile
#if PLATFORM_LINUX || PLATFORM_OSX
bool RsContextGL::createProfile( RsOpenGLVersion Version, SDL_Window* Window )
{
	switch( Version.Type_ )
	{
	case RsOpenGLType::CORE:
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
		break;
	case RsOpenGLType::COMPATIBILITY:
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY );
		break;
	case RsOpenGLType::ES:
		SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES );
		break;
	default:
		BcBreakpoint;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, Version.Major_ );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, Version.Minor_ );

	SDLGLContext_ = SDL_GL_CreateContext( Window );

	if( SDLGLContext_ != nullptr )
	{
		SDL_GL_MakeCurrent( Window, SDLGLContext_ );
		SDL_GL_SetSwapInterval( 1 );
	}

	return SDLGLContext_ != nullptr;
}
#endif


//////////////////////////////////////////////////////////////////////////
// createRenderState
bool RsContextGL::createRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Create hash for desc for quick checking of redundant state checking.
	// Super dirty and temporary.
	const auto& Desc = RenderState->getDesc();
	BcU64 HashA = BcHash::GenerateCRC32( 0, &Desc, sizeof( Desc ) );
	BcU64 HashB = BcHash::GenerateAP( &Desc, sizeof( Desc ) );
	BcU64 Hash = HashA | ( HashB << 32 );

	auto FoundIt = RenderStateMap_.find( Hash );
	if( FoundIt != RenderStateMap_.end() )
	{
		BcAssert( BcMemCompare( &Desc, &FoundIt->second, sizeof( Desc ) ) );
	}
	else
	{
		RenderStateMap_[ Hash ] = Desc;
	}

	++NoofRenderStates_;
	RenderState->setHandle( Hash );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextGL::destroyRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	--NoofRenderStates_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextGL::createSamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

#if !defined( RENDER_USE_GLES )
	if( Version_.SupportSamplerStates_ )
	{
		GLuint SamplerObject = (GLuint)-1;
		GL( GenSamplers( 1, &SamplerObject ) );
		

		// Setup sampler parmeters.
		const auto& SamplerStateDesc = SamplerState->getDesc();

		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_MIN_FILTER, gTextureFiltering[ (BcU32)SamplerStateDesc.MinFilter_ ] ) );
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_MAG_FILTER, gTextureFiltering[ (BcU32)SamplerStateDesc.MagFilter_ ] ) );
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_WRAP_S, gTextureSampling[ (BcU32)SamplerStateDesc.AddressU_ ] ) );
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_WRAP_T, gTextureSampling[ (BcU32)SamplerStateDesc.AddressV_ ] ) );	
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_WRAP_R, gTextureSampling[ (BcU32)SamplerStateDesc.AddressW_ ] ) );	
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_COMPARE_MODE, GL_NONE ) );
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ) );
		

		++NoofSamplerStates_;

		// Set handle.
		SamplerState->setHandle< GLuint >( SamplerObject );
		return SamplerObject != -1;
	}
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextGL::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

#if !defined( RENDER_USE_GLES )
	// GL3.3 minimum
	if( Version_.SupportSamplerStates_ )
	{
		GLuint SamplerObject = SamplerState->getHandle< GLuint >();
		GL( DeleteSamplers( 1, &SamplerObject ) );

		--NoofSamplerStates_;		
	}
#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextGL::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& Desc = FrameBuffer->getDesc();
	BcAssertMsg( Desc.RenderTargets_.size() < GL_MAX_COLOR_ATTACHMENTS, "Too many targets" );

	// Generate FBO.
	GLuint Handle;
	GL( GenFramebuffers( 1, &Handle ) );
	FrameBuffer->setHandle( Handle );

	

	// Bind.
	GL( BindFramebuffer( GL_FRAMEBUFFER, Handle ) );

	// Attach colour targets.
	BcU32 NoofAttachments = 0;
	for( auto Texture : Desc.RenderTargets_ )
	{
		if( Texture != nullptr )
		{
			BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) !=
				RsResourceBindFlags::NONE );
			BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) !=
				RsResourceBindFlags::NONE );
			RsTextureImplGL* TextureImpl = Texture->getHandle< RsTextureImplGL* >();
			glFramebufferTexture2D( 
				GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0 + NoofAttachments,
				GL_TEXTURE_2D,
				TextureImpl->Handle_,
				0 );
		}
	}

	// Attach depth stencil target.
	if( Desc.DepthStencilTarget_ != nullptr )
	{
		const auto& DSDesc = Desc.DepthStencilTarget_->getDesc();
		auto Attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		switch ( DSDesc.Format_ )
		{
		case RsTextureFormat::D16:
		case RsTextureFormat::D24:
		case RsTextureFormat::D32:
		case RsTextureFormat::D32F:
			Attachment = GL_DEPTH_ATTACHMENT;
			break;
		case RsTextureFormat::D24S8:
			Attachment = GL_DEPTH_STENCIL_ATTACHMENT;
			break;
		default:
			BcAssertMsg( false, "Invalid depth stencil format." );
			break;
		}

		BcAssert( ( Desc.DepthStencilTarget_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) !=
			RsResourceBindFlags::NONE );
		BcAssert( ( Desc.DepthStencilTarget_->getDesc().BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) !=
			RsResourceBindFlags::NONE );

		RsTextureImplGL* TextureImpl = Desc.DepthStencilTarget_->getHandle< RsTextureImplGL* >();
		glFramebufferTexture2D( 
			GL_FRAMEBUFFER,
			Attachment,
			GL_TEXTURE_2D,
			TextureImpl->Handle_,
			0 );
	}

	// Check status.
	auto Status = GL( CheckFramebufferStatus( GL_FRAMEBUFFER ) );
	BcAssertMsg( Status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete" );

	// Unbind.
	GL( BindFramebuffer( GL_FRAMEBUFFER, 0 ) );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextGL::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	GLuint Handle = FrameBuffer->getHandle< GLuint >();

	if( Handle != 0 )
	{
		GL( DeleteFramebuffers( 1, &Handle ) );
		FrameBuffer->setHandle< GLuint >( 0 );

		
		return true;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextGL::createBuffer( RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& BufferDesc = Buffer->getDesc();

	BcAssert( BufferDesc.SizeBytes_ > 0 );

	// Get buffer type for GL.
	auto TypeGL = gBufferType[ (BcU32)BufferDesc.Type_ ];

	// Get usage flags for GL.
	GLuint UsageFlagsGL = 0;
	
	// Data update frequencies.
	if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STATIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STATIC_DRAW;
	}
	else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_DYNAMIC_DRAW;
	}
	else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STREAM_DRAW;
	}

	// Create buffer impl.
	auto BufferImpl = new RsBufferImplGL();
	Buffer->setHandle( BufferImpl );

	// Should buffer be in main memory?
	BcBool BufferInMainMemory = 
		Version_.SupportUniformBuffers_ == BcFalse &&
		Buffer->getDesc().Type_ == RsBufferType::UNIFORM;

	++NoofBuffers_;


	// Determine which kind of buffer to create.
	if( !BufferInMainMemory )
	{
		// Generate buffer.
		GL( GenBuffers( 1, &BufferImpl->Handle_ ) );

		// Catch gen error.
		

		// Attempt to update it.
		if( BufferImpl->Handle_ != 0 )
		{
			GL( BindBuffer( TypeGL, BufferImpl->Handle_ ) );
			GL( BufferData( TypeGL, BufferDesc.SizeBytes_, nullptr, UsageFlagsGL ) );

			// Catch update error.
			
			return true;
		}
	}
	else
	{
		// Buffer is in main memory.
		BufferImpl->BufferData_ = new BcU8[ BufferDesc.SizeBytes_ ];
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextGL::destroyBuffer( RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Is buffer be in main memory?
	BcBool BufferInMainMemory =
		Version_.SupportUniformBuffers_ == BcFalse &&
		Buffer->getDesc().Type_ == RsBufferType::UNIFORM;

	--NoofBuffers_;

	// Check if it is bound anywhere.
	if( Buffer == IndexBuffer_ )
	{
		IndexBuffer_ = nullptr;
	}
	for( auto& VertexBufferBinding : VertexBuffers_ )
	{
		if( Buffer == VertexBufferBinding.Buffer_ )
		{
			VertexBufferBinding.Buffer_ = nullptr;
		}
	}
	for( auto& UniformBufferBinding : UniformBuffers_ )
	{
		if( Buffer == UniformBufferBinding.Buffer_ )
		{
			UniformBufferBinding.Buffer_ = nullptr;
		}
	}

	bool RetVal = false;
	auto BufferImpl = Buffer->getHandle< RsBufferImplGL* >();

	if( !BufferInMainMemory )
	{
		if( BufferImpl->Handle_ != 0 )
		{
			GL( DeleteBuffers( 1, &BufferImpl->Handle_ ) );
			
			RetVal = true;
		}
	}
	else
	{
		// Buffer is in main memory.
		delete [] BufferImpl->BufferData_;
		BufferImpl->BufferData_ = nullptr;
		RetVal = true;
	}

	delete BufferImpl;
	Buffer->setHandle< int >( 0 );

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsContextGL::updateBuffer( 
	RsBuffer* Buffer,
	BcSize Offset,
	BcSize Size,
	RsResourceUpdateFlags Flags,
	RsBufferUpdateFunc UpdateFunc )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::updateBuffer" );
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	// Validate size.
	const auto& BufferDesc = Buffer->getDesc();
	BcAssertMsg( ( Offset + Size ) <= BufferDesc.SizeBytes_, "Typing to update buffer outside of range." );
	BcAssertMsg( BufferDesc.Type_ != RsBufferType::UNKNOWN, "Buffer type is unknown" );

	auto BufferImpl = Buffer->getHandle< RsBufferImplGL* >();
	BcAssert( BufferImpl );

	// Is buffer be in main memory?
	BcBool BufferInMainMemory =
		Version_.SupportUniformBuffers_ == BcFalse &&
		BufferDesc.Type_ == RsBufferType::UNIFORM;

	if( !BufferInMainMemory )
	{
		if( BufferImpl->Handle_ != 0 )
		{
			// Get buffer type for GL.
			auto TypeGL = gBufferType[ (BcU32)BufferDesc.Type_ ];

			// Bind buffer.
			GL( BindBuffer( TypeGL, BufferImpl->Handle_ ) );

			// NOTE: The map range path should work correctly.
			//       The else is a very heavy handed way to force orphaning
			//       so we don't need to mess around with too much
			//       synchronisation. 
			//       NOTE: This is just a bug with the nouveau drivers.
			//		 TODO: Test this on other drivers.
#if 0 && !defined( RENDER_USE_GLES )
			// Get access flags for GL.
			GLbitfield AccessFlagsGL =
				GL_MAP_WRITE_BIT |
				GL_MAP_INVALIDATE_RANGE_BIT;

			// Map and update buffer.
			auto LockedPointer = GL( MapBufferRange( TypeGL, Offset, Size, AccessFlagsGL ) );
			if( LockedPointer != nullptr )
			{
				RsBufferLock Lock = 
				{
					LockedPointer
				};
				UpdateFunc( Buffer, Lock );
				GL( UnmapBuffer( TypeGL ) );
			}

			
#else
			// Get usage flags for GL.
			GLuint UsageFlagsGL = 0;

			// Data update frequencies.
			if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STATIC ) != RsResourceCreationFlags::NONE )
			{
				UsageFlagsGL |= GL_STATIC_DRAW;
			}
			else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
			{
				UsageFlagsGL |= GL_DYNAMIC_DRAW;
			}
			else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
			{
				UsageFlagsGL |= GL_STREAM_DRAW;
			}

			// Perform orphaning.
			if( Offset == 0 && Size == BufferDesc.SizeBytes_ )
			{
				GL( BufferData( TypeGL, 0, nullptr, UsageFlagsGL ) );
			}

			// Use glBufferSubData to upload.
			// TODO: Allocate of a temporary per-frame buffer.
			std::unique_ptr< BcChar[] > Data( new BcChar[ Size ] );
				RsBufferLock Lock =
			{
				Data.get()
			};
			UpdateFunc( Buffer, Lock );

			if( Size == BufferDesc.SizeBytes_ )
			{
				GL( BufferData( TypeGL, Size, Data.get(), UsageFlagsGL ) );
			}
			else
			{
				GL( BufferSubData( TypeGL, Offset, Size, Data.get() ) );
			}
#endif
			// Increment version.
			BufferImpl->Version_++;
			return true;
		}
	}
	else
	{
		// Dirty program.
		// TODO: Optimise later.
		ProgramDirty_ = BcTrue;
		BcAssert( BufferImpl->BufferData_ );

		// Buffer is in main memory.
		RsBufferLock Lock =
		{
			BufferImpl->BufferData_ + Offset
		};
		UpdateFunc( Buffer, Lock );

		// Increment version.
		BufferImpl->Version_++;
		return true;
	}

	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextGL::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& TextureDesc = Texture->getDesc();

	// Get buffer type for GL.
	auto TypeGL = gTextureType[ (BcU32)TextureDesc.Type_ ];

	// Get usage flags for GL.
	GLuint UsageFlagsGL = 0;
	
	// Data update frequencies.
	if( ( TextureDesc.CreationFlags_ & RsResourceCreationFlags::STATIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STATIC_DRAW;
	}
	else if( ( TextureDesc.CreationFlags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_DYNAMIC_DRAW;
	}
	else if( ( TextureDesc.CreationFlags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STREAM_DRAW;
	}

	// Check if format is a depth one.
	if( IsDepthFormat( TextureDesc.Format_ ) )
	{
		if( !Version_.SupportDepthTextures_ )
		{
			PSY_LOG( "ERROR: No depth texture support." );
			return false;
		}
	}

	// Create GL texture.
	RsTextureImplGL* TextureImpl = new RsTextureImplGL();
	Texture->setHandle( TextureImpl );

	GL( GenTextures( 1, &TextureImpl->Handle_ ) );
	
	

	if( TextureImpl->Handle_ != 0 )
	{
		// Bind texture.
		GL( BindTexture( TypeGL, TextureImpl->Handle_ ) );
		

#if !defined( RENDER_USE_GLES )
		// Set max levels.
		GL( TexParameteri( TypeGL, GL_TEXTURE_MAX_LEVEL, TextureDesc.Levels_ - 1 ) );
		

		// Set compare mode to none.
		if( TextureDesc.Format_ == RsTextureFormat::D16 ||
			TextureDesc.Format_ == RsTextureFormat::D24 ||
			TextureDesc.Format_ == RsTextureFormat::D32 ||
			TextureDesc.Format_ == RsTextureFormat::D24S8 ||
			TextureDesc.Format_ == RsTextureFormat::D32F )
		{
			GL( TexParameteri( TypeGL, GL_TEXTURE_COMPARE_MODE, GL_NONE ) );
			
			GL( TexParameteri( TypeGL, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ) );
			
		}
#endif

		// Instantiate levels.
		BcU32 Width = TextureDesc.Width_;
		BcU32 Height = TextureDesc.Height_;
		BcU32 Depth = TextureDesc.Depth_;
		for( BcU32 LevelIdx = 0; LevelIdx < TextureDesc.Levels_; ++LevelIdx )
		{
			auto TextureSlice = Texture->getSlice( LevelIdx );

			// Load slice.
			loadTexture( Texture, TextureSlice, BcFalse, 0, nullptr );
			// TODO: Error checking on loadTexture.

			// Down a power of two.
			Width = BcMax( 1, Width >> 1 );
			Height = BcMax( 1, Height >> 1 );
			Depth = BcMax( 1, Depth >> 1 );
		}

		++NoofTextures_;

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextGL::destroyTexture( 
		class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Check that we haven't already freed it.
	RsTextureImplGL* TextureImpl = Texture->getHandle< RsTextureImplGL* >();
	if( TextureImpl != nullptr )
	{
		// Delete it.
		GL( DeleteTextures( 1, &TextureImpl->Handle_ ) );
		

		setHandle< int >( 0 );
		delete TextureImpl;

		--NoofTextures_;
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
bool RsContextGL::updateTexture( 
	class RsTexture* Texture,
	const struct RsTextureSlice& Slice,
	RsResourceUpdateFlags Flags,
	RsTextureUpdateFunc UpdateFunc )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RsTextureImplGL* TextureImpl = Texture->getHandle< RsTextureImplGL* >();

	const auto& TextureDesc = Texture->getDesc();

	if( TextureImpl->Handle_ != 0 )
	{
		// Allocate a temporary buffer.
		// TODO: Use PBOs for this part.
		BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
		BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
		BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );
		BcU32 DataSize = RsTextureFormatSize( 
			TextureDesc.Format_,
			Width,
			Height,
			Depth,
			1 );
		std::vector< BcU8 > Data( DataSize );
		BcU32 SlicePitch = DataSize / Depth;
		BcU32 Pitch = SlicePitch / Height;
		RsTextureLock Lock = 
		{
			&Data[ 0 ],
			Pitch,
			SlicePitch
		};

		// Call update func.
		UpdateFunc( Texture, Lock );

		// Load slice.
		loadTexture( Texture, Slice, BcTrue, DataSize, &Data[ 0 ] );
		// TODO: Error checking on loadTexture.

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextGL::createShader(
	RsShader* Shader )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& Desc = Shader->getDesc();
	GLuint ShaderType = gShaderType[ (BcU32)Desc.ShaderType_ ];

	// Create handle for shader.
	GLuint Handle = GL( CreateShader( ShaderType ) );
	
	
	//
	const GLchar* ShaderData[] = 
	{
		reinterpret_cast< const GLchar* >( Shader->getData() ),
	};

	// Load the source code into it.
	GL( ShaderSource( Handle, 1, ShaderData, nullptr ) );
	
			
	// Compile the source code.
	GL( CompileShader( Handle ) );
	
			
	// Test if compilation succeeded.
	GLint ShaderCompiled;
	GL( GetShaderiv( Handle, GL_COMPILE_STATUS, &ShaderCompiled ) );
	if ( !ShaderCompiled )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		GL( GetShaderiv( Handle, GL_INFO_LOG_LENGTH, &i32InfoLogLength ) );
				
		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		GL( GetShaderInfoLog( Handle, i32InfoLogLength, &i32CharsWritten, pszInfoLog ) );

		PSY_LOG( "=======================================================\n" );
		PSY_LOG( "Error Compiling shader:\n" );
		PSY_LOG( "RsShaderGL: Infolog:\n", pszInfoLog );
		std::stringstream LogStream( pszInfoLog );
		std::string LogLine;
		while( std::getline( LogStream, LogLine, '\n' ) )
		{
			PSY_LOG( LogLine.c_str() );
		}
		PSY_LOG( "=======================================================\n" );
		std::stringstream ShaderStream( ShaderData[0] );
		std::string ShaderLine;
		int Line = 1;
		while( std::getline( ShaderStream, ShaderLine, '\n' ) )
		{
			PSY_LOG( "%u: %s", Line++, ShaderLine.c_str() );
		}
		PSY_LOG( "=======================================================\n" );
		delete [] pszInfoLog;

		GL( DeleteShader( Handle ) );
		return false;
	}

	++NoofShaders_;
	
	// Destroy if there is a failure.
	GLenum Error = GL( GetError() );
	if ( Error != GL_NO_ERROR )
	{
		PSY_LOG( "RsShaderGL: Error has occured: %u\n", Error );
		GL( DeleteShader( Handle ) );
		return false;
	}

	Shader->setHandle( Handle );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextGL::destroyShader(
	class RsShader* Shader )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	GLuint Handle = Shader->getHandle< GLuint >();
	GL( DeleteShader( Handle ) );

	--NoofShaders_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextGL::createProgram(
	RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Set handle.
	RsProgramImplGL* ProgramImpl = new RsProgramImplGL();
	Program->setHandle( ProgramImpl );

	const auto& Shaders = Program->getShaders();

	// Some checks to ensure validity.
	BcAssert( Shaders.size() > 0 );	

	// Create program.
	ProgramImpl->Handle_ = GL( CreateProgram() );
	

	// Attach shaders.
	for( auto* Shader : Shaders )
	{
		GL( AttachShader( ProgramImpl->Handle_, Shader->getHandle< GLuint >() ) );
		
	}
	
	// Bind all slots up.
	// NOTE: We shouldn't need this in later GL versions with explicit
	//       binding slots.
	BcChar ChannelNameChars[ 128 ] = { 0 };
	for( BcU32 Channel = 0; Channel < 16; ++Channel )
	{
		BcSPrintf( ChannelNameChars, sizeof( ChannelNameChars ) - 1, "dcl_Input%u", Channel );
		GL( BindAttribLocation( ProgramImpl->Handle_, Channel, ChannelNameChars ) );
		
	}
	
	// Link program.
	GL( LinkProgram( ProgramImpl->Handle_ ) );
	

	GLint ProgramLinked = 0;
	GL( GetProgramiv( ProgramImpl->Handle_, GL_LINK_STATUS, &ProgramLinked ) );
	if ( !ProgramLinked )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		GL( GetProgramiv( ProgramImpl->Handle_, GL_INFO_LOG_LENGTH, &i32InfoLogLength ) );

		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		GL( GetProgramInfoLog( ProgramImpl->Handle_, i32InfoLogLength, &i32CharsWritten, pszInfoLog ) );
		PSY_LOG( "RsProgramGL: Infolog:\n %s\n", pszInfoLog );
		delete [] pszInfoLog;

		GL( DeleteProgram( ProgramImpl->Handle_ ) );
		return false;
	}
	
	// Attempt to find uniform names, and uniform buffers for ES2.
	GLint ActiveUniforms = 0;
	GL( GetProgramiv( ProgramImpl->Handle_, GL_ACTIVE_UNIFORMS, &ActiveUniforms ) );
	std::set< std::string > UniformBlockSet;
	BcU32 ActiveSamplerIdx = 0;
	for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniforms; ++Idx )
	{
		// Uniform information.
		GLchar UniformName[ 256 ];
		GLsizei UniformNameLength = 0;
		GLint Size = 0;
		GLenum Type = GL_INVALID_VALUE;

		// Get the uniform.
		GL( GetActiveUniform( ProgramImpl->Handle_, Idx, sizeof( UniformName ), &UniformNameLength, &Size, &Type, UniformName ) );
		
		// Add it as a parameter.
		if( UniformNameLength > 0 && Type != GL_INVALID_VALUE )
		{
			GLint UniformLocation = GL( GetUniformLocation( ProgramImpl->Handle_, UniformName ) );

			// Trim index off.
			BcChar* pIndexStart = BcStrStr( UniformName, "[0]" );
			if( pIndexStart != NULL )
			{
				*pIndexStart = '\0';
			}

			RsShaderParameterType InternalType = RsShaderParameterType::INVALID;
			switch( Type )
			{
#if !defined( RENDER_USE_GLES )
			case GL_SAMPLER_1D:
				InternalType = RsShaderParameterType::SAMPLER_1D;
				break;
#endif
			case GL_SAMPLER_2D:
				InternalType = RsShaderParameterType::SAMPLER_2D;
				break;
			case GL_SAMPLER_3D:
				InternalType = RsShaderParameterType::SAMPLER_3D;
				break;
			case GL_SAMPLER_CUBE:
				InternalType = RsShaderParameterType::SAMPLER_CUBE;
				break;
#if !defined( RENDER_USE_GLES )
			case GL_SAMPLER_1D_SHADOW:
				InternalType = RsShaderParameterType::SAMPLER_1D_SHADOW;
				break;
#endif
			case GL_SAMPLER_2D_SHADOW:
				InternalType = RsShaderParameterType::SAMPLER_2D_SHADOW;
				break;
			default:
				InternalType = RsShaderParameterType::INVALID;
				break;
			}

			if( InternalType != RsShaderParameterType::INVALID )
			{
				// Add sampler. Will fail if not supported sampler type.
				Program->addSamplerSlot( UniformName, ActiveSamplerIdx );
				Program->addTextureSlot( UniformName, ActiveSamplerIdx );

				// Bind sampler to known index.
				GL( UseProgram( ProgramImpl->Handle_ ) );
				GL( Uniform1i( UniformLocation, ActiveSamplerIdx ) );
				GL( UseProgram( 0 ) );
				++ActiveSamplerIdx;
				
			}
			else
			{
				if( Version_.SupportUniformBuffers_ == BcFalse )
				{
					// Could be a member of a struct where we don't have uniform buffers.
					// Check the name and work out if it is. If so, add to a map so we can add all afterwards.
					auto VSTypePtr = BcStrStr( UniformName, "VS." ); 
					auto PSTypePtr = BcStrStr( UniformName, "PS." );
					if( VSTypePtr != nullptr ||
						PSTypePtr != nullptr )
					{
						// Terminate.
						if( VSTypePtr != nullptr )
						{
							VSTypePtr[ 0 ] = '\0';
						}
						else if( PSTypePtr != nullptr )
						{
							PSTypePtr[ 0 ] = '\0';
						}

						// Add to set.
						UniformBlockSet.insert( UniformName );
					}
				}
			}
		}
	}
	
	// Attempt to find uniform block names.
	if( Version_.SupportUniformBuffers_ )
	{
		GLint ActiveUniformBlocks = 0;
		GL( GetProgramiv( ProgramImpl->Handle_, GL_ACTIVE_UNIFORM_BLOCKS, &ActiveUniformBlocks ) );
	
#if !defined( RENDER_USE_GLES )
		BcU32 ActiveUniformSlotIndex = 0;
		for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniformBlocks; ++Idx )
		{
			// Uniform information.
			GLchar UniformBlockName[ 256 ];
			GLsizei UniformBlockNameLength = 0;
			GLint Size = 0;

			// Get the uniform block size.
			GL( GetActiveUniformBlockiv( ProgramImpl->Handle_, Idx, GL_UNIFORM_BLOCK_DATA_SIZE, &Size ) );
			GL( GetActiveUniformBlockName( ProgramImpl->Handle_, Idx, sizeof( UniformBlockName ), &UniformBlockNameLength, UniformBlockName ) );

			// Add it as a parameter.
			if( UniformBlockNameLength > 0 )
			{
				auto TestIdx = GL( GetUniformBlockIndex( ProgramImpl->Handle_, UniformBlockName ) );
				BcAssert( TestIdx == Idx );
				BcUnusedVar( TestIdx );

				auto Class = ReManager::GetClass( UniformBlockName );
				BcAssert( Class->getSize() == (size_t)Size );
				Program->addUniformBufferSlot( 
					UniformBlockName, 
					Idx, 
					Class );

				GL( UniformBlockBinding( ProgramImpl->Handle_, Idx, ActiveUniformSlotIndex++ ) );
				
			}
		}
#endif // !defined( RENDER_USE_GLES )
	}
	else
	{
		// Base uniform entry.
		RsProgramImplGL::UniformEntry UniformEntry;
		BcU32 UniformHandle = 0;
		for( auto UniformBlockName : UniformBlockSet )
		{
			const ReClass* Class = ReManager::GetClass( UniformBlockName );
			Program->addUniformBufferSlot( 
				UniformBlockName, 
				UniformHandle,
				Class );
			
			if( Class != nullptr )
			{
				// Statically cache the types.
				static auto TypeU32 = ReManager::GetClass( "BcU32" );
				static auto TypeS32 = ReManager::GetClass( "BcS32" );
				static auto TypeF32 = ReManager::GetClass( "BcF32" );
				static auto TypeVec2 = ReManager::GetClass( "MaVec2d" );
				static auto TypeVec3 = ReManager::GetClass( "MaVec3d" );
				static auto TypeVec4 = ReManager::GetClass( "MaVec4d" );
				static auto TypeMat4 = ReManager::GetClass( "MaMat4d" );			
				static auto TypeColour = ReManager::GetClass( "RsColour" );			

				// Iterate over all elements and grab the uniforms.
				auto ClassName = *Class->getName();
				auto ClassNameVS = ClassName + "VS";
				for( auto Field : Class->getFields() )
				{
					auto FieldName = *Field->getName();
					auto ValueType = Field->getType();
					auto UniformNameVS = ClassNameVS + ".X" + FieldName;

					UniformEntry.BindingPoint_ = UniformHandle;
					UniformEntry.Count_ = static_cast< GLsizei >( Field->getSize() / ValueType->getSize() );
					UniformEntry.Offset_ = Field->getOffset();

					auto UniformLocationVS = GL( GetUniformLocation( ProgramImpl->Handle_, UniformNameVS.c_str() ) );
					
					if( ValueType == TypeU32 || ValueType == TypeS32 )
					{
						UniformEntry.Type_ = RsProgramImplGL::UniformEntry::Type::UNIFORM_1IV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( BcU32 ) * UniformEntry.Count_;
							ProgramImpl->UniformEntries_.push_back( UniformEntry );
						}						
					}
					else if( ValueType == TypeF32 )
					{
						UniformEntry.Type_ = RsProgramImplGL::UniformEntry::Type::UNIFORM_1FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * UniformEntry.Count_;
							ProgramImpl->UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeVec2 )
					{
						UniformEntry.Type_ = RsProgramImplGL::UniformEntry::Type::UNIFORM_2FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 2 * UniformEntry.Count_;
							ProgramImpl->UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeVec3 )
					{
						UniformEntry.Type_ = RsProgramImplGL::UniformEntry::Type::UNIFORM_3FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 3 * UniformEntry.Count_;
							ProgramImpl->UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeVec4 )
					{
						UniformEntry.Type_ = RsProgramImplGL::UniformEntry::Type::UNIFORM_4FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 4 * UniformEntry.Count_;
							ProgramImpl->UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeColour )
					{
						UniformEntry.Type_ = RsProgramImplGL::UniformEntry::Type::UNIFORM_4FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( float ) * 4 * UniformEntry.Count_;
							ProgramImpl->UniformEntries_.push_back( UniformEntry );
						}
					}
					else if( ValueType == TypeMat4 )
					{
						UniformEntry.Type_ = RsProgramImplGL::UniformEntry::Type::UNIFORM_MATRIX_4FV;

						if( UniformLocationVS != -1 )
						{
							UniformEntry.Loc_ = UniformLocationVS;
							UniformEntry.Size_ = sizeof( MaMat4d ) * UniformEntry.Count_;
							ProgramImpl->UniformEntries_.push_back( UniformEntry );
						}
					}

					UniformEntry.CachedOffset_ += UniformEntry.Size_;
				}
			}

			++UniformHandle;
		}

		// Allocate a buffer to cache uniform values in.
		ProgramImpl->CachedUniforms_.reset( new BcU8[ UniformEntry.CachedOffset_ ] );
		BcMemSet( ProgramImpl->CachedUniforms_.get(), 0xff, UniformEntry.CachedOffset_ );
	}

	// Catch error.
	

	// Validate program.
	GL( ValidateProgram( ProgramImpl->Handle_ ) );
	GLint ProgramValidated = 0;
	GL( GetProgramiv( ProgramImpl->Handle_, GL_VALIDATE_STATUS, &ProgramValidated ) );
	if ( !ProgramLinked )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		GL( GetProgramiv( ProgramImpl->Handle_, GL_INFO_LOG_LENGTH, &i32InfoLogLength ) );

		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		GL( GetProgramInfoLog( ProgramImpl->Handle_, i32InfoLogLength, &i32CharsWritten, pszInfoLog ) );
		PSY_LOG( "RsProgramGL: Infolog:\n %s\n", pszInfoLog );
		delete [] pszInfoLog;

		GL( DeleteProgram( ProgramImpl->Handle_ ) );
		delete ProgramImpl;
		Program->setHandle( 0 );
		return false;
	}

	++NoofPrograms_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextGL::destroyProgram(
	class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RsProgramImplGL* ProgramImpl = Program->getHandle< RsProgramImplGL* >();
	GL( DeleteProgram( ProgramImpl->Handle_ ) );
	delete ProgramImpl;
	Program->setHandle( 0 );

	--NoofPrograms_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
bool RsContextGL::createVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyVertexDeclaration
bool RsContextGL::destroyVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration  )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextGL::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	for( BcU32 Sampler = 0; Sampler < MAX_TEXTURE_SLOTS; ++Sampler )
	{
		setTexture( Sampler, nullptr, BcTrue );
	}

	flushState();
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextGL::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextGL::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	NoofTextureStateBinds_ = 0;
	for( BcU32 Idx = 0; Idx < MAX_TEXTURE_SLOTS; ++Idx )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Idx ];
		
		TextureStateValue.Dirty_ = BcTrue;
		
		BcAssert( NoofTextureStateBinds_ < MAX_TEXTURE_SLOTS );
		TextureStateBinds_[ NoofTextureStateBinds_++ ] = Idx;
	}
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextGL::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RenderState_ = RenderState;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextGL::setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( Slot < MAX_TEXTURE_SLOTS )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Slot ];
		
		const BcBool WasDirty = TextureStateValue.Dirty_;
		
		TextureStateValue.Dirty_ |= ( TextureStateValue.pSamplerState_ != SamplerState );
		TextureStateValue.pSamplerState_ = SamplerState;
	
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && TextureStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofTextureStateBinds_ < MAX_TEXTURE_SLOTS );
			TextureStateBinds_[ NoofTextureStateBinds_++ ] = Slot;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextGL::setTexture( BcU32 Sampler, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( Sampler < MAX_TEXTURE_SLOTS )
	{
		if( pTexture != nullptr )
		{
			BcAssertMsg( ( pTexture->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE,
				"Texture can't be bound as a shader resource. Has it been created with RsResourceBindFlags::SHADER_RESOURCE?" );
		}

		TTextureStateValue& TextureStateValue = TextureStateValues_[ Sampler ];
		
		const BcBool WasDirty = TextureStateValue.Dirty_;
		
		TextureStateValue.Dirty_ |= ( TextureStateValue.pTexture_ != pTexture ) || Force;
		TextureStateValue.pTexture_ = pTexture;
	
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && TextureStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofTextureStateBinds_ < MAX_TEXTURE_SLOTS );
			TextureStateBinds_[ NoofTextureStateBinds_++ ] = Sampler;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextGL::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( Program_ != Program )
	{
		Program_ = Program;
		ProgramDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setPrimitive
void RsContextGL::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcAssertMsg( IndexBuffer->getDesc().Type_ == RsBufferType::INDEX, "Buffer must be index buffer." );

	if( IndexBuffer_ != IndexBuffer )
	{
		IndexBuffer_ = IndexBuffer;
		BindingsDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextGL::setVertexBuffer( 
	BcU32 StreamIdx, 
	class RsBuffer* VertexBuffer,
	BcU32 Stride )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcAssertMsg( VertexBuffer->getDesc().Type_ == RsBufferType::VERTEX, "Buffer must be vertex buffer." );

	if( VertexBuffers_[ StreamIdx ].Buffer_ != VertexBuffer ||
		VertexBuffers_[ StreamIdx ].Stride_ != Stride )
	{
		VertexBuffers_[ StreamIdx ].Buffer_ = VertexBuffer;
		VertexBuffers_[ StreamIdx ].Stride_ = Stride;
		BindingsDirty_ = BcTrue;
		ProgramDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextGL::setUniformBuffer( 
	BcU32 SlotIdx, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcAssertMsg( UniformBuffer->getDesc().Type_ == RsBufferType::UNIFORM, "Buffer must be uniform buffer." );

	if( UniformBuffers_[ SlotIdx ].Buffer_ != UniformBuffer )
	{
		UniformBuffers_[ SlotIdx ].Buffer_ = UniformBuffer;
		BindingsDirty_ = BcTrue;
		ProgramDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextGL::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( FrameBuffer_ != FrameBuffer )
	{
		DirtyFrameBuffer_ = BcTrue;
		FrameBuffer_ = FrameBuffer;

		BcU32 Width = getWidth();
		BcU32 Height = getHeight();
		if( FrameBuffer )
		{
			const auto& FBDesc = FrameBuffer->getDesc();
			const auto& TexDesc = FBDesc.RenderTargets_[ 0 ]->getDesc();
			Width = TexDesc.Width_;
			Height = TexDesc.Height_;
		}
		auto Viewport = RsViewport( 0, 0, Width, Height );
		setViewport( Viewport );
	}
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextGL::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( VertexDeclaration_ != VertexDeclaration )
	{
		VertexDeclaration_ = VertexDeclaration;
		BindingsDirty_ = BcTrue;
		ProgramDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextGL::flushState()
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::flushState" );
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	
	
	// Bind render states.
	// TODO: Check for redundant state.
	if( RenderState_ != nullptr )
	{
		const auto& Desc = RenderState_->getDesc();
		setRenderStateDesc( Desc, BcFalse );
	}	

	// Bind texture states.
	for( BcU32 TextureStateIdx = 0; TextureStateIdx < NoofTextureStateBinds_; ++TextureStateIdx )
	{
		BcU32 TextureStateID = TextureStateBinds_[ TextureStateIdx ];
		TTextureStateValue& TextureStateValue = TextureStateValues_[ TextureStateID ];

		if( TextureStateValue.Dirty_ )
		{
			RsTexture* pTexture = TextureStateValue.pTexture_;			
			const RsSamplerState* SamplerState = TextureStateValue.pSamplerState_;
			const RsTextureType InternalType = pTexture ? pTexture->getDesc().Type_ : RsTextureType::TEX2D;
			const GLenum TextureType = gTextureTypes[ (BcU32)InternalType ];

			GL( ActiveTexture( GL_TEXTURE0 + TextureStateID ) );
			if( pTexture != nullptr )
			{
				RsTextureImplGL* TextureImpl = pTexture->getHandle< RsTextureImplGL* >();
				BcAssert( TextureImpl != nullptr );
				GL( BindTexture( TextureType, TextureImpl->Handle_ ) );
			}
			else
			{
				GL( BindTexture( TextureType, 0 ) );
			}
			

			if( pTexture != nullptr && SamplerState != nullptr )
			{
#if !defined( RENDER_USE_GLES )
				if( Version_.SupportSamplerStates_ )
				{
					GLuint SamplerObject = SamplerState->getHandle< GLuint >();
					GL( BindSampler( TextureStateIdx, SamplerObject ) );
				}
				else
#endif
				{
					// TODO MipLODBias_
					// TODO MaxAnisotropy_
					// TODO BorderColour_
					// TODO MinLOD_
					// TODO MaxLOD_
					const auto& SamplerStateDesc = SamplerState->getDesc();
					GL( TexParameteri( TextureType, GL_TEXTURE_MIN_FILTER, gTextureFiltering[ (BcU32)SamplerStateDesc.MinFilter_ ] ) );
					GL( TexParameteri( TextureType, GL_TEXTURE_MAG_FILTER, gTextureFiltering[ (BcU32)SamplerStateDesc.MagFilter_ ] ) );
					GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_S, gTextureSampling[ (BcU32)SamplerStateDesc.AddressU_ ] ) );
					GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_T, gTextureSampling[ (BcU32)SamplerStateDesc.AddressV_ ] ) );	
#if !defined( RENDER_USE_GLES )
					GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_R, gTextureSampling[ (BcU32)SamplerStateDesc.AddressW_ ] ) );	
					GL( TexParameteri( TextureType, GL_TEXTURE_COMPARE_MODE, GL_NONE ) );
					GL( TexParameteri( TextureType, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ) );
#endif
					
				}
			}

			TextureStateValue.Dirty_ = BcFalse;
		}
	}
	
	// Reset binds.
	NoofTextureStateBinds_ = 0;

	

	// Bind program and primitive.
	if( ( Program_ != nullptr &&
		  VertexDeclaration_ != nullptr ) &&
		( ProgramDirty_ || BindingsDirty_ ) )
	{
		const auto& ProgramVertexAttributeList = Program_->getVertexAttributeList();
		const auto& VertexDeclarationDesc = VertexDeclaration_->getDesc();
		const auto& PrimitiveVertexElementList = VertexDeclarationDesc.Elements_;

		// Bind program
		RsProgramImplGL* ProgramImpl = Program_->getHandle< RsProgramImplGL* >();

		GL( UseProgram( ProgramImpl->Handle_ ) );
		

		// Bind up uniform buffers, or uniforms.
#if !defined( RENDER_USE_GLES )	
		if( Version_.SupportUniformBuffers_ )
		{
			BcU32 BindingPoint = 0;
			for( auto It( UniformBuffers_.begin() ); It != UniformBuffers_.end(); ++It )
			{
				auto Buffer = (*It).Buffer_;
				if( Buffer != nullptr )
				{
					auto BufferImpl = Buffer->getHandle< RsBufferImplGL* >();
					BcAssert( BufferImpl );
					GL( BindBufferRange( GL_UNIFORM_BUFFER, BindingPoint, BufferImpl->Handle_, 0, Buffer->getDesc().SizeBytes_ ) );
					
					++BindingPoint;
				}
			}
		}
		else
#endif
		{
			for( auto& UniformEntry : ProgramImpl->UniformEntries_ )
			{
				const BcU32 BindingPoint = UniformEntry.BindingPoint_;
				auto Buffer = UniformBuffers_[ BindingPoint ].Buffer_;
				if( Buffer != nullptr )
				{
					const auto BufferImpl = Buffer->getHandle< RsBufferImplGL* >();
					BcAssert( BufferImpl );

					// Check version, if equal, then don't update uniform.
					if( UniformEntry.Buffer_ != Buffer ||
						UniformEntry.Version_ != BufferImpl->Version_ )
					{
						// Update buffer & version.
						UniformEntry.Buffer_ = Buffer;
						UniformEntry.Version_ = BufferImpl->Version_;

						// Setup uniforms.
						const auto* BufferData = BufferImpl->BufferData_;
						BcAssert( BufferData );
						const auto* UniformData = BufferData + UniformEntry.Offset_;
						auto* CachedUniformData = ProgramImpl->CachedUniforms_.get() + UniformEntry.CachedOffset_;

						// Check if value has changed.
						if( memcmp( CachedUniformData, UniformData, UniformEntry.Size_ ) != 0 )
						{
							memcpy( CachedUniformData, UniformData, UniformEntry.Size_ );
							switch( UniformEntry.Type_ )
							{
							case RsProgramImplGL::UniformEntry::Type::UNIFORM_1IV:
								GL( Uniform1iv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcS32* >( UniformData ) ) );
								break;
							case RsProgramImplGL::UniformEntry::Type::UNIFORM_1FV:
								GL( Uniform1fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
								break;
							case RsProgramImplGL::UniformEntry::Type::UNIFORM_2FV:
								GL( Uniform2fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
								break;
							case RsProgramImplGL::UniformEntry::Type::UNIFORM_3FV:
								GL( Uniform3fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
								break;
							case RsProgramImplGL::UniformEntry::Type::UNIFORM_4FV:
								GL( Uniform4fv( UniformEntry.Loc_, UniformEntry.Count_, reinterpret_cast< const BcF32* >( UniformData ) ) );
								break;
							case RsProgramImplGL::UniformEntry::Type::UNIFORM_MATRIX_4FV:
								GL( UniformMatrix4fv( UniformEntry.Loc_, UniformEntry.Count_, GL_FALSE, reinterpret_cast< const BcF32* >( UniformData ) ) );
								break;
							default:
								BcBreakpoint;
								break;
							}
							
						}
					}
				}
				else
				{
					BcBreakpoint;
				}
			}
		}


		// Cached vertex handle for binding.
		GLuint BoundVertexHandle = 0;

		// Brute force disable vertex arrays.
		for( BcU32 Idx = 0; Idx < 16; ++Idx )
		{
			GL( DisableVertexAttribArray( Idx ) );
		}

		// Bind up all elements to attributes.
		BcU32 BoundElements = 0;
		for( const auto& Attribute : ProgramVertexAttributeList )
		{
			auto FoundElement = std::find_if( PrimitiveVertexElementList.begin(), PrimitiveVertexElementList.end(),
				[ & ]( const RsVertexElement& Element )
				{
					return ( Element.Usage_ == Attribute.Usage_ &&
						Element.UsageIdx_ == Attribute.UsageIdx_ );
				} );

			// Force to an element with zero offset if we can't find a valid one.
			// TODO: Find a better approach.
			if( FoundElement == PrimitiveVertexElementList.end() )
			{
				FoundElement = std::find_if( PrimitiveVertexElementList.begin(), PrimitiveVertexElementList.end(),
					[ & ]( const RsVertexElement& Element )
					{
						return Element.Offset_ == 0;
					} );
			}

			// Found an element we can bind to.
			if( FoundElement != PrimitiveVertexElementList.end() )
			{
				auto VertexBufferBinding = VertexBuffers_[ FoundElement->StreamIdx_ ];
				auto VertexBuffer = VertexBufferBinding.Buffer_;
				auto VertexStride = VertexBufferBinding.Stride_;
			
				// Bind up new vertex buffer if we need to.
				BcAssertMsg( FoundElement->StreamIdx_ < VertexBuffers_.size(), "Stream index out of bounds for primitive." );
				BcAssertMsg( VertexBuffer != nullptr, "Vertex buffer not bound!" );
				auto VertexBufferImpl = VertexBuffer->getHandle< RsBufferImplGL* >();
				BcAssert( VertexBufferImpl );
				GLuint VertexHandle = VertexBufferImpl->Handle_;
				if( BoundVertexHandle != VertexHandle )
				{
					GL( BindBuffer( GL_ARRAY_BUFFER, VertexHandle ) );
					BoundVertexHandle = VertexHandle;
				}

				// Enable array.
				GL( EnableVertexAttribArray( Attribute.Channel_ ) );

				// Bind.
				BcU64 CalcOffset = FoundElement->Offset_;

				glVertexAttribPointer( Attribute.Channel_, 
					FoundElement->Components_,
					gVertexDataTypes[ (BcU32)FoundElement->DataType_ ],
					gVertexDataNormalised[ (BcU32)FoundElement->DataType_ ],
					VertexStride,
					(GLvoid*)CalcOffset );

				++BoundElements;
			}
		}

		BcAssert( ProgramVertexAttributeList.size() == BoundElements );

		// Bind indices.
		auto IndexBufferImpl = IndexBuffer_ ? IndexBuffer_->getHandle< RsBufferImplGL* >() : nullptr;
		GLuint IndicesHandle = IndexBufferImpl != nullptr ? IndexBufferImpl->Handle_ : 0;
		GL( BindBuffer( GL_ELEMENT_ARRAY_BUFFER, IndicesHandle ) );
		ProgramDirty_ = BcFalse;
		BindingsDirty_ = BcFalse;
	}

	// TODO: Redundant state.
	if( DirtyFrameBuffer_ )
	{
		if( FrameBuffer_ != nullptr )
		{
			GL( BindFramebuffer( GL_FRAMEBUFFER, FrameBuffer_->getHandle< GLuint >() ) );
		}
		else
		{
			GL( BindFramebuffer( GL_FRAMEBUFFER, 0 ) );
		}
		DirtyFrameBuffer_ = BcFalse;
	}

	if( DirtyViewport_ )
	{
		GL( Viewport( Viewport_.x(), Viewport_.y(), Viewport_.width(), Viewport_.height() ) );
		DirtyViewport_ = BcFalse;
	}
	if( DirtyScissor_ )
	{
		GL( Scissor( ScissorX_, ScissorY_, ScissorW_, ScissorH_ ) );
		DirtyScissor_ = BcFalse;
	}

	
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextGL::clear( 
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::clear" );
	flushState();
	GL( ClearColor( Colour.r(), Colour.g(), Colour.b(), Colour.a() ) );
	
	// Disable scissor if we need to.
	auto& BoundRasteriserState = BoundRenderStateDesc_.RasteriserState_;
	if( BoundRasteriserState.ScissorEnable_ )
	{
		GL( Disable( GL_SCISSOR_TEST ) );
		BoundRasteriserState.ScissorEnable_ = BcFalse;
	}

	// TODO: Look into this? It causes an invalid operation.
	if( Version_.Type_ != RsOpenGLType::ES )
	{
		GL( ClearDepthf( 1.0f ) );
	}

	GL( ClearStencil( 0 ) );
	auto& BoundDepthStencilState = BoundRenderStateDesc_.DepthStencilState_;
	if( !BoundDepthStencilState.DepthWriteEnable_ )
	{
		GL( DepthMask( GL_TRUE ) );
		BoundDepthStencilState.DepthWriteEnable_ = BcTrue;
	}
	glClear( 
		( EnableClearColour ? GL_COLOR_BUFFER_BIT : 0 ) | 
		( EnableClearDepth ? GL_DEPTH_BUFFER_BIT : 0 ) | 
		( EnableClearStencil ? GL_STENCIL_BUFFER_BIT : 0 ) );	
	
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextGL::drawPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::drawPrimitives" );
	++NoofDrawCalls_;
	flushState();
	BcAssert( Program_ != nullptr );
	GL( DrawArrays( gTopologyType[ (BcU32)TopologyType ], IndexOffset, NoofIndices ) );

	
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextGL::drawIndexedPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::drawIndexedPrimitives" );
	++NoofDrawCalls_;
	flushState();
	BcAssert( Program_ != nullptr );
	BcAssert( ( IndexOffset * sizeof( BcU16 ) ) + NoofIndices <= IndexBuffer_->getDesc().SizeBytes_ );

	if( VertexOffset == 0 )
	{
		GL( DrawElements( gTopologyType[ (BcU32)TopologyType ], NoofIndices, GL_UNSIGNED_SHORT, (void*)( IndexOffset * sizeof( BcU16 ) ) ) );
	}
#if !defined( RENDER_USE_GLES )
	else if( Version_.SupportDrawElementsBaseVertex_ )
	{
		GL( DrawElementsBaseVertex( gTopologyType[ (BcU32)TopologyType ], NoofIndices, GL_UNSIGNED_SHORT, (void*)( IndexOffset * sizeof( BcU16 ) ), VertexOffset ) );
	}
#endif
	else
	{
		BcBreakpoint;	
	}

	
}

//////////////////////////////////////////////////////////////////////////
// copyFrameBufferRenderTargetToTexture
void RsContextGL::copyFrameBufferRenderTargetToTexture( RsFrameBuffer* FrameBuffer, BcU32 Idx, RsTexture* Texture )
{
	// Grab current width + height.
	auto FBWidth = getWidth();
	auto FBHeight = getHeight();
	if( FrameBuffer_ != nullptr )
	{
		auto RT = FrameBuffer_->getDesc().RenderTargets_[ 0 ];
		BcAssert( RT );
		FBWidth = RT->getDesc().Width_;
		FBHeight = RT->getDesc().Height_;
	}

	BcAssert( FBWidth > 0 );
	BcAssert( FBHeight > 0 );

	// Copying the back buffer.
	if( FrameBuffer == nullptr )
	{
		BcAssert( Idx == 0 );
	}
	else
	{
		BcAssert( FrameBuffer->getDesc().RenderTargets_[ Idx ] != nullptr );
	}

	// Bind framebuffer.
	if( FrameBuffer != nullptr )
	{
		GL( BindFramebuffer( GL_FRAMEBUFFER, FrameBuffer->getHandle< GLint >() ) );
	}
	else
	{
		GL( BindFramebuffer( GL_FRAMEBUFFER, 0 ) );	
	}
	DirtyFrameBuffer_ = BcTrue;

	// Set read buffer
	GL( ReadBuffer( GL_COLOR_ATTACHMENT0 + Idx ) );

	const auto& TextureDesc = Texture->getDesc();
	auto TypeGL = gTextureType[ (BcU32)TextureDesc.Type_ ];
	const auto& FormatGL = gTextureFormats[ (BcU32)TextureDesc.Format_ ];

	RsTextureImplGL* DestTextureImpl = Texture->getHandle< RsTextureImplGL* >();
	if( DestTextureImpl->Handle_ != 0 )
	{
		// Bind texture.
		GL( BindTexture( TypeGL, DestTextureImpl->Handle_ ) );
		glCopyTexImage2D( 
			TypeGL, 
			0, 
			FormatGL.InternalFormat_,
			0,
			0,
			TextureDesc.Width_,
			TextureDesc.Height_,
			0 );
		
		GL( BindTexture( TypeGL, 0 ) );
	}
	else
	{
		BcBreakpoint;
	}
}

//////////////////////////////////////////////////////////////////////////
// copyTextureToFrameBufferRenderTarget
void RsContextGL::copyTextureToFrameBufferRenderTarget( RsTexture* Texture, RsFrameBuffer* FrameBuffer, BcU32 Idx )
{
	// Grab current width + height.
	auto FBWidth = getWidth();
	auto FBHeight = getHeight();
	if( FrameBuffer_ != nullptr )
	{
		auto RT = FrameBuffer_->getDesc().RenderTargets_[ 0 ];
		BcAssert( RT );
		FBWidth = RT->getDesc().Width_;
		FBHeight = RT->getDesc().Height_;
	}

	BcAssert( FBWidth > 0 );
	BcAssert( FBHeight > 0 );

	// Copying the back buffer.
	if( FrameBuffer == nullptr )
	{
		BcAssert( Idx == 0 );
	}
	else
	{
		BcAssert( FrameBuffer->getDesc().RenderTargets_[ Idx ] != nullptr );
	}

	const auto& TextureDesc = Texture->getDesc();
	auto TypeGL = gTextureType[ (BcU32)TextureDesc.Type_ ];
	const auto& FormatGL = gTextureFormats[ (BcU32)TextureDesc.Format_ ];

	DirtyFrameBuffer_ = BcTrue;
	RsTextureImplGL* SrcTextureImpl = Texture->getHandle< RsTextureImplGL* >();

	GL( BindFramebuffer( GL_READ_FRAMEBUFFER, TransferFBOs_[ 0 ] ) );
	glFramebufferTexture2D( 
		GL_READ_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		SrcTextureImpl->Handle_,
		0 );
	auto ReadStatus = GL( CheckFramebufferStatus( GL_READ_FRAMEBUFFER ) );
	BcAssertMsg( ReadStatus == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete" );

	if( FrameBuffer != nullptr )
	{
		RsTextureImplGL* DestTextureImpl = 
			FrameBuffer->getDesc().RenderTargets_[ Idx ]->getHandle< RsTextureImplGL* >();
		GL( BindFramebuffer( GL_DRAW_FRAMEBUFFER, TransferFBOs_[ 1 ] ) );
		glFramebufferTexture2D( 
			GL_DRAW_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			DestTextureImpl->Handle_,
			0 );
		auto DrawStatus = GL( CheckFramebufferStatus( GL_DRAW_FRAMEBUFFER ) );
		BcAssertMsg( DrawStatus == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete" );
	}
	else
	{
		GL( BindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
	}

	auto Width = TextureDesc.Width_;
	auto Height = TextureDesc.Height_;

	GL( BlitFramebuffer( 
		0, 0, Width, Height,
		0, 0, Width, Height,
		GL_COLOR_BUFFER_BIT, GL_NEAREST ) );
	

	GL( BindFramebuffer( GL_READ_FRAMEBUFFER, 0 ) );
	GL( BindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 ) );
	
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextGL::setViewport( const class RsViewport& Viewport )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcAssert( Viewport.width() > 0 );
	BcAssert( Viewport.height() > 0 );

	auto FBWidth = getWidth();
	auto FBHeight = getHeight();
	if( FrameBuffer_ != nullptr )
	{
		auto RT = FrameBuffer_->getDesc().RenderTargets_[ 0 ];
		BcAssert( RT );
		FBWidth = RT->getDesc().Width_;
		FBHeight = RT->getDesc().Height_;
	}

	BcAssert( FBWidth > 0 );
	BcAssert( FBHeight > 0 );

	// Convert to top-left.
	auto X = Viewport.x();
	auto Y = FBHeight - Viewport.height();
	auto W = Viewport.width() - Viewport.x();
	auto H = Viewport.height() - Viewport.y();
	auto NewViewport = RsViewport( X, Y, W, H );

	if( Viewport_.x() != NewViewport.x() ||
		Viewport_.y() != NewViewport.y() ||
		Viewport_.width() != NewViewport.width() ||
		Viewport_.height() != NewViewport.height() )
	{
		DirtyViewport_ = BcTrue;
		DirtyScissor_ = BcTrue;
		Viewport_ = NewViewport;
		ScissorX_ = X;
		ScissorY_ = Y;
		ScissorW_ = W;
		ScissorH_ = H;
	}
}

//////////////////////////////////////////////////////////////////////////
// setScissorRect
void RsContextGL::setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height )
{
	auto FBHeight = getHeight();
	if( FrameBuffer_ != nullptr )
	{
		auto RT = FrameBuffer_->getDesc().RenderTargets_[ 0 ];
		BcAssert( RT );
		FBHeight = RT->getDesc().Height_;
	}

	auto SX = X;
	auto SY = FBHeight - Height;
	auto SW = Width - X;
	auto SH = Height - Y;

	if( ScissorX_ != SX ||
		ScissorY_ != SY ||
		ScissorW_ != SW ||
		ScissorH_ != SH )
	{
		DirtyScissor_ = BcTrue;
		ScissorX_ = SX;
		ScissorY_ = SY;
		ScissorW_ = SW;
		ScissorH_ = SH;	
	}
}

//////////////////////////////////////////////////////////////////////////
// getOpenGLVersion
const RsOpenGLVersion& RsContextGL::getOpenGLVersion() const
{
	return Version_;
}

//////////////////////////////////////////////////////////////////////////
// bindStencilFunc
void RsContextGL::bindStencilFunc()
{
	/*
	TRenderStateValue& CompareValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_COMPARE ];
	TRenderStateValue& RefValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_REF ];
	TRenderStateValue& MaskValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_MASK ];

	GL( StencilFunc( gCompareMode[ CompareValue.Value_ ], RefValue.Value_, MaskValue.Value_ ) );

	CompareValue.Dirty_ = BcFalse;
	RefValue.Dirty_ = BcFalse;
	MaskValue.Dirty_ = BcFalse;

	
	*/
}

//////////////////////////////////////////////////////////////////////////
// bindStencilOp
void RsContextGL::bindStencilOp()
{
	/*
	TRenderStateValue& SFailValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_SFAIL ];
	TRenderStateValue& DPFailValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPFAIL ];
	TRenderStateValue& DPPassValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPPASS ];

	GL( StencilFunc( gCompareMode[ CompareValue.Value_ ], RefValue.Value_, MaskValue.Value_ ) );
	GL( StencilOp( gStencilOp[ SFailValue.Value_ ], gStencilOp[ DPFailValue.Value_ ], gStencilOp[ DPPassValue.Value_ ] ) );

	SFailValue.Dirty_ = BcFalse;
	DPFailValue.Dirty_ = BcFalse;
	DPPassValue.Dirty_ = BcFalse;

	
	*/
}

//////////////////////////////////////////////////////////////////////////
// bindBlendMode
void RsContextGL::bindBlendMode( RsBlendingMode BlendMode )
{
	/*
	switch( BlendMode )
	{
		case RsBlendingMode::NONE:
			GL( Disable( GL_BLEND ) );
			GL( BlendFunc( GL_ONE, GL_ZERO ) );
			break;
					
		case RsBlendingMode::BLEND:
			GL( Enable( GL_BLEND ) );
			GL( BlendEquation( GL_FUNC_ADD ) );
			GL( BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) );
			break;
					
		case RsBlendingMode::ADD:
			GL( Enable( GL_BLEND ) );
			GL( BlendEquation( GL_FUNC_ADD ) );
			GL( BlendFunc( GL_SRC_ALPHA, GL_ONE ) );
			break;

		case RsBlendingMode::SUBTRACT:
			GL( Enable( GL_BLEND ) );
			GL( BlendEquation( GL_FUNC_REVERSE_SUBTRACT ) );
			GL( BlendFunc( GL_SRC_ALPHA, GL_ONE ) );
			break;
			
		default:
			BcBreakpoint;
			break;
	}

	
	*/
}

////////////////////////////////////////////////////////////////////////////////
// loadTexture
void RsContextGL::loadTexture(
		RsTexture* Texture, 
		const RsTextureSlice& Slice,
		BcBool Bind, 
		BcU32 DataSize,
		void* Data )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RsTextureImplGL* TextureImpl = Texture->getHandle< RsTextureImplGL* >();

	const auto& TextureDesc = Texture->getDesc();

	// Get buffer type for GL.
	auto TypeGL = gTextureType[ (BcU32)TextureDesc.Type_ ];

	// Bind.
	if( Bind )
	{
		GL( BindTexture( TypeGL, TextureImpl->Handle_ ) );
	}
		
	// Load level.
	BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
	BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
	BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );

	const auto& FormatGL = gTextureFormats[ (BcU32)TextureDesc.Format_ ];

	if( FormatGL.Compressed_ == BcFalse )
	{
		switch( TextureDesc.Type_ )
		{
		case RsTextureType::TEX1D:
#if !defined( RENDER_USE_GLES )
			GL( TexImage1D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data ) );
#else
			// TODO ES2.
			BcBreakpoint;
#endif
			break;

		case RsTextureType::TEX2D:
			GL( TexImage2D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data ) );
			break;

		case RsTextureType::TEX3D:
#if !defined( RENDER_USE_GLES )
			GL( TexImage3D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				Depth,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data ) );
#else
			// TODO ES2.
			BcBreakpoint;
#endif
			break;

		case RsTextureType::TEXCUBE:
			BcBreakpoint;

		default:
			BcBreakpoint;
		}

	}
	else if( Version_.SupportDXTTextures_ )
	{
		// TODO: More intrusive checking of format.
		if( DataSize == 0 || Data == nullptr )
		{
			return;
		}

		switch( TextureDesc.Type_ )
		{
#if !defined( RENDER_USE_GLES )
		case RsTextureType::TEX1D:
			GL( CompressedTexImage1D( 
				TypeGL, 
				Slice.Level_,
				FormatGL.InternalFormat_,
				Width,
				0,
				DataSize,
				Data ) );
			break;
#endif // !defined( RENDER_USE_GLES )

		case RsTextureType::TEX2D:
			GL( CompressedTexImage2D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				0,
				DataSize,
				Data ) );
			break;

#if !defined( RENDER_USE_GLES )
		case RsTextureType::TEX3D:
			GL( CompressedTexImage3D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				Depth,
				0,
				DataSize,
				Data ) );
			break;

		case RsTextureType::TEXCUBE:
			BcBreakpoint;
#endif

		default:
			BcBreakpoint;
		}
	}
	else
	{
		BcBreakpoint;
	}
}

////////////////////////////////////////////////////////////////////////////////
// setRenderStateDesc
void RsContextGL::setRenderStateDesc( const RsRenderStateDesc& Desc, BcBool Force )
{
#if !defined( RENDER_USE_GLES )
	if( Version_.SupportSeparateBlendState_ )
	{
		for( BcU32 Idx = 0; Idx < 8; ++Idx )
		{
			const auto& RenderTarget = Desc.BlendState_.RenderTarget_[ Idx ];
			const auto& BoundRenderTarget = BoundRenderStateDesc_.BlendState_.RenderTarget_[ Idx ];
			
			if( Force || 
				RenderTarget.Enable_ != BoundRenderTarget.Enable_ )
			{
				if( RenderTarget.Enable_ )
				{
					GL( Enablei( GL_BLEND, Idx ) );
				}
				else
				{
					GL( Disablei( GL_BLEND, Idx ) );
				}
			}

			if( Force ||
				RenderTarget.BlendOp_ != BoundRenderTarget.BlendOp_ ||
				RenderTarget.BlendOpAlpha_ != BoundRenderTarget.BlendOpAlpha_ )
			{
				GL( BlendEquationSeparatei( 
					Idx, 
					gBlendOp[ (BcU32)RenderTarget.BlendOp_ ], gBlendOp[ (BcU32)RenderTarget.BlendOpAlpha_ ] ) );
			}

			if( Force ||
				RenderTarget.SrcBlend_ != BoundRenderTarget.SrcBlend_ ||
				RenderTarget.DestBlend_ != BoundRenderTarget.DestBlend_ ||
				RenderTarget.SrcBlendAlpha_ != BoundRenderTarget.SrcBlendAlpha_ ||
				RenderTarget.DestBlendAlpha_ != BoundRenderTarget.DestBlendAlpha_ )
			{
				GL( BlendFuncSeparatei( 
					Idx, 
					gBlendType[ (BcU32)RenderTarget.SrcBlend_ ], gBlendType[ (BcU32)RenderTarget.DestBlend_ ],
					gBlendType[ (BcU32)RenderTarget.SrcBlendAlpha_ ], gBlendType[ (BcU32)RenderTarget.DestBlendAlpha_ ] ) );
			}

			if( Force ||
				RenderTarget.WriteMask_ != BoundRenderTarget.WriteMask_ )
			{
				GL( ColorMaski(
					Idx,
					RenderTarget.WriteMask_ & 1 ? GL_TRUE : GL_FALSE,
					RenderTarget.WriteMask_ & 2 ? GL_TRUE : GL_FALSE,
					RenderTarget.WriteMask_ & 4 ? GL_TRUE : GL_FALSE,
					RenderTarget.WriteMask_ & 8 ? GL_TRUE : GL_FALSE ) );
			}
		}
	}
	else
#endif
	{
		const auto& MainRenderTarget = Desc.BlendState_.RenderTarget_[ 0 ];
		const auto& BoundMainRenderTarget = BoundRenderStateDesc_.BlendState_.RenderTarget_[ 0 ];

		if( Force ||
			MainRenderTarget.Enable_ != BoundMainRenderTarget.Enable_ )
		{
			if( MainRenderTarget.Enable_ )
			{
				GL( Enable( GL_BLEND ) );
			}
			else
			{
				GL( Disable( GL_BLEND ) );
			}
		}

		if( Force ||
			MainRenderTarget.BlendOp_ != BoundMainRenderTarget.BlendOp_ ||
			MainRenderTarget.BlendOpAlpha_ != BoundMainRenderTarget.BlendOpAlpha_ )
		{
			GL( BlendEquationSeparate( 
				gBlendOp[ (BcU32)MainRenderTarget.BlendOp_ ], gBlendOp[ (BcU32)MainRenderTarget.BlendOpAlpha_ ] ) );
		}

		if( Force ||
			MainRenderTarget.SrcBlend_ != BoundMainRenderTarget.SrcBlend_ ||
			MainRenderTarget.DestBlend_ != BoundMainRenderTarget.DestBlend_ ||
			MainRenderTarget.SrcBlendAlpha_ != BoundMainRenderTarget.SrcBlendAlpha_ ||
			MainRenderTarget.DestBlendAlpha_ != BoundMainRenderTarget.DestBlendAlpha_ )
		{
			GL( BlendFuncSeparate( 
				gBlendType[ (BcU32)MainRenderTarget.SrcBlend_ ], gBlendType[ (BcU32)MainRenderTarget.DestBlend_ ],
				gBlendType[ (BcU32)MainRenderTarget.SrcBlendAlpha_ ], gBlendType[ (BcU32)MainRenderTarget.DestBlendAlpha_ ] ) );
		}

		if( Version_.SupportMRT_ )
		{
#if !defined( RENDER_USE_GLES )
			for( BcU32 Idx = 0; Idx < 8; ++Idx )
			{
				const auto& RenderTarget = Desc.BlendState_.RenderTarget_[ Idx ];
				const auto& BoundRenderTarget = BoundRenderStateDesc_.BlendState_.RenderTarget_[ Idx ];

				if( Force ||
					RenderTarget.WriteMask_ != BoundRenderTarget.WriteMask_ )
				{
					GL( ColorMaski(
						Idx,
						RenderTarget.WriteMask_ & 1 ? GL_TRUE : GL_FALSE,
						RenderTarget.WriteMask_ & 2 ? GL_TRUE : GL_FALSE,
						RenderTarget.WriteMask_ & 4 ? GL_TRUE : GL_FALSE,
						RenderTarget.WriteMask_ & 8 ? GL_TRUE : GL_FALSE ) );
				}
			}
#endif // !defined( RENDER_USE_GLES )
		}
		else
		{
			if( Force ||
				MainRenderTarget.WriteMask_ != BoundMainRenderTarget.WriteMask_ )
			{
				GL( ColorMask(
					MainRenderTarget.WriteMask_ & 1 ? GL_TRUE : GL_FALSE,
					MainRenderTarget.WriteMask_ & 2 ? GL_TRUE : GL_FALSE,
					MainRenderTarget.WriteMask_ & 4 ? GL_TRUE : GL_FALSE,
					MainRenderTarget.WriteMask_ & 8 ? GL_TRUE : GL_FALSE ) );
			}
		}
	}

	const auto& DepthStencilState = Desc.DepthStencilState_;
	const auto& BoundDepthStencilState = BoundRenderStateDesc_.DepthStencilState_;
	
	if( Force ||
		DepthStencilState.DepthTestEnable_ != BoundDepthStencilState.DepthTestEnable_ )
	{
		if( DepthStencilState.DepthTestEnable_ )
		{
			GL( Enable( GL_DEPTH_TEST ) );
		}
		else
		{
			GL( Disable( GL_DEPTH_TEST ) );
		}
	}

	if( Force ||
		DepthStencilState.DepthWriteEnable_ != BoundDepthStencilState.DepthWriteEnable_ )
	{
		GL( DepthMask( (GLboolean)DepthStencilState.DepthWriteEnable_ ) );
	}

	if( Force ||
		DepthStencilState.DepthFunc_ != BoundDepthStencilState.DepthFunc_ )
	{
		GL( DepthFunc( gCompareMode[ (BcU32)DepthStencilState.DepthFunc_ ] ) );
	}

	if( Force ||
		DepthStencilState.StencilEnable_ != BoundDepthStencilState.StencilEnable_ )
	{
		if( DepthStencilState.StencilEnable_ )
		{
			GL( Enable( GL_STENCIL_TEST ) );
		}
		else
		{
			GL( Disable( GL_STENCIL_TEST ) );
		}
	}

	if( Force ||
		DepthStencilState.StencilFront_.Func_ != BoundDepthStencilState.StencilFront_.Func_ ||
		DepthStencilState.StencilRef_ != BoundDepthStencilState.StencilRef_ ||
		DepthStencilState.StencilFront_.Mask_ != BoundDepthStencilState.StencilFront_.Mask_ )
	{
		GL( StencilFuncSeparate( 
			GL_FRONT,
			gCompareMode[ (BcU32)DepthStencilState.StencilFront_.Func_ ], 
			DepthStencilState.StencilRef_, DepthStencilState.StencilFront_.Mask_ ) );
	}

	if( Force ||
		DepthStencilState.StencilBack_.Func_ != BoundDepthStencilState.StencilBack_.Func_ ||
		DepthStencilState.StencilRef_ != BoundDepthStencilState.StencilRef_ ||
		DepthStencilState.StencilBack_.Mask_ != BoundDepthStencilState.StencilBack_.Mask_ )
	{
		GL( StencilFuncSeparate( 
			GL_BACK,
			gCompareMode[ (BcU32)DepthStencilState.StencilBack_.Func_ ], 
			DepthStencilState.StencilRef_, DepthStencilState.StencilBack_.Mask_ ) );
	}

	if( Force ||
		DepthStencilState.StencilFront_.Fail_ != BoundDepthStencilState.StencilFront_.Fail_ ||
		DepthStencilState.StencilFront_.DepthFail_ != BoundDepthStencilState.StencilFront_.DepthFail_ ||
		DepthStencilState.StencilFront_.Pass_ != BoundDepthStencilState.StencilFront_.Pass_ )
	{
		GL( StencilOpSeparate( 
			GL_FRONT,
			gStencilOp[ (BcU32)DepthStencilState.StencilFront_.Fail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilFront_.DepthFail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilFront_.Pass_ ] ) );
	}

	if( Force ||
		DepthStencilState.StencilBack_.Fail_ != BoundDepthStencilState.StencilBack_.Fail_ ||
		DepthStencilState.StencilBack_.DepthFail_ != BoundDepthStencilState.StencilBack_.DepthFail_ ||
		DepthStencilState.StencilBack_.Pass_ != BoundDepthStencilState.StencilBack_.Pass_ )
	{
		GL( StencilOpSeparate( 
			GL_BACK,
			gStencilOp[ (BcU32)DepthStencilState.StencilBack_.Fail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilBack_.DepthFail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilBack_.Pass_ ] ) );
	}

	const auto& RasteriserState = Desc.RasteriserState_;
	const auto& BoundRasteriserState = BoundRenderStateDesc_.RasteriserState_;

#if !defined( RENDER_USE_GLES )
	if( Version_.SupportPolygonMode_ )
	{
		if( Force ||
			RasteriserState.FillMode_ != BoundRasteriserState.FillMode_ )
		{
			GL( PolygonMode( GL_FRONT_AND_BACK, RsFillMode::SOLID == RasteriserState.FillMode_ ? GL_FILL : GL_LINE ) );
		}
	}
#endif

	if( Force ||
		RasteriserState.CullMode_ != BoundRasteriserState.CullMode_ )
	{
		switch( RasteriserState.CullMode_ )
		{
		case RsCullMode::NONE:
			GL( Disable( GL_CULL_FACE ) );
			break;
		case RsCullMode::CW:
			GL( Enable( GL_CULL_FACE ) );
			GL( CullFace( GL_FRONT ) );
			break;
		case RsCullMode::CCW:
			GL( Enable( GL_CULL_FACE ) );
			GL( CullFace( GL_BACK ) );
			break;
		default:
			BcBreakpoint;
		}
	}

	// TODO DepthBias_
	// TODO SlopeScaledDepthBias_
	// TODO DepthClipEnable_

	if( Force ||
		RasteriserState.ScissorEnable_ != BoundRasteriserState.ScissorEnable_ )
	{
		if( RasteriserState.ScissorEnable_ )
		{
			GL( Enable( GL_SCISSOR_TEST ) );
		}
		else
		{
			GL( Disable( GL_SCISSOR_TEST ) );
		}
	}

#if !defined( RENDER_USE_GLES )
	if( Version_.SupportAntialiasedLines_ )
	{
		if( Force ||
			RasteriserState.AntialiasedLineEnable_ != BoundRasteriserState.AntialiasedLineEnable_ )
		{
			if( RasteriserState.AntialiasedLineEnable_ )
			{
				GL( Enable( GL_LINE_SMOOTH ) );
			}
			else
			{
				GL( Disable( GL_LINE_SMOOTH ) );
			}
		}
	}
#endif
	
	

	// Copy over. Could do less work. Look into this later.
	BoundRenderStateDesc_ = Desc;
}
