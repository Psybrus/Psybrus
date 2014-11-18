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

#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"

#include "System/Renderer/RsVertexDeclaration.h"
#include "System/Renderer/RsViewport.h"

#include "System/Os/OsClient.h"

#include "Base/BcMath.h"

#include <memory>
#include <boost/format.hpp>

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// Debug output.
#if !defined( PSY_PRODUCTION )
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
		BcPrintf( "Source: %x, Type: %x, Id: %x, Severity: %s\n - %s\n",
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
	
	// Mipmapping nearest
	GL_NEAREST_MIPMAP_NEAREST,
	GL_LINEAR_MIPMAP_NEAREST,
	
	// Mipmapping linear
	GL_NEAREST_MIPMAP_LINEAR,
	GL_LINEAR_MIPMAP_LINEAR
};

static GLenum gTextureSampling[] = 
{
	GL_REPEAT,
	GL_MIRRORED_REPEAT,
	GL_CLAMP_TO_EDGE,
	GL_DECAL
};

static GLenum gTextureTypes[] = 
{
	0,
	GL_TEXTURE_1D,
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
	GL_LINES_ADJACENCY,				// RsTopologyType::LINE_LIST_ADJACENCY,
	GL_LINE_STRIP_ADJACENCY,		// RsTopologyType::LINE_STRIP_ADJACENCY,
	GL_TRIANGLES,					// RsTopologyType::TRIANGLE_LIST,
	GL_TRIANGLE_STRIP,				// RsTopologyType::TRIANGLE_STRIP,
	GL_TRIANGLES_ADJACENCY,			// RsTopologyType::TRIANGLE_LIST_ADJACENCY,
	GL_TRIANGLE_STRIP_ADJACENCY,	// RsTopologyType::TRIANGLE_STRIP_ADJACENCY,
	GL_TRIANGLE_FAN,				// RsTopologyType::TRIANGLE_FAN,
	GL_PATCHES						// RsTopologyType::PATCHES,
};

static GLenum gBufferType[] =
{
	0,								// RsBufferType::UNKNOWN
	GL_ARRAY_BUFFER,				// RsBufferType::VERTEX
	GL_ELEMENT_ARRAY_BUFFER,		// RsBufferType::INDEX
	GL_UNIFORM_BUFFER,				// RsBufferType::UNIFORM
	GL_IMAGE_BUFFER,				// RsBufferType::UNORDERED_ACCESS
	GL_DRAW_INDIRECT_BUFFER,		// RsBufferType::DRAW_INDIRECT
	GL_TRANSFORM_FEEDBACK_BUFFER,	// RsBufferType::STREAM_OUT
};

static GLenum gTextureType[] =
{
	0,								// RsTextureType::UNKNOWN
	GL_TEXTURE_1D,					// RsTextureType::TEX1D
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
	{ BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0 },	// RsTextureFormat::DXT1,
	{ BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0 }, // RsTextureFormat::DXT3,
	{ BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0 }, // RsTextureFormat::DXT5,
	// Depth stencil.
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT16, 0, 0 },			// RsTextureFormat::D16,
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT32, 0, 0 },			// RsTextureFormat::D32,
	{ BcFalse, BcTrue, GL_DEPTH24_STENCIL8, 0, 0 },				// RsTextureFormat::D24S8,
	{ BcFalse, BcTrue, GL_DEPTH_COMPONENT32F, 0, 0 },			 // RsTextureFormat::D32F,

};

