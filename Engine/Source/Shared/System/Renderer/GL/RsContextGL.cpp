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
#include "System/Renderer/GL/RsBufferGL.h"
#include "System/Renderer/GL/RsProgramGL.h"
#include "System/Renderer/GL/RsTextureGL.h"
#include "System/Renderer/GL/RsUtilsGL.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsGeometryBinding.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsProgramBinding.h"
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

#define ENABLE_DEBUG_OUTPUT ( 1 && !defined( PSY_PRODUCTION ) && !PLATFORM_HTML5 && !PLATFORM_ANDROID )

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
// Small util.
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
	InsideBeginEnd_( 0 ),
	Width_( 0 ),
	Height_( 0 ),
	ScreenshotFunc_(),
	OwningThread_( BcErrorCode ),
	FrameCount_( 0 ),
	GlobalVAO_( 0 )
{
	BcMemZero( &VertexBufferActiveState_[ 0 ], sizeof( VertexBufferActiveState_ ) );
	BcMemZero( &VertexBufferActiveNextState_[ 0 ], sizeof( VertexBufferActiveNextState_ ) );
	RenderState_ = nullptr;

	// Stats.
	NoofDrawCalls_ = 0;
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
// getClient
//virtual
OsClient* RsContextGL::getClient() const
{
	return pClient_;
}

//////////////////////////////////////////////////////////////////////////
// getFeatures
//virtual
const RsFeatures& RsContextGL::getFeatures() const
{
	return Version_.Features_;
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
// getWidth
//virtual
BcU32 RsContextGL::getWidth() const
{
	BcAssert( InsideBeginEnd_ == 1 );
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextGL::getHeight() const
{
	BcAssert( InsideBeginEnd_ == 1 );
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// getBackBufferRT
class RsTexture* RsContextGL::getBackBufferRT() const
{
	return BackBufferRT_;
}

//////////////////////////////////////////////////////////////////////////
// getBackBufferDS
class RsTexture* RsContextGL::getBackBufferDS() const
{
	return BackBufferDS_;
}

//////////////////////////////////////////////////////////////////////////
// beginFrame
void RsContextGL::beginFrame( BcU32 Width, BcU32 Height )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsFrame::endFrame" );
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcAssert( InsideBeginEnd_ == 0 );
	++InsideBeginEnd_;

#if PLATFORM_ANDROID
	//
	ANativeWindow* Window = static_cast< ANativeWindow* >( pClient_->getWindowHandle() );
	BcAssert( Window != nullptr );

	if( Width != Width_ || Height != Height_ || EGLWindow_ != Window )
	{
		EGLWindow_ = Window;

		// Destroy old surface.
		if( EGLSurface_ != nullptr )
		{
			if( !eglDestroySurface( EGLSurface_, EGLContext_ ) )
			{
				PSY_LOG( "eglDestroySurface() returned error %d", eglGetError() );
			}
		}

		ANativeWindow_setBuffersGeometry( Window, 0, 0, EGLFormat_ );

		// Recreate EGL surface for new window.
		if ( !( EGLSurface_ = eglCreateWindowSurface( EGLDisplay_, EGLConfig_, Window, 0 ) ) )
		{
			PSY_LOG( "eglCreateWindowSurface() returned error %d", eglGetError() );
			return;
		}
		else
		{
			PSY_LOG( "eglCreateWindowSurface() success" );
		}


		// Make context current with new surface.
		if ( !eglMakeCurrent( EGLDisplay_, EGLSurface_, EGLSurface_, EGLContext_ ) )
		{
			PSY_LOG( "eglMakeCurrent() returned error %d", eglGetError() );
			return;
		}
		else
		{
			PSY_LOG( "eglMakeCurrent() success" );
		}
	}

#endif

	if( Width_ != Width || Height_ != Height )
	{
		// Update dimensions.
		BackBufferRTDesc_.Width_ = Width;
		BackBufferRTDesc_.Height_ = Height;
		BackBufferDSDesc_.Width_ = Width;
		BackBufferDSDesc_.Height_ = Height;

		// Recreate textures in place.
		auto BackBufferRTInternal = BackBufferRT_->getHandle< BcU64 >();
		BackBufferRT_->~RsTexture();
		BackBufferRT_ = new ( BackBufferRT_ ) RsTexture( this, BackBufferRTDesc_ );
		BackBufferRT_->setHandle( BackBufferRTInternal );

		auto BackBufferDSInternal = BackBufferDS_->getHandle< BcU64 >();
		BackBufferDS_->~RsTexture();
		BackBufferDS_ = new ( BackBufferDS_ ) RsTexture( this, BackBufferDSDesc_ );
		BackBufferDS_->setHandle( BackBufferDSInternal );
	}

	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextGL::endFrame()
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsFrame::endFrame" );
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcAssert( InsideBeginEnd_ == 1 );
	--InsideBeginEnd_;

	GL( Flush() );		

	NoofDrawCalls_ = 0;

	auto ScreenshotFunc = std::move( ScreenshotFunc_ );
	if( ScreenshotFunc != nullptr )
	{
		BcBreakpoint;

		const BcU32 W = Width_;
		const BcU32 H = Height_;

		// Read the back buffer.
#if !PLATFORM_ANDROID && !PLATFORM_HTML5
		GL( ReadBuffer( GL_BACK ) );
#endif
		std::unique_ptr< BcU32[] > ImageData( new BcU32[ W * H ] );
		std::unique_ptr< BcU32[] > RowData( new BcU32[ W ] );
		GL( ReadPixels( 0, 0, W, H, GL_RGBA, GL_UNSIGNED_BYTE, ImageData.get() ) );

		const BcU32 RowPitch = sizeof( BcU32 ) * W;
		for( BcU32 Y = 0; Y < ( H / 2 ); ++Y )
		{
			const BcU32 OppositeY = ( H - Y ) - 1;

			// Swap rows to flip image.
			BcU32* RowA = ImageData.get() + ( Y * W );
			BcU32* RowB = ImageData.get() + ( OppositeY * W );
			memcpy( RowData.get(), RowA, RowPitch );
			memcpy( RowA, RowB, RowPitch );
			memcpy( RowB, RowData.get(), RowPitch );
		}

		RsScreenshot Screenshot;
		Screenshot.Data_ = ImageData.get();
		Screenshot.Width_ = W;
		Screenshot.Height_ = H;
		Screenshot.Format_ = RsTextureFormat::R8G8B8A8;
		if( ScreenshotFunc( Screenshot ) )
		{
			ScreenshotFunc_ = ScreenshotFunc;
		}
	}
	

#if PLATFORM_WINDOWS
	{
		PSY_PROFILER_SECTION( SwapRoot, "::SwapBuffers" );
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
void RsContextGL::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
	ScreenshotFunc_ = std::move( ScreenshotFunc );
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsContextGL::create()
{
	bool WantGLES = SysArgs_.find( "-gles" ) != std::string::npos;

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
		RsOpenGLVersion( 3, 2, RsOpenGLType::ES, RsShaderCodeType::GLSL_ES_310 ),
		RsOpenGLVersion( 3, 1, RsOpenGLType::ES, RsShaderCodeType::GLSL_ES_310 ),
		RsOpenGLVersion( 3, 0, RsOpenGLType::ES, RsShaderCodeType::GLSL_ES_300 ),
		RsOpenGLVersion( 2, 0, RsOpenGLType::ES, RsShaderCodeType::GLSL_ES_100 ),
	};

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

	auto RTFormat = RsTextureFormat::R8G8B8A8;
	auto DSFormat = RsTextureFormat::D24S8;
	
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

	auto RTFormat = RsTextureFormat::R8G8B8A8;
	auto DSFormat = RsTextureFormat::D24S8;

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

	auto RTFormat = RsTextureFormat::R8G8B8A8;
	auto DSFormat = RsTextureFormat::D16;

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
		BcAssertMsg( false, "eglInitialize() returned error %d", eglGetError() );
		return;
	}
	else
	{
		BcPrintf( "eglInitialize() success" );
	}

	if ( !eglChooseConfig( EGLDisplay_, EGLConfigAttribs, &EGLConfig_, 1, &EGLNumConfigs_ ) )
	{
		BcAssertMsg( false, "eglChooseConfig() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglChooseConfig() success" );
	}

	if ( !eglGetConfigAttrib( EGLDisplay_, EGLConfig_, EGL_NATIVE_VISUAL_ID, &EGLFormat_ ) ) 
	{
		BcAssertMsg( false, "eglGetConfigAttrib() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglGetConfigAttrib() success" );
	}

	ANativeWindow* Window = static_cast< ANativeWindow* >( pClient_->getWindowHandle() );
	BcAssert( Window != nullptr );
	EGLWindow_ = Window;

	ANativeWindow_setBuffersGeometry( Window, 0, 0, EGLFormat_ );
	
	PSY_LOG( "ANativeWindow_setBuffersGeometry() success" );

		if ( !( EGLContext_ = eglCreateContext( EGLDisplay_, EGLConfig_, 0, EGContextAttribs ) ) )
	{
		PSY_LOG( "eglCreateContext() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglCreateContext() success" );
	}
	
	if ( !( EGLSurface_ = eglCreateWindowSurface( EGLDisplay_, EGLConfig_, Window, 0 ) ) )
	{
		PSY_LOG( "eglCreateWindowSurface() returned error %d", eglGetError() );
		return;
	}
	else
	{
		PSY_LOG( "eglCreateWindowSurface() success" );
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
		PSY_LOG( "eglQuerySurface() success. %u x %u", EGLWidth_, EGLHeight_ );
		OsClientAndroid* Client = static_cast< OsClientAndroid* >( pClient_ );
		Client->setSize( EGLWidth_, EGLHeight_ );
		Width_ = EGLWidth_;
		Height_ = EGLHeight_;
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
	if( GLEW_KHR_debug )
	{
		GL( DebugMessageCallback( debugOutput, nullptr ) );
		GL( GetError() );
		PSY_LOG( "INFO: Using GLEW_KHR_debug" );
	}
	else
	{
		PSY_LOG( "WARNING: No GLEW_KHR_debug" );
	}
#endif // ENABLE_DEBUG_OUTPUT

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

#if !PLATFORM_ANDROID
	// Create + bind global VAO.
	if( Version_.SupportVAOs_ )
	{
		BcAssert( glGenVertexArrays != nullptr );
		BcAssert( glBindVertexArray != nullptr );
		GL( GenVertexArrays( 1, &GlobalVAO_ ) );
		GL( BindVertexArray( GlobalVAO_ ) );		
	}

	// Create transfer FBO.
	GL( GenFramebuffers( 2, TransferFBOs_ ) );
#endif

	// Force set render state to the default.
	// Initialises redundant state caching.
	RsRenderStateDesc RenderStateDesc = BoundRenderStateDesc_;
	bindRenderStateDesc( RenderStateDesc, BcTrue );
	BoundRenderStateHash_ = 0;

	// Setup BB RT + DS.
	BackBufferRTDesc_ = RsTextureDesc( RsTextureType::TEX2D,
		RsResourceCreationFlags::STATIC,
		RsResourceBindFlags::RENDER_TARGET,
		RTFormat,
		1, 1, 1, 1 );
	BackBufferRT_ = new RsTexture( this, BackBufferRTDesc_ );
	new RsTextureGL( BackBufferRT_, RsTextureGL::ResourceType::BACKBUFFER_RT );
	++NoofTextures_;

	BackBufferDSDesc_ = RsTextureDesc( RsTextureType::TEX2D,
		RsResourceCreationFlags::STATIC,
		RsResourceBindFlags::DEPTH_STENCIL,
		DSFormat,
		1, 1, 1, 1 );
	BackBufferDS_ = new RsTexture( this, BackBufferDSDesc_ );
	new RsTextureGL( BackBufferDS_, RsTextureGL::ResourceType::BACKBUFFER_DS );
	++NoofTextures_;

	// Ensure all buffers are cleared to black first.
	const BcU32 Width = pClient_->getWidth();
	const BcU32 Height = pClient_->getHeight();
	for( BcU32 Idx = 0; Idx < 3; ++Idx )
	{
		beginFrame( Width, Height );
		GL( Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT ) );
		endFrame();
	}

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
#if !PLATFORM_ANDROID
	GL( BindVertexArray( 0 ) );
	GL( DeleteVertexArrays( 1, &GlobalVAO_ ) );
#endif

	destroyTexture( BackBufferRT_ );
	destroyTexture( BackBufferDS_ );

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
		if( Version.Major_ == 2 && Version.Minor_ == 0 )
		{
			ContextAttribs[ 1 ] = WGL_CONTEXT_ES2_PROFILE_BIT_EXT;
		}
		else
		{
			return false;
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
bool RsContextGL::createRenderState( RsRenderState* RenderState )
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
bool RsContextGL::destroyRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	unbindResource( RenderState );

	--NoofRenderStates_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextGL::createSamplerState( RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

#if !defined( RENDER_USE_GLES )
	if( Version_.SupportSamplerStates_ )
	{
		GLuint SamplerObject = (GLuint)-1;
		GL( GenSamplers( 1, &SamplerObject ) );
		

		// Setup sampler parmeters.
		const auto& SamplerStateDesc = SamplerState->getDesc();

		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_MIN_FILTER, RsUtilsGL::GetTextureFiltering( SamplerStateDesc.MinFilter_ ) ) );
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_MAG_FILTER, RsUtilsGL::GetTextureFiltering( SamplerStateDesc.MagFilter_ ) ) );
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_WRAP_S, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressU_ ) ) );
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_WRAP_T, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressV_ ) ) );	
		GL( SamplerParameteri( SamplerObject, GL_TEXTURE_WRAP_R, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressW_ ) ) );	
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
bool RsContextGL::destroySamplerState( RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	unbindResource( SamplerState );

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

	// Check if it's the backbuffer we're trying to create.
	if( Desc.RenderTargets_[ 0 ] == BackBufferRT_ || Desc.DepthStencilTarget_ == BackBufferDS_ ) 
	{
		if( Desc.RenderTargets_[ 0 ] != BackBufferRT_ || Desc.DepthStencilTarget_ != BackBufferDS_ )
		{
			return false;
		}
		for( size_t Idx = 1; Idx < Desc.RenderTargets_.size(); ++Idx )
		{
			if( Desc.RenderTargets_[ Idx ] != nullptr )
			{
				return false;
			}
		}
		FrameBuffer->setHandle( 0 );
		return true;
	}

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
			RsTextureGL* TextureGL = Texture->getHandle< RsTextureGL* >();
			GL( FramebufferTexture2D( 
				GL_FRAMEBUFFER, 
				GL_COLOR_ATTACHMENT0 + NoofAttachments,
				GL_TEXTURE_2D,
				TextureGL->getHandle(),
				0 ) );
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

		RsTextureGL* TextureGL = Desc.DepthStencilTarget_->getHandle< RsTextureGL* >();
		GL( FramebufferTexture2D( 
			GL_FRAMEBUFFER,
			Attachment,
			GL_TEXTURE_2D,
			TextureGL->getHandle(),
			0 ) );
	}

	// Check status.
	auto Status = GL( CheckFramebufferStatus( GL_FRAMEBUFFER ) );
	BcAssertMsg( Status == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete" );

	// Unbind.
	GL( BindFramebuffer( GL_FRAMEBUFFER, 0 ) );
	BoundFrameBuffer_ = nullptr;
	BoundViewport_  = RsViewport();
	BoundScissorRect_ = RsScissorRect( -1, -1, -1, -1 );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextGL::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	unbindResource( FrameBuffer );

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

	// Create buffer impl.
	new RsBufferGL( Buffer, Version_ );

	++NoofBuffers_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextGL::destroyBuffer( RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	unbindResource( Buffer );

	// Is buffer be in main memory?
	--NoofBuffers_;

	auto BufferGL = Buffer->getHandle< RsBufferGL* >();
	delete BufferGL;
	Buffer->setHandle< int >( 0 );

	return true;
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
	BcAssertMsg( BufferDesc.BindFlags_ != RsResourceBindFlags::NONE, "Buffer bind flags are unknown" );

	auto BufferGL = Buffer->getHandle< RsBufferGL* >();
	BcAssert( BufferGL );

	// If buffer is backed in main memory, use its own buffer for staging.
	BcU8* BufferData = BufferGL->getBufferData();
	std::unique_ptr< BcU8[] > LocalBufferData;
	if( BufferData == nullptr )
	{
		LocalBufferData.reset( new BcU8[ Size ] );
		BufferData = LocalBufferData.get();
	}

	// Call update func to fill data.
	RsBufferLock Lock =
	{
		BufferData + Offset
	};
	UpdateFunc( Buffer, Lock );

	// If the buffer has a handle, upload to GPU now.
	GLuint HandleGL = BufferGL->getHandle();
	if( HandleGL != 0 )
	{
		// Get buffer type for GL.
		auto TypeGL = RsUtilsGL::GetBufferType( BufferDesc.BindFlags_ );

		// Bind buffer.
		bindBuffer( TypeGL, Buffer, 0 );

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

		if( Size == BufferDesc.SizeBytes_ )
		{
			GL( BufferData( TypeGL, Size, BufferData, UsageFlagsGL ) );
		}
		else
		{
			GL( BufferSubData( TypeGL, Offset, Size, BufferData ) );
		}
	}

	// Increment version.
	BufferGL->incVersion();
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextGL::createTexture( class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Create GL texture.
	new RsTextureGL( Texture, RsTextureGL::ResourceType::TEXTURE );
	
	++NoofTextures_;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextGL::destroyTexture( class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	unbindResource( Texture );

	// Check that we haven't already freed it.
	RsTextureGL* TextureGL = Texture->getHandle< RsTextureGL* >();
	setHandle< int >( 0 );
	delete TextureGL;

	--NoofTextures_;
	return true;
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

	RsTextureGL* TextureGL = Texture->getHandle< RsTextureGL* >();

	const auto& TextureDesc = Texture->getDesc();

	if( TextureGL->getHandle() != 0 )
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
		BcU32 SlicePitch = RsTextureSlicePitch( 
			TextureDesc.Format_,
			Width,
			Height );
		BcU32 Pitch = RsTexturePitch( 
			TextureDesc.Format_,
			Width,
			Height );;
		RsTextureLock Lock = 
		{
			&Data[ 0 ],
			Pitch,
			SlicePitch
		};

		// Call update func.
		UpdateFunc( Texture, Lock );

		// Load slice.
		TextureGL->loadTexture( Slice, DataSize, &Data[ 0 ] );

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextGL::createShader( RsShader* Shader )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& Desc = Shader->getDesc();
	GLuint ShaderType = RsUtilsGL::GetShaderType( Desc.ShaderType_ );

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
	GLint ShaderCompiled = 0;
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
			auto PrintLine = Line++;
			if( ShaderLine.size() > 0 )
			{
				PSY_LOG( "%u: %s", PrintLine, ShaderLine.c_str() );
			}
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
bool RsContextGL::destroyShader( class RsShader* Shader )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	unbindResource( Shader );

	GLuint Handle = Shader->getHandle< GLuint >();
	GL( DeleteShader( Handle ) );

	--NoofShaders_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextGL::createProgram( RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Set handle.
	new RsProgramGL( Program, Version_ );

	++NoofPrograms_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextGL::destroyProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	unbindResource( Program );

	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
	delete ProgramGL;
	Program->setHandle( 0 );

	--NoofPrograms_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgramBinding
bool RsContextGL::createProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgramBinding
bool RsContextGL::destroyProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createGeometryBinding
bool RsContextGL::createGeometryBinding( class RsGeometryBinding* GeometryBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyGeometryBinding
bool RsContextGL::destroyGeometryBinding( class RsGeometryBinding* GeometryBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	unbindResource( GeometryBinding );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
bool RsContextGL::createVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyVertexDeclaration
bool RsContextGL::destroyVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	unbindResource( VertexDeclaration );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextGL::clear( 
	const RsFrameBuffer* FrameBuffer,
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::clear" );

	bindFrameBuffer( FrameBuffer, nullptr, nullptr );
	GL( ClearColor( Colour.r(), Colour.g(), Colour.b(), Colour.a() ) );
	
	// Disable scissor if we need to.
	auto& BoundRasteriserState = BoundRenderStateDesc_.RasteriserState_;
	if( BoundRasteriserState.ScissorEnable_ )
	{
		GL( Disable( GL_SCISSOR_TEST ) );
		BoundRasteriserState.ScissorEnable_ = BcFalse;
		BoundRenderStateHash_ = 0; // TODO: Use actual hash.
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
		BoundRenderStateHash_ = 0; // TODO: Use actual hash.
	}
	GL( Clear( 
		( EnableClearColour ? GL_COLOR_BUFFER_BIT : 0 ) | 
		( EnableClearDepth ? GL_DEPTH_BUFFER_BIT : 0 ) | 
		( EnableClearStencil ? GL_STENCIL_BUFFER_BIT : 0 ) ) );
	
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextGL::drawPrimitives( 
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding, 
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer,
		const RsViewport* Viewport,
		const RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, BcU32 VertexOffset, BcU32 NoofVertices )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::drawPrimitives" );
	++NoofDrawCalls_;

	const auto& ProgramBindingDesc = ProgramBinding->getDesc();
	const auto* Program = ProgramBinding->getProgram();
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
	bindProgram( Program );
	ProgramGL->copyUniformBuffersToUniforms( ProgramBindingDesc.UniformBuffers_.size(), ProgramBindingDesc.UniformBuffers_.data() );

	bindGeometry( Program, GeometryBinding, 0 );
	bindFrameBuffer( FrameBuffer, Viewport, ScissorRect );
	if( BoundRenderStateHash_ != RenderState->getHandle< BcU64 >() )
	{
		bindRenderStateDesc( RenderState->getDesc(), BcFalse );
		BoundRenderStateHash_ = RenderState->getHandle< BcU64 >();
	}
	if( BoundProgramBinding_ != ProgramBinding )
	{
		bindSRVs( Program, ProgramBindingDesc );
		bindSamplerStates( Program, ProgramBindingDesc );
		bindUniformBuffers( Program, ProgramBindingDesc );
		BoundProgramBinding_ = ProgramBinding;
	}
	// TODO: Add memory barrier to the binding object.
	bindUAVs( Program, ProgramBinding->getDesc(), MemoryBarrier_ );

	GL( DrawArrays( RsUtilsGL::GetTopologyType( TopologyType ), VertexOffset, NoofVertices ) );

#if !defined( RENDER_USE_GLES )
	if( MemoryBarrier_ )
	{
		GL( MemoryBarrier( MemoryBarrier_ ) );
		MemoryBarrier_ = 0;
	}
#endif // !defined( RENDER_USE_GLES )
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextGL::drawIndexedPrimitives( 
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding, 
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer,
		const RsViewport* Viewport,
		const RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::drawIndexedPrimitives" );
	++NoofDrawCalls_;

	const auto& ProgramBindingDesc = ProgramBinding->getDesc();
	const auto* Program = ProgramBinding->getProgram();
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
	bindProgram( Program );
	ProgramGL->copyUniformBuffersToUniforms( ProgramBindingDesc.UniformBuffers_.size(), ProgramBindingDesc.UniformBuffers_.data() );

	if( Version_.SupportDrawElementsBaseVertex_ )
	{
		bindGeometry( Program, GeometryBinding, 0 );
	}
	else
	{
		bindGeometry( Program, GeometryBinding, VertexOffset );
		VertexOffset = 0;
	}
	bindFrameBuffer( FrameBuffer, Viewport, ScissorRect );
	if( BoundRenderStateHash_ != RenderState->getHandle< BcU64 >() )
	{
		bindRenderStateDesc( RenderState->getDesc(), BcFalse );
		BoundRenderStateHash_ = RenderState->getHandle< BcU64 >();
	}
	if( BoundProgramBinding_ != ProgramBinding )
	{
		bindSRVs( Program, ProgramBindingDesc );
		bindSamplerStates( Program, ProgramBindingDesc );
		bindUniformBuffers( Program, ProgramBindingDesc );
		BoundProgramBinding_ = ProgramBinding;
	}
	// TODO: Add memory barrier to the binding object.
	bindUAVs( Program, ProgramBinding->getDesc(), MemoryBarrier_ );

	const auto GeometryBindingDesc = GeometryBinding->getDesc();

	GLenum IndexFormat = GL_UNSIGNED_SHORT;
	switch( GeometryBindingDesc.IndexBuffer_.Stride_ )
	{
	case 1:
		IndexFormat = GL_UNSIGNED_BYTE;
		break;
	case 2:
		IndexFormat = GL_UNSIGNED_SHORT;
		break;
	case 4:
		IndexFormat = GL_UNSIGNED_INT;
		break;
	default:
		BcAssertMsg( BcFalse, "Invalid index buffer stride specified: %u", GeometryBinding->getDesc().IndexBuffer_.Stride_ );
	}

	// Convert offset to bytes.
	IndexOffset *= GeometryBindingDesc.IndexBuffer_.Stride_;
	// Add byte offset from binding.
	IndexOffset += GeometryBindingDesc.IndexBuffer_.Offset_;

	BcAssert( GeometryBinding->getDesc().IndexBuffer_.Buffer_ );
	BcAssert( ( IndexOffset + ( NoofIndices * GeometryBindingDesc.IndexBuffer_.Stride_ ) ) <= GeometryBindingDesc.IndexBuffer_.Buffer_->getDesc().SizeBytes_ );

	if( VertexOffset == 0 )
	{
		GL( DrawElements( RsUtilsGL::GetTopologyType( TopologyType ), NoofIndices, IndexFormat, (void*)( IndexOffset ) ) );
	}
#if !defined( RENDER_USE_GLES )
	else if( Version_.SupportDrawElementsBaseVertex_ )
	{
		GL( DrawElementsBaseVertex( RsUtilsGL::GetTopologyType( TopologyType ), NoofIndices, IndexFormat, (void*)( IndexOffset ), VertexOffset ) );
	}
#endif
	else
	{
		BcBreakpoint;
	}

#if !defined( RENDER_USE_GLES )
	if( MemoryBarrier_ )
	{
		GL( MemoryBarrier( MemoryBarrier_ ) );
		MemoryBarrier_ = 0;
	}
#endif // !defined( RENDER_USE_GLES )	
}

//////////////////////////////////////////////////////////////////////////
// copyFrameBufferRenderTargetToTexture
void RsContextGL::copyFrameBufferRenderTargetToTexture( RsFrameBuffer* FrameBuffer, BcU32 Idx, RsTexture* Texture )
{
	PSY_PROFILE_FUNCTION;

#if !defined( RENDER_USE_GLES )
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

	// Set read buffer
	GL( ReadBuffer( GL_COLOR_ATTACHMENT0 + Idx ) );

	const auto& TextureDesc = Texture->getDesc();
	auto TypeGL = RsUtilsGL::GetTextureType( TextureDesc.Type_ );
	const auto& FormatGL = RsUtilsGL::GetTextureFormat( TextureDesc.Format_ );

	RsTextureGL* DestTextureGL = Texture->getHandle< RsTextureGL* >();
	if( DestTextureGL->getHandle() != 0 )
	{
		// Bind texture.
		bindTexture( 0, Texture );
		GL( CopyTexImage2D( 
			TypeGL, 
			0, 
			FormatGL.InternalFormat_,
			0,
			0,
			TextureDesc.Width_,
			TextureDesc.Height_,
			0 ) );
		
		auto& BindingInfo = TextureBindingInfo_[ 0 ];
		BindingInfo.Texture_ = 0;
		BindingInfo.Target_ = TypeGL;
	}
	else
	{
		BcBreakpoint;
	}
#endif // !defined( RENDER_USE_GLES )
}

//////////////////////////////////////////////////////////////////////////
// copyTextureToFrameBufferRenderTarget
void RsContextGL::copyTextureToFrameBufferRenderTarget( RsTexture* Texture, RsFrameBuffer* FrameBuffer, BcU32 Idx )
{
	PSY_PROFILE_FUNCTION;

#if !defined( RENDER_USE_GLES )
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

	RsTextureGL* SrcTextureGL = Texture->getHandle< RsTextureGL* >();

	GL( BindFramebuffer( GL_READ_FRAMEBUFFER, TransferFBOs_[ 0 ] ) );
	GL( FramebufferTexture2D( 
		GL_READ_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D,
		SrcTextureGL->getHandle(),
		0 ) );
	auto ReadStatus = GL( CheckFramebufferStatus( GL_READ_FRAMEBUFFER ) );
	BcAssertMsg( ReadStatus == GL_FRAMEBUFFER_COMPLETE, "Framebuffer not complete" );

	if( FrameBuffer != nullptr )
	{
		RsTextureGL* DestTextureGL = 
			FrameBuffer->getDesc().RenderTargets_[ Idx ]->getHandle< RsTextureGL* >();
		GL( BindFramebuffer( GL_DRAW_FRAMEBUFFER, TransferFBOs_[ 1 ] ) );
		GL( FramebufferTexture2D( 
			GL_DRAW_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			DestTextureGL->getHandle(),
			0 ) );
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
	BoundFrameBuffer_ = nullptr;
	BoundViewport_  = RsViewport();
	BoundScissorRect_ = RsScissorRect( -1, -1, -1, -1 );
#endif // !defined( RENDER_USE_GLES )
}

//////////////////////////////////////////////////////////////////////////
// dispatchCompute
void RsContextGL::dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups )
{
	PSY_PROFILE_FUNCTION;

#if !defined( RENDER_USE_GLES )

	const auto& ProgramBindingDesc = ProgramBinding->getDesc();
	const auto* Program = ProgramBinding->getProgram();
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();

	bindProgram( Program );
	ProgramGL->copyUniformBuffersToUniforms( ProgramBindingDesc.UniformBuffers_.size(), ProgramBindingDesc.UniformBuffers_.data() );
	if( BoundProgramBinding_ != ProgramBinding )
	{
		BoundProgramBinding_ = ProgramBinding;
		bindSRVs( Program, ProgramBindingDesc );
		bindSamplerStates( Program, ProgramBindingDesc );
		bindUniformBuffers( Program, ProgramBindingDesc );
	}
	// TODO: Add memory barrier to the binding object.
	bindUAVs( ProgramBinding->getProgram(), ProgramBinding->getDesc(), MemoryBarrier_ );

	GL( DispatchCompute( XGroups, YGroups, ZGroups ) );

	if( MemoryBarrier_ )
	{
		GL( MemoryBarrier( MemoryBarrier_ ) );
		MemoryBarrier_ = 0;
	}
#endif // !defined( RENDER_USE_GLES )
}

//////////////////////////////////////////////////////////////////////////
// bindFrameBuffer
void RsContextGL::bindFrameBuffer( const RsFrameBuffer* FrameBuffer, const RsViewport* Viewport, const RsScissorRect* ScissorRect )
{
	PSY_PROFILE_FUNCTION;
	BcAssert( FrameBuffer );

	// Determine frame buffer width + height.
	auto RT = FrameBuffer->getDesc().RenderTargets_[ 0 ];
	BcAssert( RT );
	auto FBWidth = RT->getDesc().Width_;
	auto FBHeight = RT->getDesc().Height_;

	RsViewport FullViewport( 0, 0, FBWidth, FBHeight );
	RsScissorRect FullScissorRect( 0, 0, FBWidth, FBHeight );

	BcAssert( FBWidth > 0 );
	BcAssert( FBHeight > 0 );

	if( BoundFrameBuffer_ != FrameBuffer )
	{
		GLint Handle = FrameBuffer->getHandle< GLint >();
		GL( BindFramebuffer( GL_FRAMEBUFFER, Handle ) );
		BoundFrameBuffer_ = FrameBuffer;
		BoundViewport_  = RsViewport();
		BoundScissorRect_ = RsScissorRect( -1, -1, -1, -1 );
	}

	// Setup viewport if null.
	Viewport = Viewport != nullptr ? Viewport : &FullViewport;

	// Setup scissor rect if null.
	ScissorRect = ScissorRect != nullptr ? ScissorRect : &FullScissorRect;

	if( BoundViewport_ != *Viewport )
	{
		const auto VX = Viewport->x();
		const auto VY = FBHeight - Viewport->height();
		const auto VW = Viewport->width() - Viewport->x();
		const auto VH = Viewport->height() - Viewport->y();
		GL( Viewport( VX, VY, VW, VH ) );
		BoundViewport_ = *Viewport;
	}

	if( BoundScissorRect_ != *ScissorRect )
	{
		const auto SX = ScissorRect->X_;
		const auto SY = FBHeight - ( ScissorRect->Height_ + ScissorRect->Y_ );
		const auto SW = ScissorRect->Width_;
		const auto SH = ScissorRect->Height_;
		GL( Scissor( SX, SY, SW, SH ) );
		BoundScissorRect_ = *ScissorRect;
	}
}

//////////////////////////////////////////////////////////////////////////
// bindProgram
void RsContextGL::bindProgram( const RsProgram* Program )
{
	PSY_PROFILE_FUNCTION;

	if( BoundProgram_ != Program )
	{
		BoundProgram_ = Program;
		BoundProgramBinding_ = nullptr;
		BoundGeometryBinding_ = nullptr;
		RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
		GL( UseProgram( ProgramGL->getHandle() ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// bindGeometry
void RsContextGL::bindGeometry( const RsProgram* Program, const RsGeometryBinding* GeometryBinding, BcU32 VertexOffset )
{
	PSY_PROFILE_FUNCTION;

	if( BoundGeometryBinding_ != GeometryBinding || 
		BoundVertexOffset_ != VertexOffset )
	{
		BoundGeometryBinding_ = GeometryBinding;
		BoundVertexOffset_ = VertexOffset;
		const auto& Desc = GeometryBinding->getDesc();

		const auto& ProgramVertexAttributeList = Program->getVertexAttributeList();
		const auto& VertexDeclarationDesc = Desc.VertexDeclaration_->getDesc();
		const auto& PrimitiveVertexElementList = VertexDeclarationDesc.Elements_;

		// Cached vertex handle for binding.
		GLuint BoundVertexHandle = 0;

		// Reset next state + disable all vertex attribs.
		BcMemZero( &VertexBufferActiveNextState_[ 0 ], sizeof( VertexBufferActiveNextState_ ) );

		// Bind up all elements to attributes.
		BcU32 BoundElements = 0;
		for( const auto& Attribute : ProgramVertexAttributeList )
		{
			auto FoundElement = std::find_if( PrimitiveVertexElementList.begin(), PrimitiveVertexElementList.end(),
				[ &Attribute ]( const RsVertexElement& Element )
				{
					return ( Element.Usage_ == Attribute.Usage_ &&
						Element.UsageIdx_ == Attribute.UsageIdx_ );
				} );

			// Force to an element with zero offset if we can't find a valid one.
			// TODO: Find a better approach.
			if( FoundElement == PrimitiveVertexElementList.end() )
			{
				FoundElement = std::find_if( PrimitiveVertexElementList.begin(), PrimitiveVertexElementList.end(),
					[]( const RsVertexElement& Element )
					{
						return Element.Offset_ == 0;
					} );
			}

			// Found an element we can bind to.
			if( FoundElement != PrimitiveVertexElementList.end() )
			{
				auto VertexBufferBinding = Desc.VertexBuffers_[ FoundElement->StreamIdx_ ];
				auto VertexBuffer = VertexBufferBinding.Buffer_;
				auto VertexStride = VertexBufferBinding.Stride_;
				auto FullVertexOffset = ( VertexOffset * VertexStride ) + VertexBufferBinding.Offset_;

				// Bind up new vertex buffer if we need to.
				BcAssertMsg( FoundElement->StreamIdx_ < Desc.VertexBuffers_.size(), "Stream index out of bounds for primitive." );
				BcAssertMsg( VertexBuffer != nullptr, "Vertex buffer not bound!" );
				auto VertexBufferGL = VertexBuffer->getHandle< RsBufferGL* >();
				BcAssert( VertexBufferGL );
				GLuint VertexHandle = VertexBufferGL->getHandle();
				if( BoundVertexHandle != VertexHandle )
				{
					bindBuffer( GL_ARRAY_BUFFER, VertexBuffer, 0 );
					BoundVertexHandle = VertexHandle;
				}

				// Enable array.
				VertexBufferActiveNextState_[ Attribute.Channel_ ] = true;

				// Bind.
				BcU64 CalcOffset = FoundElement->Offset_ + FullVertexOffset;

				GL( VertexAttribPointer( Attribute.Channel_, 
					FoundElement->Components_,
					RsUtilsGL::GetVertexDataType( FoundElement->DataType_ ),
					RsUtilsGL::GetVertexDataNormalised( FoundElement->DataType_ ),
					VertexStride,
					(GLvoid*)CalcOffset ) );
				++BoundElements;
			}
		}
		BcAssert( ProgramVertexAttributeList.size() == BoundElements );

		// Enable/disable states.
		for( BcU32 Idx = 0; Idx < MAX_VERTEX_STREAMS; ++Idx )
		{
			if( VertexBufferActiveState_[ Idx ] != VertexBufferActiveNextState_[ Idx ] )
			{
				VertexBufferActiveState_[ Idx ] = VertexBufferActiveNextState_[ Idx ];
				if( VertexBufferActiveState_[ Idx ] )
				{
					GL( EnableVertexAttribArray( Idx ) );
				}
				else
				{
					GL( DisableVertexAttribArray( Idx ) );
				}
			}
		}

		// Bind indices.
		bindBuffer( GL_ELEMENT_ARRAY_BUFFER, Desc.IndexBuffer_.Buffer_, 0 );
	}
}

////////////////////////////////////////////////////////////////////////////////
// bindRenderStateDesc
void RsContextGL::bindRenderStateDesc( const RsRenderStateDesc& Desc, BcBool Force )
{
	PSY_PROFILE_FUNCTION;

#if !defined( RENDER_USE_GLES )
	if( Version_.Features_.SeparateBlendState_ )
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
					RsUtilsGL::GetBlendOp( RenderTarget.BlendOp_ ), 
					RsUtilsGL::GetBlendOp( RenderTarget.BlendOpAlpha_ ) ) );
			}

			if( Force ||
				RenderTarget.SrcBlend_ != BoundRenderTarget.SrcBlend_ ||
				RenderTarget.DestBlend_ != BoundRenderTarget.DestBlend_ ||
				RenderTarget.SrcBlendAlpha_ != BoundRenderTarget.SrcBlendAlpha_ ||
				RenderTarget.DestBlendAlpha_ != BoundRenderTarget.DestBlendAlpha_ )
			{
				GL( BlendFuncSeparatei( 
					Idx, 
					RsUtilsGL::GetBlendType( RenderTarget.SrcBlend_ ), RsUtilsGL::GetBlendType( RenderTarget.DestBlend_ ),
					RsUtilsGL::GetBlendType( RenderTarget.SrcBlendAlpha_ ), RsUtilsGL::GetBlendType( RenderTarget.DestBlendAlpha_ ) ) );
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
				RsUtilsGL::GetBlendOp( MainRenderTarget.BlendOp_ ), 
				RsUtilsGL::GetBlendOp( MainRenderTarget.BlendOpAlpha_ ) ) );
		}

		if( Force ||
			MainRenderTarget.SrcBlend_ != BoundMainRenderTarget.SrcBlend_ ||
			MainRenderTarget.DestBlend_ != BoundMainRenderTarget.DestBlend_ ||
			MainRenderTarget.SrcBlendAlpha_ != BoundMainRenderTarget.SrcBlendAlpha_ ||
			MainRenderTarget.DestBlendAlpha_ != BoundMainRenderTarget.DestBlendAlpha_ )
		{
			GL( BlendFuncSeparate( 
				RsUtilsGL::GetBlendType( MainRenderTarget.SrcBlend_ ), RsUtilsGL::GetBlendType( MainRenderTarget.DestBlend_ ),
				RsUtilsGL::GetBlendType( MainRenderTarget.SrcBlendAlpha_ ), RsUtilsGL::GetBlendType( MainRenderTarget.DestBlendAlpha_ ) ) );
		}

		if( Version_.Features_.MRT_ )
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
		GL( DepthFunc( RsUtilsGL::GetCompareMode( DepthStencilState.DepthFunc_ ) ) );
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
			RsUtilsGL::GetCompareMode( DepthStencilState.StencilFront_.Func_ ), 
			DepthStencilState.StencilRef_, DepthStencilState.StencilFront_.Mask_ ) );
	}

	if( Force ||
		DepthStencilState.StencilBack_.Func_ != BoundDepthStencilState.StencilBack_.Func_ ||
		DepthStencilState.StencilRef_ != BoundDepthStencilState.StencilRef_ ||
		DepthStencilState.StencilBack_.Mask_ != BoundDepthStencilState.StencilBack_.Mask_ )
	{
		GL( StencilFuncSeparate( 
			GL_BACK,
			RsUtilsGL::GetCompareMode( DepthStencilState.StencilBack_.Func_ ), 
			DepthStencilState.StencilRef_, DepthStencilState.StencilBack_.Mask_ ) );
	}

	if( Force ||
		DepthStencilState.StencilFront_.Fail_ != BoundDepthStencilState.StencilFront_.Fail_ ||
		DepthStencilState.StencilFront_.DepthFail_ != BoundDepthStencilState.StencilFront_.DepthFail_ ||
		DepthStencilState.StencilFront_.Pass_ != BoundDepthStencilState.StencilFront_.Pass_ )
	{
		GL( StencilOpSeparate( 
			GL_FRONT,
			RsUtilsGL::GetStencilOp( DepthStencilState.StencilFront_.Fail_ ), 
			RsUtilsGL::GetStencilOp( DepthStencilState.StencilFront_.DepthFail_ ), 
			RsUtilsGL::GetStencilOp( DepthStencilState.StencilFront_.Pass_ ) ) );
	}

	if( Force ||
		DepthStencilState.StencilBack_.Fail_ != BoundDepthStencilState.StencilBack_.Fail_ ||
		DepthStencilState.StencilBack_.DepthFail_ != BoundDepthStencilState.StencilBack_.DepthFail_ ||
		DepthStencilState.StencilBack_.Pass_ != BoundDepthStencilState.StencilBack_.Pass_ )
	{
		GL( StencilOpSeparate( 
			GL_BACK,
			RsUtilsGL::GetStencilOp( DepthStencilState.StencilBack_.Fail_ ), 
			RsUtilsGL::GetStencilOp( DepthStencilState.StencilBack_.DepthFail_ ), 
			RsUtilsGL::GetStencilOp( DepthStencilState.StencilBack_.Pass_ ) ) );
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
	if( Version_.Features_.AntialiasedLines_ )
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

//////////////////////////////////////////////////////////////////////////
// bindSRVs
void RsContextGL::bindSRVs( const RsProgram* Program, const RsProgramBindingDesc& Bindings )
{
	PSY_PROFILE_FUNCTION;
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
#if PSY_DEBUG
	GLint BoundProgram = 0;
	GL( GetIntegerv( GL_CURRENT_PROGRAM, &BoundProgram ) );
	BcAssert( (GLuint)BoundProgram ==  ProgramGL->getHandle() );
#endif

	for( BcU32 Idx = 0; Idx < ProgramGL->getSRVBindCount(); ++Idx )
	{
		auto& SRVSlot = Bindings.ShaderResourceSlots_[ Idx ];
		if( SRVSlot.Resource_ )
		{
			const auto& SRVSlotGL = ProgramGL->getSRVBindInfo( Idx );
			switch( SRVSlotGL.BindType_ )
			{
			case RsProgramBindTypeGL::NONE:
				break;
			case RsProgramBindTypeGL::TEXTURE:
				{
					// TODO: Redundant state checking.
					BcAssert( ( SRVSlot.Texture_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
					bindTexture( SRVSlotGL.Slot_, SRVSlot.Texture_ );
				}
				break;
#if !defined( RENDER_USE_GLES )	
			case RsProgramBindTypeGL::IMAGE:
				{
					// TODO: Redundant state checking.
					BcAssert( ( SRVSlot.Texture_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
					RsTextureGL* TextureGL = SRVSlot.Texture_->getHandle< RsTextureGL* >();
					const auto Format = RsUtilsGL::GetImageFormat( SRVSlot.Texture_->getDesc().Format_ );
					auto& BindingInfo = ImageBindingInfo_[ SRVSlotGL.Slot_ ];
					if( BindingInfo.Resource_ != SRVSlot.Resource_ ||
						BindingInfo.Texture_ != TextureGL->getHandle() ||
						BindingInfo.Level_ != 0 ||
						BindingInfo.Layered_ != GL_FALSE ||
						BindingInfo.Layer_ != 0 ||
						BindingInfo.Access_ != GL_READ_ONLY ||
						BindingInfo.Format_ != Format )
					{
						GL( BindImageTexture( SRVSlotGL.Slot_, TextureGL->getHandle(),
							0, GL_FALSE, 0, GL_READ_ONLY,
							Format ) );
						BindingInfo.Resource_ = SRVSlot.Resource_;
						BindingInfo.Texture_ = TextureGL->getHandle();
						BindingInfo.Level_ = 0;
						BindingInfo.Layered_ = GL_FALSE;
						BindingInfo.Layer_ = 0;
						BindingInfo.Access_ = GL_READ_ONLY;
						BindingInfo.Format_ = Format;
					}
				};
				break;
			case RsProgramBindTypeGL::SHADER_STORAGE_BUFFER_OBJECT:
				{
					// TODO: Redundant state checking.
					BcAssert( ( SRVSlot.Buffer_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
					RsBufferGL* BufferGL = SRVSlot.Buffer_->getHandle< RsBufferGL* >();
					auto& BindingInfo = ShaderStorageBufferBindingInfo_[ SRVSlotGL.Slot_ ];
					if( BindingInfo.Resource_ != SRVSlot.Resource_ ||
						BindingInfo.Buffer_ != BufferGL->getHandle() ||
						BindingInfo.Offset_ != 0 ||
						BindingInfo.Size_ != SRVSlot.Buffer_->getDesc().SizeBytes_ )
					{
						GL( BindBufferBase( GL_SHADER_STORAGE_BUFFER, SRVSlotGL.Slot_, BufferGL->getHandle() ) );
						BindingInfo.Resource_ = SRVSlot.Resource_;
						BindingInfo.Buffer_ = BufferGL->getHandle();
						BindingInfo.Offset_ = 0;
						BindingInfo.Size_ = (GLsizei)SRVSlot.Buffer_->getDesc().SizeBytes_;
					}
				}
				break;
#endif // !defined( RENDER_USE_GLES )	
			default:
				BcBreakpoint;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// bindUAVs
void RsContextGL::bindUAVs( const RsProgram* Program, const RsProgramBindingDesc& Bindings, GLbitfield& Barrier )
{
	PSY_PROFILE_FUNCTION;
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
#if PSY_DEBUG
	GLint BoundProgram = 0;
	GL( GetIntegerv( GL_CURRENT_PROGRAM, &BoundProgram ) );
	BcAssert( (GLuint)BoundProgram ==  ProgramGL->getHandle() );
#endif

#if !defined( RENDER_USE_GLES )	
	for( BcU32 Idx = 0; Idx < ProgramGL->getUAVBindCount(); ++Idx )
	{
		auto& UAVSlot = Bindings.UnorderedAccessSlots_[ Idx ];
		if( UAVSlot.Resource_ )
		{
			const auto& UAVSlotGL = ProgramGL->getUAVBindInfo( Idx );
			switch( UAVSlotGL.BindType_ )
			{
			case RsProgramBindTypeGL::NONE:
				break;
			case RsProgramBindTypeGL::IMAGE:
				{
					// TODO: Redundant state checking.
					BcAssert( ( UAVSlot.Texture_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
					RsTextureGL* TextureGL = UAVSlot.Texture_->getHandle< RsTextureGL* >(); 
					const auto Format = RsUtilsGL::GetImageFormat( UAVSlot.Texture_->getDesc().Format_ );
					auto& BindingInfo = ImageBindingInfo_[ UAVSlotGL.Slot_ ];
					if( BindingInfo.Resource_ != UAVSlot.Resource_ ||
						BindingInfo.Texture_ != TextureGL->getHandle() ||
						BindingInfo.Level_ != 0 ||
						BindingInfo.Layered_ != GL_FALSE ||
						BindingInfo.Layer_ != 0 ||
						BindingInfo.Access_ != GL_READ_WRITE||
						BindingInfo.Format_ != Format )
					{
						GL( BindImageTexture( UAVSlotGL.Slot_, TextureGL->getHandle(),
							0, GL_FALSE, 0, GL_READ_WRITE,
							Format ) );
						BindingInfo.Resource_ = UAVSlot.Resource_;
						BindingInfo.Texture_ = TextureGL->getHandle();
						BindingInfo.Level_ = 0;
						BindingInfo.Layered_ = GL_FALSE;
						BindingInfo.Layer_ = 0;
						BindingInfo.Access_ = GL_READ_WRITE;
						BindingInfo.Format_ = Format;
						// TODO: Other params...
					}
					Barrier |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
				};
				break;
			case RsProgramBindTypeGL::SHADER_STORAGE_BUFFER_OBJECT:
				{
					// TODO: Redundant state checking.
					BcAssert( ( UAVSlot.Buffer_->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
					RsBufferGL* BufferGL = UAVSlot.Buffer_->getHandle< RsBufferGL* >(); 
					auto& BindingInfo = ShaderStorageBufferBindingInfo_[ UAVSlotGL.Slot_ ];
					if( BindingInfo.Resource_ != UAVSlot.Resource_ ||
						BindingInfo.Buffer_ != BufferGL->getHandle() ||
						BindingInfo.Offset_ != 0 ||
						BindingInfo.Size_ != (GLsizei)UAVSlot.Buffer_->getDesc().SizeBytes_ )
					{
						GL( BindBufferBase( GL_SHADER_STORAGE_BUFFER, UAVSlotGL.Slot_, BufferGL->getHandle() ) );
						BindingInfo.Resource_ = UAVSlot.Resource_;
						BindingInfo.Buffer_ = BufferGL->getHandle();
						BindingInfo.Offset_ = 0;
						BindingInfo.Size_ = (GLsizei)UAVSlot.Buffer_->getDesc().SizeBytes_;
					}
					Barrier |= GL_SHADER_STORAGE_BARRIER_BIT;
				}
				break;
			default:
				BcBreakpoint;
			}
		}
	}
#endif // !defined( RENDER_USE_GLES )	
}

//////////////////////////////////////////////////////////////////////////
// bindSamplerStates
void RsContextGL::bindSamplerStates( const RsProgram* Program, const RsProgramBindingDesc& Bindings )
{
	PSY_PROFILE_FUNCTION;
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
#if PSY_DEBUG
	GLint BoundProgram = 0;
	GL( GetIntegerv( GL_CURRENT_PROGRAM, &BoundProgram ) );
	BcAssert( (GLuint)BoundProgram ==  ProgramGL->getHandle() );
#endif

	for( BcU32 Idx = 0; Idx < ProgramGL->getSamplerBindCount(); ++Idx )
	{
		const auto& SamplerState = Bindings.SamplerStates_[ Idx ];
		if( SamplerState )
		{
			const auto& SamplerStateSlotGL = ProgramGL->getSamplerBindInfo( Idx );

#if !defined( RENDER_USE_GLES )
			if( Version_.SupportSamplerStates_ )
			{
				// TODO: Redundant state checking.
				auto& BindingInfo = SamplerBindingInfo_[ SamplerStateSlotGL.Slot_ ];
				GLuint SamplerObject = SamplerState->getHandle< GLuint >();
				if( BindingInfo.Sampler_ != SamplerObject )
				{
					GL( BindSampler( SamplerStateSlotGL.Slot_, SamplerObject ) );
					BindingInfo.Resource_ = SamplerState;
					BindingInfo.Sampler_ = SamplerObject;
				}
			}
			else
#endif // !defined( RENDER_USE_GLES )
			{
				// TODO: Redundant state checking on currently bound texture.

				// TODO MipLODBias_
				// TODO MaxAnisotropy_
				// TODO BorderColour_
				// TODO MinLOD_
				// TODO MaxLOD_
				const auto& SamplerStateDesc = SamplerState->getDesc();
				const GLenum TextureType = RsUtilsGL::GetTextureType( SamplerStateSlotGL.TextureType_ );
				GL( ActiveTexture( GL_TEXTURE0 + SamplerStateSlotGL.Slot_ ) );
				GL( TexParameteri( TextureType, GL_TEXTURE_MIN_FILTER, RsUtilsGL::GetTextureFiltering( SamplerStateDesc.MinFilter_ ) ) );
				GL( TexParameteri( TextureType, GL_TEXTURE_MAG_FILTER, RsUtilsGL::GetTextureFiltering( SamplerStateDesc.MagFilter_ ) ) );
				GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_S, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressU_ ) ) );
				GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_T, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressV_ ) ) );	
#if !defined( RENDER_USE_GLES )
				GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_R, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressW_ ) ) );	
				GL( TexParameteri( TextureType, GL_TEXTURE_COMPARE_MODE, GL_NONE ) );
				GL( TexParameteri( TextureType, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ) );
#endif // !defined( RENDER_USE_GLES )
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// bindUniformBuffers
void RsContextGL::bindUniformBuffers( const RsProgram* Program, const RsProgramBindingDesc& Bindings )
{
	PSY_PROFILE_FUNCTION;
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
#if PSY_DEBUG
	GLint BoundProgram = 0;
	GL( GetIntegerv( GL_CURRENT_PROGRAM, &BoundProgram ) );
	BcAssert( (GLuint)BoundProgram ==  ProgramGL->getHandle() );
#endif

#if !defined( RENDER_USE_GLES )
	if( Version_.SupportUniformBuffers_ )
	{
		for( BcU32 Idx = 0; Idx < ProgramGL->getUniformBufferBindCount(); ++Idx )
		{
			const auto& UniformBuffer = Bindings.UniformBuffers_[ Idx ];
			if( UniformBuffer )
			{
				// TODO: Redundant state checking.
				const auto& UniformBufferSlotGL = ProgramGL->getUniformBufferBindInfo( Idx );
				RsBufferGL* BufferGL = UniformBuffer->getHandle< RsBufferGL* >();
				auto& BindingInfo = UniformBufferBindingInfo_[ UniformBufferSlotGL.Slot_ ];
				if( BindingInfo.Resource_ != UniformBuffer ||
					BindingInfo.Buffer_ != BufferGL->getHandle() ||
					BindingInfo.Offset_ != 0 ||
					BindingInfo.Size_ != (GLsizei)UniformBuffer->getDesc().SizeBytes_ )
				{
					GL( BindBufferRange( GL_UNIFORM_BUFFER, UniformBufferSlotGL.Slot_, BufferGL->getHandle(), 0, UniformBuffer->getDesc().SizeBytes_ ) );
					BindingInfo.Resource_ = UniformBuffer;
					BindingInfo.Buffer_ = BufferGL->getHandle();
					BindingInfo.Offset_ = 0;
					BindingInfo.Size_ = (GLsizei)UniformBuffer->getDesc().SizeBytes_;
				}
			}
		}
	}
#endif // !defined( RENDER_USE_GLES )
}

//////////////////////////////////////////////////////////////////////////
// bindTexture
void RsContextGL::bindTexture( BcU32 Slot, const RsTexture* Texture )
{
	const GLenum TextureType = RsUtilsGL::GetTextureType( Texture->getDesc().Type_ );
	const RsTextureGL* TextureGL = Texture->getHandle< RsTextureGL* >();
	auto& BindingInfo = TextureBindingInfo_[ Slot ];
	if( BindingInfo.Resource_ != Texture ||
		BindingInfo.Texture_ != TextureGL->getHandle() ||
		BindingInfo.Target_ != TextureType )
	{
		GL( ActiveTexture( GL_TEXTURE0 + Slot ) );
		GL( BindTexture( TextureType, TextureGL->getHandle() ) );
		BindingInfo.Resource_ = Texture;
		BindingInfo.Texture_ = TextureGL->getHandle();
		BindingInfo.Target_ = TextureType;
	}

	// Binding of slot 0 textures mean we potentially need to rebind program bindings.
	if( Slot == 0 )
	{
		BoundProgramBinding_ = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
// bindBuffer
void RsContextGL::bindBuffer( GLenum BindTypeGL, const RsBuffer* Buffer, BcU32 Offset )
{
	switch( BindTypeGL )
	{
	case GL_ARRAY_BUFFER:
		{
			BcAssert( Offset == 0 );
			bindBufferInternal( VertexBufferBindingInfo_, GL_ARRAY_BUFFER, Buffer, 0 );
		}
		break;
	case GL_ELEMENT_ARRAY_BUFFER:
		{
			BcAssert( Offset == 0 );
			bindBufferInternal( IndexBufferBindingInfo_, GL_ELEMENT_ARRAY_BUFFER, Buffer, 0 );
		}
		break;
	case GL_UNIFORM_BUFFER:
		{
			bindBufferInternal( UniformBufferBindingInfo_[0], GL_UNIFORM_BUFFER, Buffer, Offset );

			// Binding of uniform buffers mean we potentially need to rebind program bindings.
			BoundProgramBinding_ = nullptr;
		}
		break;
	default:
		BcBreakpoint;
	}
}

//////////////////////////////////////////////////////////////////////////
// bindBufferInternal
void RsContextGL::bindBufferInternal( BufferBindingInfo& BindingInfo, GLenum BindTypeGL, const RsBuffer* Buffer, BcU32 Offset )
{
	RsBufferGL* BufferGL = Buffer ? Buffer->getHandle< RsBufferGL* >() : nullptr;
	const auto Size = Buffer ? Buffer->getDesc().SizeBytes_ : 0;
	const auto BindSize = Size - Offset;
	BcAssert( Offset == 0 || Offset < Size );
	auto Handle = BufferGL ? BufferGL->getHandle() : 0;
	if( BindingInfo.Resource_ != Buffer ||
		BindingInfo.Buffer_ != Handle ||
		BindingInfo.Offset_ != Offset ||
		BindingInfo.Size_ != 0 )
	{
		if( Offset == 0 )
		{
			GL( BindBuffer( BindTypeGL, Handle ) );
		}
		else
		{
			GL( BindBufferRange( BindTypeGL, 0, Handle, Offset, BindSize ) );
		}
		BindingInfo.Resource_ = Buffer;
		BindingInfo.Buffer_ = Handle;
		BindingInfo.Offset_ = Offset;
		BindingInfo.Size_ = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// unbindResource
void RsContextGL::unbindResource( const RsResource* Resource )
{
	for( auto& BindingInfo : TextureBindingInfo_ )
	{
		if( BindingInfo.Resource_ == Resource )
		{
			BindingInfo = TextureBindingInfo();
		}
	}

	for( auto& BindingInfo : ImageBindingInfo_ )
	{
		if( BindingInfo.Resource_ == Resource )
		{
			BindingInfo = ImageBindingInfo();
		}
	}

	for( auto& BindingInfo : ShaderStorageBufferBindingInfo_ )
	{
		if( BindingInfo.Resource_ == Resource )
		{
			BindingInfo = BufferBindingInfo();
		}
	}

	for( auto& BindingInfo : UniformBufferBindingInfo_ )
	{
		if( BindingInfo.Resource_ == Resource )
		{
			BindingInfo = BufferBindingInfo();
		}
	}

	for( auto& BindingInfo : SamplerBindingInfo_ )
	{
		if( BindingInfo.Resource_ == Resource )
		{
			BindingInfo = SamplerBindingInfo();
		}
	}

	if( IndexBufferBindingInfo_.Resource_ == Resource )
	{
		bindBuffer( GL_ELEMENT_ARRAY_BUFFER, nullptr, 0 );
	}

	if( VertexBufferBindingInfo_.Resource_ == Resource )
	{
		bindBuffer( GL_ARRAY_BUFFER, nullptr, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// getOpenGLVersion
const RsOpenGLVersion& RsContextGL::getOpenGLVersion() const
{
	return Version_;
}