static GLenum gShaderType[] = 
{
	GL_VERTEX_SHADER,											// RsShaderType::VERTEX
	GL_TESS_CONTROL_SHADER,										// RsShaderType::HULL
	GL_TESS_EVALUATION_SHADER,									// RsShaderType::DOMAIN
	GL_GEOMETRY_SHADER,											// RsShaderType::GEOMETRY
	GL_FRAGMENT_SHADER,											// RsShaderType::PIXEL
#if !PLATFORM_HTML5
	GL_COMPUTE_SHADER,											// RsShaderType::COMPUTE
#endif
};

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextGL::RsContextGL( OsClient* pClient, RsContextGL* pParent ):
	RsContext( pParent ),
	pParent_( pParent ),
	pClient_( pClient ),
	ScreenshotRequested_( BcFalse ),
	OwningThread_( BcErrorCode ),
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
	NoofTextureStateBinds_ = 0;
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
// isShaderCodeTypeSupported
//virtual
BcBool RsContextGL::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	switch( CodeType )
	{
	case RsShaderCodeType::GLSL_140:
		if( Version_.Major_ >= 3 &&
			Version_.Minor_ >= 1 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_150:
		if( Version_.Major_ >= 3 &&
			Version_.Minor_ >= 2 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_330:
		if( Version_.Major_ >= 3 &&
			Version_.Minor_ >= 3 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_400:
		if( Version_.Major_ >= 4 &&
			//Version_.Minor_ >= 0 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_410:
		if( Version_.Major_ >= 4 &&
			Version_.Minor_ >= 1 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_420:
		if( Version_.Major_ >= 4 &&
			Version_.Minor_ >= 2 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_430:
		if( Version_.Major_ >= 4 &&
			Version_.Minor_ >= 3 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_440:
		if( Version_.Major_ >= 4 &&
			Version_.Minor_ >= 4 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_450:
		if( Version_.Major_ >= 4 &&
			Version_.Minor_ >= 5 &&
			Version_.Type_ == RsOpenGLType::CORE )
		{
			return BcTrue;
		}
	case RsShaderCodeType::GLSL_ES_100:
		if( Version_.Major_ >= 2 &&
			//Version_.Minor_ >= 0 &&
			Version_.Type_ == RsOpenGLType::ES )
		{
			return BcTrue;
		}
	default:
		break;
	}
	return BcFalse;
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
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( ScreenshotRequested_ == BcFalse )
	{
		glFlush();
		RsGLCatchError();
	}
	else
	{
		// Finish all rendering.
		glFinish();
		
		// Read the back buffer.
		glReadBuffer( GL_BACK );
		BcU32* pImageData = new BcU32[ getWidth() * getHeight() ];
		BcU32* pReadImageData = pImageData;
		glReadPixels(0, 0, getWidth(), getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
		
		// Convert to image.
		ImgImage* pImage = new ImgImage();
		pImage->create( getWidth(), getHeight(), NULL );
		
		BcU32 W = getWidth();
		BcU32 H = getHeight();
		for( BcU32 Y = 0; Y < H; ++Y )
		{
			BcU32 RealY = ( H - Y ) - 1;
			for( BcU32 X = 0; X < W; ++X )
			{
				ImgColour* pColour = (ImgColour*)pReadImageData++;
				pImage->setPixel( X, RealY, *pColour );
			}
		}
		
		// Save out image.				
		// NEILO TODO: Generate an automatic filename.
		Img::save( "screenshot.png", pImage );

		// Free image.
		delete pImage;

		// Free image data.
		delete [] pImageData;

		// No more screenshot requested.
		ScreenshotRequested_ = BcFalse;
	}

#if PLATFORM_WINDOWS
	::SwapBuffers( WindowDC_ );
#endif

#if PLATFORM_LINUX
	SDL_GL_SwapWindow( reinterpret_cast< SDL_Window* >( pClient_->getDeviceHandle() ) );
#endif

	RsGLCatchError();
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
		BcPrintf( "Can't create pixel format.\n" );
	}
	
	if( !::SetPixelFormat( WindowDC_, PixelFormat, &pfd ) )               // Are We Able To Set The Pixel Format?
	{
	    BcPrintf( "Can't Set The PixelFormat." );
	}

	// Create a rendering context to start with.
	WindowRC_ = wglCreateContext( WindowDC_ );
	BcAssertMsg( WindowRC_ != NULL, "RsCoreImplGL: Render context is NULL!" );

	// Make current.
	wglMakeCurrent( WindowDC_, WindowRC_ );

	// Init GLEW.
	glewInit();
	
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
		RsOpenGLVersion( 3, 1, RsOpenGLType::CORE, RsShaderCodeType::GLSL_140 ),
	};

	HGLRC ParentContext = pParent_ != NULL ? pParent_->WindowRC_ : NULL;
	for( auto Version : Versions )
	{
		if( createProfile( Version, ParentContext ) )
		{
			Version_ = Version;
			BcPrintf( "RsContextGL: Created OpenGL %u.%u %s Profile.\n", 
				Version.Major_, 
				Version.Minor_,
				Version.Type_ == RsOpenGLType::CORE ? "Core" : "Compatibility" );
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
#endif

#if PLATFORM_LINUX	
	SDL_GL_SetAttribute( SDL_GL_RED_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_GREEN_SIZE, 8 );
	SDL_GL_SetAttribute( SDL_GL_BLUE_SIZE, 8 );
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
	};

	BcAssert( pParent_ == nullptr );
	SDL_Window* Window = reinterpret_cast< SDL_Window* >( pClient_->getDeviceHandle() );
	bool Success = false;
	for( auto Version : Versions )
	{
		if( createProfile( Version, Window ) )
		{
			Version_ = Version;
			BcPrintf( "RsContextGL: Created OpenGL %u.%u %s Profile.\n", 
				Version.Major_, 
				Version.Minor_,
				Version.Type_ == RsOpenGLType::CORE ? "Core" : "Compatibility" );
			break;
		}
		else
		{
			BcPrintf( "RsContextGL: Failed to create OpenGL %u.%u %s Profile.\n", 
				Version.Major_, 
				Version.Minor_,
				Version.Type_ == RsOpenGLType::CORE ? "Core" : "Compatibility" );

		}
	}

	// Init GLEW.
	glewExperimental = 1;
	glewInit();
	glGetError();
#endif

	// Debug output extension.	
#if !defined( PSY_PRODUCTION )
	if( GLEW_ARB_debug_output )
	{
		glDebugMessageCallbackARB( debugOutput, nullptr );
		glGetError();
	}
#endif

	glGetError();
	RsGLCatchError();
	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	// Create + bind global VAO.
	if( Version_.Major_ >= 3 )
	{
		BcAssert( glGenVertexArrays != nullptr );
		BcAssert( glBindVertexArray != nullptr );
		glGenVertexArrays( 1, &GlobalVAO_ );
		glBindVertexArray( GlobalVAO_ );
		RsGLCatchError();
	}

	// Set default state.
	setDefaultState();

	// Clear screen and flip.
	clear( RsColour( 0.0f, 0.0f, 0.0f, 0.0f ) );

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
	// Destroy global VAO.
	glBindVertexArray( 0 );
	glDeleteVertexArrays( 1, &GlobalVAO_ );

#if PLATFORM_WINDOWS
	// Destroy rendering context.
	wglMakeCurrent( WindowDC_, NULL );
	wglDeleteContext( WindowRC_ );
#endif

#if PLATFORM_LINUX
	SDL_GL_DeleteContext( SDLGLContext_ );
#endif
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
		ContextAttribs[ 1 ] = WGL_CONTEXT_ES2_PROFILE_BIT_EXT;
		break;
	}
	

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
#if PLATFORM_LINUX
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
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextGL::destroyRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextGL::createSamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextGL::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextGL::createBuffer( RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& BufferDesc = Buffer->getDesc();

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

	// Generate buffer.
	GLuint Handle;
	glGenBuffers( 1, &Handle );
	Buffer->setHandle( Handle );

	// Catch gen error.
	RsGLCatchError();

	// Attempt to update it.
	if( Handle != 0 )
	{
		glBindBuffer( TypeGL, Handle );
		glBufferData( TypeGL, BufferDesc.SizeBytes_, nullptr, UsageFlagsGL );

		// Catch update error.
		RsGLCatchError();

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextGL::destroyBuffer( RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	GLuint Handle = Buffer->getHandle< GLuint >();
	
	if( Handle != 0 )
	{
		glDeleteBuffers( 1, &Handle );
		Buffer->setHandle< GLuint >( 0 );

		RsGLCatchError();
		return true;
	}

	return false;
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
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	GLuint Handle = Buffer->getHandle< GLuint >();
	
	if( Handle != 0 )
	{
		const auto& BufferDesc = Buffer->getDesc();

		// Get buffer type for GL.
		auto TypeGL = gBufferType[ (BcU32)BufferDesc.Type_ ];

		// Get access flags for GL.
		GLbitfield AccessFlagsGL = 
			GL_MAP_WRITE_BIT | 
			GL_MAP_INVALIDATE_RANGE_BIT;

		// Bind buffer.
		glBindBuffer( TypeGL, Handle );

		// Map and update buffer.
		auto LockedPointer = glMapBufferRange( TypeGL, Offset, Size, AccessFlagsGL );
		if( LockedPointer != nullptr )
		{
			RsBufferLock Lock = 
			{
				LockedPointer
			};
			UpdateFunc( Buffer, Lock );
			glUnmapBuffer( TypeGL );
		}

		RsGLCatchError();

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
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
	if( ( TextureDesc.Flags_ & RsResourceCreationFlags::STATIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STATIC_DRAW;
	}
	else if( ( TextureDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_DYNAMIC_DRAW;
	}
	else if( ( TextureDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
	{
		UsageFlagsGL |= GL_STREAM_DRAW;
	}

	// Create GL texture.
	GLuint Handle;
	glGenTextures( 1, &Handle );
	Texture->setHandle( Handle );
	
	RsGLCatchError();		

	if( Handle != 0 )
	{
		// Bind texture.
		glBindTexture( TypeGL, Handle );

		// Set max levels.
		glTexParameteri( TypeGL, GL_TEXTURE_MAX_LEVEL, TextureDesc.Levels_ - 1 );

		// Instanciate levels.
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
	GLuint Handle = Texture->getHandle< GLuint >();
	if( Handle != 0 )
	{
		// Delete it.
		glDeleteTextures( 1, &Handle );
		setHandle< GLuint >( 0 );

		RsGLCatchError();

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

	GLuint Handle = Texture->getHandle< GLuint >();

	const auto& TextureDesc = Texture->getDesc();

	if( Handle != 0 )
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

		RsTextureLock Lock = 
		{
			&Data[ 0 ],
			TextureDesc.Width_,
			TextureDesc.Width_ * TextureDesc.Height_
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
	GLuint Handle = glCreateShader( ShaderType );
	RsGLCatchError();
	
	if( Handle != 0 )
	{
		//
		const GLchar* ShaderData = reinterpret_cast< const GLchar* >( Shader->getData() );

		// Load the source code into it.
		glShaderSource( Handle, 1, &ShaderData, nullptr );
		RsGLCatchError();
				
		// Compile the source code.
		glCompileShader( Handle );
		RsGLCatchError();
				
		// Test if compilation succeeded.
		GLint ShaderCompiled;
		glGetShaderiv( Handle, GL_COMPILE_STATUS, &ShaderCompiled );
		if ( !ShaderCompiled )
		{					 
			// There was an error here, first get the length of the log message.
			int i32InfoLogLength, i32CharsWritten; 
			glGetShaderiv( Handle, GL_INFO_LOG_LENGTH, &i32InfoLogLength );
					
			// Allocate enough space for the message, and retrieve it.
			char* pszInfoLog = new char[i32InfoLogLength];
			glGetShaderInfoLog( Handle, i32InfoLogLength, &i32CharsWritten, pszInfoLog );

			BcPrintf( "=======================================================\n" );
			BcPrintf( "Error Compiling shader:\n" );
			BcPrintf( "RsShaderGL: Infolog:\n %s\n", pszInfoLog );
			BcPrintf( "=======================================================\n" );
			delete [] pszInfoLog;

			glDeleteShader( Handle );
			return false;
		}
	}
	
	// Destroy if there is a failure.
	GLenum Error = glGetError();
	if ( Error != GL_NO_ERROR )
	{
		BcPrintf( "RsShaderGL: Error has occured: %u\n", Error );
		glDeleteShader( Handle );
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
	glDeleteShader( Handle );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextGL::createProgram(
	RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& Shaders = Program->getShaders();

	// Some checks to ensure validity.
	BcAssert( Shaders.size() > 0 );	

	// Create program.
	GLuint Handle = glCreateProgram();
	BcAssert( Handle != 0 );

	// Attach shaders.
	for( auto* Shader : Shaders )
	{
		glAttachShader( Handle, Shader->getHandle< GLuint >() );
		RsGLCatchError();
	}
	
	// Bind all slots up.
	// NOTE: We shouldn't need this in later GL versions with explicit
	//       binding slots.
	for( BcU32 Channel = 0; Channel < 16; ++Channel )
	{
		const std::string Name = boost::str( boost::format( "dcl_Input%1%" ) % Channel );
		glBindAttribLocation( Handle, Channel, Name.c_str() );
		RsGLCatchError();
	}
	
	// Link program.
	glLinkProgram( Handle );
	RsGLCatchError();

	GLint ProgramLinked = 0;
	glGetProgramiv( Handle, GL_LINK_STATUS, &ProgramLinked );
	if ( !ProgramLinked )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		glGetProgramiv( Handle, GL_INFO_LOG_LENGTH, &i32InfoLogLength );

		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog( Handle, i32InfoLogLength, &i32CharsWritten, pszInfoLog );
		BcPrintf( "RsProgramGL: Infolog:\n %s\n", pszInfoLog );
		delete [] pszInfoLog;

		glDeleteProgram( Handle );
		return false;
	}
	
	// Attempt to find uniform names.
	GLint ActiveUniforms = 0;
	glGetProgramiv( Handle, GL_ACTIVE_UNIFORMS, &ActiveUniforms );
	
	BcU32 ActiveSamplerIdx = 0;
	for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniforms; ++Idx )
	{
		// Uniform information.
		GLchar UniformName[ 256 ];
		GLsizei UniformNameLength = 0;
		GLint Size = 0;
		GLenum Type = GL_INVALID_VALUE;

		// Get the uniform.
		glGetActiveUniform( Handle, Idx, sizeof( UniformName ), &UniformNameLength, &Size, &Type, UniformName );
		
		// Add it as a parameter.
		if( UniformNameLength > 0 && Type != GL_INVALID_VALUE )
		{
			GLint UniformLocation = glGetUniformLocation( Handle, UniformName );

			// Trim index off.
			BcChar* pIndexStart = BcStrStr( UniformName, "[0]" );
			if( pIndexStart != NULL )
			{
				*pIndexStart = '\0';
			}

			RsShaderParameterType InternalType = RsShaderParameterType::INVALID;
			switch( Type )
			{
			case GL_SAMPLER_1D:
				InternalType = RsShaderParameterType::SAMPLER_1D;
				break;
			case GL_SAMPLER_2D:
				InternalType = RsShaderParameterType::SAMPLER_2D;
				break;
			case GL_SAMPLER_3D:
				InternalType = RsShaderParameterType::SAMPLER_3D;
				break;
			case GL_SAMPLER_CUBE:
				InternalType = RsShaderParameterType::SAMPLER_CUBE;
				break;
			case GL_SAMPLER_1D_SHADOW:
				InternalType = RsShaderParameterType::SAMPLER_1D_SHADOW;
				break;
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
				glUseProgram( Handle );
				glUniform1i( UniformLocation, ActiveSamplerIdx );
				glUseProgram( 0 );
				++ActiveSamplerIdx;
				RsGLCatchError();
			}
		}
	}
	
	// Attempt to find uniform block names.
	GLint ActiveUniformBlocks = 0;
	glGetProgramiv( Handle, GL_ACTIVE_UNIFORM_BLOCKS, &ActiveUniformBlocks );
	
	BcU32 ActiveUniformSlotIndex = 0;
	for( BcU32 Idx = 0; Idx < (BcU32)ActiveUniformBlocks; ++Idx )
	{
		// Uniform information.
		GLchar UniformBlockName[ 256 ];
		GLsizei UniformBlockNameLength = 0;
		GLint Size = 0;

		// Get the uniform block size.
		glGetActiveUniformBlockiv( Handle, Idx, GL_UNIFORM_BLOCK_DATA_SIZE, &Size );
		glGetActiveUniformBlockName( Handle, Idx, sizeof( UniformBlockName ), &UniformBlockNameLength, UniformBlockName );
		
		// Add it as a parameter.
		if( UniformBlockNameLength > 0 )
		{
			auto TestIdx = glGetUniformBlockIndex( Handle, UniformBlockName );
			BcAssert( TestIdx == Idx );
			BcUnusedVar( TestIdx );

			Program->addUniformBufferSlot( UniformBlockName, Idx, Size );

			glUniformBlockBinding( Handle, Idx, ActiveUniformSlotIndex++ );
			RsGLCatchError();
		}
	}

	// Catch error.
	RsGLCatchError();

	// Validate program.
	glValidateProgram( Handle );
	GLint ProgramValidated = 0;
	glGetProgramiv( Handle, GL_VALIDATE_STATUS, &ProgramValidated );
	if ( !ProgramLinked )
	{					 
		// There was an error here, first get the length of the log message.
		int i32InfoLogLength, i32CharsWritten; 
		glGetProgramiv( Handle, GL_INFO_LOG_LENGTH, &i32InfoLogLength );

		// Allocate enough space for the message, and retrieve it.
		char* pszInfoLog = new char[i32InfoLogLength];
		glGetProgramInfoLog( Handle, i32InfoLogLength, &i32CharsWritten, pszInfoLog );
		BcPrintf( "RsProgramGL: Infolog:\n %s\n", pszInfoLog );
		delete [] pszInfoLog;

		glDeleteProgram( Handle );
		return false;
	}

	// Set handle.
	Program->setHandle( Handle );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextGL::destroyProgram(
	class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	GLuint Handle = Program->getHandle< GLuint >();
	glDeleteProgram( Handle );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextGL::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Setup default texture states.
	RsTextureParams TextureParams = 
	{
		RsTextureFilteringMode::LINEAR, RsTextureFilteringMode::LINEAR, RsTextureSamplingMode::WRAP, RsTextureSamplingMode::WRAP
	};

	for( BcU32 Sampler = 0; Sampler < MAX_TEXTURE_SLOTS; ++Sampler )
	{
		setSamplerState( Sampler, TextureParams, BcTrue );
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

}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextGL::setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextGL::getRenderState( RsRenderStateType State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextGL::setSamplerState( BcU32 Sampler, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( Sampler < MAX_TEXTURE_SLOTS )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Sampler ];
		
		const BcBool WasDirty = TextureStateValue.Dirty_;
		
		TextureStateValue.Dirty_ |= ( TextureStateValue.Params_ != Params ) || Force;
		TextureStateValue.Params_ = Params;
	
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && TextureStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofTextureStateBinds_ < MAX_TEXTURE_SLOTS );
			TextureStateBinds_[ NoofTextureStateBinds_++ ] = Sampler;
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

	if( UniformBuffers_[ SlotIdx ].Buffer_ != UniformBuffer )
	{
		UniformBuffers_[ SlotIdx ].Buffer_ = UniformBuffer;
		BindingsDirty_ = BcTrue;
		ProgramDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setPrimitive
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
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RsGLCatchError();
	
	// Bind render states.
	// TODO: Check for redundant state.
	if( RenderState_ != nullptr )
	{
		const auto& Desc = RenderState_->getDesc();
#if 0 
		// GL4+
		for( BcU32 Idx = 0; Idx < 8; ++Idx )
		{
			const auto& RenderTarget = Desc.BlendState_.RenderTarget_[ Idx ];

			if( MainRenderTarget.Enable_ )
			{
				glEnablei( GL_BLEND, Idx );
			}
			else
			{
				glDisablei( GL_BLEND, Idx );		
			}

			glBlendEquationSeparatei( 
				Idx, 
				gBlendOp[ (BcU32)RenderTarget.BlendOp_ ], gBlendOp[ (BcU32)RenderTarget.BlendOpAlpha_ ] );
			RsGLCatchError();

			glBlendFuncSeparatei( 
				Idx, 
				gBlendType[ (BcU32)RenderTarget.SrcBlend_ ], gBlendType[ (BcU32)RenderTarget.DestBlend_ ],
				gBlendType[ (BcU32)RenderTarget.SrcBlendAlpha_ ], gBlendType[ (BcU32)RenderTarget.DestBlendAlpha_ ] );
			RsGLCatchError();

			glColorMaski(
				Idx,
				RenderTarget.WriteMask_ & 1 ? GL_TRUE : GL_FALSE,
				RenderTarget.WriteMask_ & 2 ? GL_TRUE : GL_FALSE,
				RenderTarget.WriteMask_ & 4 ? GL_TRUE : GL_FALSE,
				RenderTarget.WriteMask_ & 8 ? GL_TRUE : GL_FALSE );
			RsGLCatchError();
		}
#else
		const auto& MainRenderTarget = Desc.BlendState_.RenderTarget_[ 0 ];

		MainRenderTarget.Enable_ ? glEnable( GL_BLEND ) : glDisable( GL_BLEND );

		glBlendEquationSeparate( 
			gBlendOp[ (BcU32)MainRenderTarget.BlendOp_ ], gBlendOp[ (BcU32)MainRenderTarget.BlendOpAlpha_ ] );

		glBlendFuncSeparate( 
			gBlendType[ (BcU32)MainRenderTarget.SrcBlend_ ], gBlendType[ (BcU32)MainRenderTarget.DestBlend_ ],
			gBlendType[ (BcU32)MainRenderTarget.SrcBlendAlpha_ ], gBlendType[ (BcU32)MainRenderTarget.DestBlendAlpha_ ] );

		for( BcU32 Idx = 0; Idx < 8; ++Idx )
		{
			const auto& RenderTarget = Desc.BlendState_.RenderTarget_[ Idx ];
			glColorMaski(
				Idx,
				RenderTarget.WriteMask_ & 1 ? GL_TRUE : GL_FALSE,
				RenderTarget.WriteMask_ & 2 ? GL_TRUE : GL_FALSE,
				RenderTarget.WriteMask_ & 4 ? GL_TRUE : GL_FALSE,
				RenderTarget.WriteMask_ & 8 ? GL_TRUE : GL_FALSE );
			RsGLCatchError();
		}
#endif

		const auto& DepthStencilState = Desc.DepthStencilState_;
		
		DepthStencilState.DepthTestEnable_ ? glEnable( GL_DEPTH_TEST ) : glDisable( GL_DEPTH_TEST );
		glDepthMask( (GLboolean)DepthStencilState.DepthWriteEnable_ );
		glDepthFunc( gCompareMode[ (BcU32)DepthStencilState.DepthFunc_ ] );

		DepthStencilState.StencilEnable_ ? glEnable( GL_STENCIL_TEST ) : glDisable( GL_STENCIL_TEST );

		glStencilFuncSeparate( 
			GL_FRONT,
			gCompareMode[ (BcU32)DepthStencilState.StencilFront_.Func_ ], 
			DepthStencilState.StencilFront_.Ref_, DepthStencilState.StencilFront_.Mask_ );

		glStencilFuncSeparate( 
			GL_BACK,
			gCompareMode[ (BcU32)DepthStencilState.StencilBack_.Func_ ], 
			DepthStencilState.StencilBack_.Ref_, DepthStencilState.StencilBack_.Mask_ );

		glStencilOpSeparate( 
			GL_FRONT,
			gStencilOp[ (BcU32)DepthStencilState.StencilFront_.Fail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilFront_.DepthFail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilFront_.Pass_ ] );

		glStencilOpSeparate( 
			GL_BACK,
			gStencilOp[ (BcU32)DepthStencilState.StencilBack_.Fail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilBack_.DepthFail_ ], 
			gStencilOp[ (BcU32)DepthStencilState.StencilBack_.Pass_ ] );

		const auto& RasteriserState = Desc.RasteriserState_;

		glPolygonMode( GL_FRONT_AND_BACK, RsFillMode::SOLID == RasteriserState.FillMode_ ? GL_FILL : GL_LINE );

		switch( RasteriserState.CullMode_ )
		{
		case RsCullMode::NONE:
			glDisable( GL_CULL_FACE );
			break;
		case RsCullMode::CW:
			glEnable( GL_CULL_FACE );
			glCullFace( GL_FRONT );
			break;
		case RsCullMode::CCW:
			glEnable( GL_CULL_FACE );
			glCullFace( GL_BACK );
			break;
		default:
			BcBreakpoint;
		}

		// TODO DepthBias_
		// TODO SlopeScaledDepthBias_
		// TODO DepthClipEnable_
		// TODO ScissorEnable_

		RasteriserState.AntialiasedLineEnable_ ? glEnable( GL_LINE_SMOOTH) : glDisable( GL_LINE_SMOOTH );
	}

	// Bind texture states.
	for( BcU32 TextureStateIdx = 0; TextureStateIdx < NoofTextureStateBinds_; ++TextureStateIdx )
	{
		BcU32 TextureStateID = TextureStateBinds_[ TextureStateIdx ];
		TTextureStateValue& TextureStateValue = TextureStateValues_[ TextureStateID ];

		if( TextureStateValue.Dirty_ )
		{
			RsTexture* pTexture = TextureStateValue.pTexture_;
			const RsTextureParams& Params = TextureStateValue.Params_;
			const RsTextureType InternalType = pTexture ? pTexture->getDesc().Type_ : RsTextureType::TEX2D;
			const GLenum TextureType = gTextureTypes[ (BcU32)InternalType ];

			glActiveTexture( GL_TEXTURE0 + TextureStateID );
			glBindTexture( TextureType, pTexture ? pTexture->getHandle< GLuint >() : 0 );
			RsGLCatchError();

			if( pTexture )
			{
				glTexParameteri( TextureType, GL_TEXTURE_MIN_FILTER, gTextureFiltering[ (BcU32)Params.MinFilter_ ] );
				glTexParameteri( TextureType, GL_TEXTURE_MAG_FILTER, gTextureFiltering[ (BcU32)Params.MagFilter_ ] );
				glTexParameteri( TextureType, GL_TEXTURE_WRAP_S, gTextureSampling[ (BcU32)Params.UMode_ ] );
				glTexParameteri( TextureType, GL_TEXTURE_WRAP_T, gTextureSampling[ (BcU32)Params.VMode_ ] );	
				glTexParameteri( TextureType, GL_TEXTURE_WRAP_R, gTextureSampling[ (BcU32)Params.WMode_ ] );	
				RsGLCatchError();
			}

			TextureStateValue.Dirty_ = BcFalse;
		}
	}
	
	// Reset binds.
	NoofTextureStateBinds_ = 0;

	RsGLCatchError();

	// Bind program and primitive.
	if( ( Program_ != nullptr ||
		VertexDeclaration_ != nullptr ) &&
		( ProgramDirty_ || BindingsDirty_ ) )
	{
		const auto& ProgramVertexAttributeList = Program_->getVertexAttributeList();
		const auto& VertexDeclarationDesc = VertexDeclaration_->getDesc();
		const auto& PrimitiveVertexElementList = VertexDeclarationDesc.Elements_;

		// Bind program.
		glUseProgram( Program_->getHandle< GLuint >() );
		RsGLCatchError();

		// TODO: Bind up as individual uniforms as an alternative
		//       to uniform buffers where appropriate.
		BcU32 BindingPoint = 0;
		for( auto It( UniformBuffers_.begin() ); It != UniformBuffers_.end(); ++It )
		{
			auto Buffer = (*It).Buffer_;
			if( Buffer != nullptr )
			{
				glBindBufferRange( GL_UNIFORM_BUFFER, BindingPoint, Buffer->getHandle< GLuint >(), 0, Buffer->getDesc().SizeBytes_ );
				++BindingPoint;
				RsGLCatchError();
			}
		}


		// Cached vertex handle for binding.
		GLuint BoundVertexHandle = 0;

		// Brute force disable vertex arrays.
		for( BcU32 Idx = 0; Idx < 16; ++Idx )
		{
			glDisableVertexAttribArray( Idx );
		}

		// Bind up all elements to attributes.
		for( const auto& Attribute : ProgramVertexAttributeList )
		{
			for( const auto& Element : PrimitiveVertexElementList )
			{
				// Found an element we can bind to.
				if( Attribute.Usage_ == Element.Usage_ &&
					Attribute.UsageIdx_ == Element.UsageIdx_ )
				{
					auto VertexBufferBinding = VertexBuffers_[ Element.StreamIdx_ ];
					auto VertexBuffer = VertexBufferBinding.Buffer_;
					auto VertexStride = VertexBufferBinding.Stride_;
				
					// Bind up new vertex buffer if we need to.
					BcAssertMsg( Element.StreamIdx_ < VertexBuffers_.size(), "Stream index out of bounds for primitive." );
					BcAssertMsg( VertexBuffer != nullptr, "Vertex buffer not bound!" );
					GLuint VertexHandle = VertexBuffer->getHandle< GLuint >();
					if( BoundVertexHandle != VertexHandle )
					{
						glBindBuffer( GL_ARRAY_BUFFER, VertexHandle );
						BoundVertexHandle = VertexHandle;
					}

					// Enable array.
					glEnableVertexAttribArray( Attribute.Channel_ );

					// Bind.
					BcU64 CalcOffset = Element.Offset_;

					glVertexAttribPointer( Attribute.Channel_, 
						Element.Components_,
						gVertexDataTypes[ (BcU32)Element.DataType_ ],
						gVertexDataNormalised[ (BcU32)Element.DataType_ ],
						VertexStride,
						(GLvoid*)CalcOffset );
					break;	
				}
			}
		}

		// Bind indices.
		GLuint IndicesHandle = IndexBuffer_ != nullptr ? IndexBuffer_->getHandle< GLuint >() : 0;
		if( IndicesHandle != 0 )
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IndicesHandle );
		}

		ProgramDirty_ = BcFalse;
		BindingsDirty_ = BcFalse;
		RsGLCatchError();
	}
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextGL::clear( const RsColour& Colour )
{
	flushState();
	glClearColor( Colour.r(), Colour.g(), Colour.b(), Colour.a() );
	glClearDepth( 1.0f );
	glClearStencil( 0 );
	glDepthMask( GL_TRUE );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );	
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextGL::drawPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	flushState();
	glDrawArrays( gTopologyType[ (BcU32)TopologyType ], IndexOffset, NoofIndices );
	RsGLCatchError();
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextGL::drawIndexedPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	flushState();
	glDrawElementsBaseVertex( gTopologyType[ (BcU32)TopologyType ], NoofIndices, GL_UNSIGNED_SHORT, (void*)( IndexOffset * sizeof( BcU16 ) ), VertexOffset );
	RsGLCatchError();
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextGL::setViewport( class RsViewport& Viewport )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	glViewport( Viewport.x(), Viewport.y(), Viewport.width(), Viewport.height() );
	glDepthRangef( Viewport.zNear(), Viewport.zFar() );
	RsGLCatchError();
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

	glStencilFunc( gCompareMode[ CompareValue.Value_ ], RefValue.Value_, MaskValue.Value_ );

	CompareValue.Dirty_ = BcFalse;
	RefValue.Dirty_ = BcFalse;
	MaskValue.Dirty_ = BcFalse;

	RsGLCatchError();
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

	glStencilFunc( gCompareMode[ CompareValue.Value_ ], RefValue.Value_, MaskValue.Value_ );
	glStencilOp( gStencilOp[ SFailValue.Value_ ], gStencilOp[ DPFailValue.Value_ ], gStencilOp[ DPPassValue.Value_ ] );

	SFailValue.Dirty_ = BcFalse;
	DPFailValue.Dirty_ = BcFalse;
	DPPassValue.Dirty_ = BcFalse;

	RsGLCatchError();
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
			glDisable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ZERO );
			break;
					
		case RsBlendingMode::BLEND:
			glEnable( GL_BLEND );
			glBlendEquation( GL_FUNC_ADD );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
					
		case RsBlendingMode::ADD:
			glEnable( GL_BLEND );
			glBlendEquation( GL_FUNC_ADD );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;

		case RsBlendingMode::SUBTRACT:
			glEnable( GL_BLEND );
			glBlendEquation( GL_FUNC_REVERSE_SUBTRACT );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;
			
		default:
			BcBreakpoint;
			break;
	}

	RsGLCatchError();
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
	RsGLCatchError();

	GLuint Handle = Texture->getHandle< GLuint >();

	const auto& TextureDesc = Texture->getDesc();

	// Get buffer type for GL.
	auto TypeGL = gTextureType[ (BcU32)TextureDesc.Type_ ];

	// Bind.
	if( Bind )
	{
		glBindTexture( TypeGL, Handle );
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
			glTexImage1D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data );
			RsGLCatchError();
			break;

		case RsTextureType::TEX2D:
			glTexImage2D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data );
			RsGLCatchError();
			break;

		case RsTextureType::TEX3D:
			glTexImage3D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				Width,
				Height,
				Depth,
				0,
				FormatGL.Format_,
				FormatGL.Type_,
				Data );
			RsGLCatchError();
			break;

		case RsTextureType::TEXCUBE:
			BcBreakpoint;

		default:
			BcBreakpoint;
		}

	}
	else
	{
		if( DataSize == 0 || Data == nullptr )
		{
			return;
		}

		switch( TextureDesc.Type_ )
		{
		case RsTextureType::TEX1D:
			glCompressedTexImage1D( 
				TypeGL, 
				Slice.Level_,
				FormatGL.InternalFormat_,
				TextureDesc.Width_,
				0,
				DataSize,
				Data );
			RsGLCatchError();
			break;

		case RsTextureType::TEX2D:
			glCompressedTexImage2D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				TextureDesc.Width_,
				TextureDesc.Height_,
				0,
				DataSize,
				Data );
			RsGLCatchError();
			break;

		case RsTextureType::TEX3D:
			glCompressedTexImage3D( 
				TypeGL, 
				Slice.Level_, 
				FormatGL.InternalFormat_,
				TextureDesc.Width_,
				TextureDesc.Height_,
				TextureDesc.Depth_,
				0,
				DataSize,
				Data );
			RsGLCatchError();
			break;

		case RsTextureType::TEXCUBE:
			BcBreakpoint;

		default:
			BcBreakpoint;
		}
	}
}
