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
	FrameBufferDirty_( BcTrue ),
	FrameBuffer_( nullptr ),
	DirtyViewport_( BcTrue ),
	Viewport_( 0, 0, 0, 0 ),
	DirtyScissor_( BcTrue ),
	ScissorX_( 0 ),
	ScissorY_( 0 ),
	ScissorW_( 0 ),
	ScissorH_( 0 ),
	GlobalVAO_( 0 ),
	VertexDeclaration_( nullptr ),
	Program_( nullptr ),
	ProgramDirty_( BcTrue ),
	IndexBuffer_( nullptr ),
	IndexBufferDirty_( BcTrue ),
	VertexBuffersDirty_( BcTrue ),
	UniformBuffersDirty_( BcTrue )
{
	BcMemZero( &TextureStateValues_[ 0 ], sizeof( TextureStateValues_ ) );
	BcMemZero( &TextureStateBinds_[ 0 ], sizeof( TextureStateBinds_ ) );
	BcMemZero( &VertexBuffers_[ 0 ], sizeof( VertexBuffers_ ) );
	BcMemZero( &VertexBufferActiveState_[ 0 ], sizeof( VertexBufferActiveState_ ) );
	BcMemZero( &VertexBufferActiveNextState_[ 0 ], sizeof( VertexBufferActiveNextState_ ) );
	BcMemZero( &UniformBuffers_[ 0 ], sizeof( UniformBuffers_ ) );
	BcMemZero( &UniformBufferBindingInfo_[ 0 ], sizeof( UniformBufferBindingInfo_ ) );
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

	Width_ = Width;
	Height_ = Height;

	setDefaultState();
	setFrameBuffer( nullptr );
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

	//PSY_LOG( "Draw calls: %u\n", NoofDrawCalls_ );
	//PSY_LOG( "Render state flushes: %u\n", NoofRenderStateFlushes_ );
	NoofDrawCalls_ = 0;
	NoofRenderStateFlushes_ = 0;

	auto ScreenshotFunc = std::move( ScreenshotFunc_ );
	if( ScreenshotFunc != nullptr )
	{
		setFrameBuffer( nullptr );
		flushState();

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
	setRenderStateDesc( RenderStateDesc, BcTrue );

	// Ensure all buffers are cleared to black first.
	const BcU32 Width = pClient_->getWidth();
	const BcU32 Height = pClient_->getHeight();
	for( BcU32 Idx = 0; Idx < 3; ++Idx )
	{
		beginFrame( Width, Height );
		clear( RsColour( 0.0f, 0.0f, 0.0f, 0.0f ), BcTrue, BcTrue, BcTrue );
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

	// Create buffer impl.
	auto BufferGL = new RsBufferGL( Buffer, Version_ );
	Buffer->setHandle( BufferGL );

	++NoofBuffers_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextGL::destroyBuffer( RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Is buffer be in main memory?
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
	for( auto& UniformBuffer : UniformBuffers_ )
	{
		if( Buffer == UniformBuffer )
		{
			UniformBuffer = nullptr;
		}
	}

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
	BcU8* BufferData = BufferGL->BufferData_.get();
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
	if( BufferGL->Handle_ != 0 )
	{
		// Get buffer type for GL.
		auto TypeGL = RsUtilsGL::GetBufferType( BufferDesc.BindFlags_ );

		// Bind buffer.
		GL( BindBuffer( TypeGL, BufferGL->Handle_ ) );

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
	BufferGL->Version_++;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextGL::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& TextureDesc = Texture->getDesc();

	// Check if format is supported one.
	if( !Version_.Features_.TextureFormat_[ (int)TextureDesc.Format_ ] )
	{
		PSY_LOG( "ERROR: No support for %u format.", TextureDesc.Format_ );
		return false;
	}

	// Create GL texture.
	RsTextureGL* TextureGL = new RsTextureGL( Texture );
	Texture->setHandle( TextureGL );
	
	++NoofTextures_;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextGL::destroyTexture( 
		class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

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
		TextureGL->loadTexture( Slice, BcTrue, DataSize, &Data[ 0 ] );
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
	RsProgramGL* ProgramGL = new RsProgramGL( Program, Version_ );
	Program->setHandle( ProgramGL );

	// When creating programs, we want to make sure the program is rebound for next draw.
	ProgramDirty_ = BcTrue;

	++NoofPrograms_;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextGL::destroyProgram(
	class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
	delete ProgramGL;
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
	BcAssertMsg( IndexBuffer, "Can't bind a null index buffer." );
	BcAssertMsg( ( IndexBuffer->getDesc().BindFlags_ & RsResourceBindFlags::INDEX_BUFFER ) != RsResourceBindFlags::NONE, 
		"Buffer must be index buffer." );
	
	if( IndexBuffer_ != IndexBuffer )
	{
		IndexBuffer_ = IndexBuffer;
		IndexBufferDirty_ = BcTrue;
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
	BcAssertMsg( VertexBuffer, "Can't bind a null vertex buffer." );
	BcAssertMsg( ( VertexBuffer->getDesc().BindFlags_ & RsResourceBindFlags::VERTEX_BUFFER ) != RsResourceBindFlags::NONE, 
		"Buffer must be vertex buffer." );

	if( VertexBuffers_[ StreamIdx ].Buffer_ != VertexBuffer ||
		VertexBuffers_[ StreamIdx ].Stride_ != Stride )
	{
		VertexBuffers_[ StreamIdx ].Buffer_ = VertexBuffer;
		VertexBuffers_[ StreamIdx ].Stride_ = Stride;
		VertexBuffersDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextGL::setUniformBuffer( 
	BcU32 SlotIdx, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcAssertMsg( UniformBuffer, "Can't bind a null uniform buffer." );
	BcAssertMsg( ( UniformBuffer->getDesc().BindFlags_ & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE,
		"Buffer must be uniform buffer." );

	if( UniformBuffers_[ SlotIdx ] != UniformBuffer )
	{
		UniformBuffers_[ SlotIdx ] = UniformBuffer;
		UniformBufferBindingInfo_[ SlotIdx ].Dirty_ = BcTrue;
		UniformBuffersDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextGL::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( FrameBuffer_ != FrameBuffer )
	{
		FrameBufferDirty_ = BcTrue;
		FrameBuffer_ = FrameBuffer;

		BcU32 Width = Width_;
		BcU32 Height = Height_;
		if( FrameBuffer )
		{
			const auto& FBDesc = FrameBuffer->getDesc();
			const auto& TexDesc = FBDesc.RenderTargets_[ 0 ]->getDesc();
			Width = TexDesc.Width_;
			Height = TexDesc.Height_;
		}
		setViewport( RsViewport( 0, 0, Width, Height ) );
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
		PSY_PROFILER_SECTION( RSRoot, "Render state" );
		const auto& Desc = RenderState_->getDesc();
		setRenderStateDesc( Desc, BcFalse );
	}	

	// Bind texture states.
	{
		PSY_PROFILER_SECTION( TextureRoot, "Texture state" );
		for( BcU32 TextureStateIdx = 0; TextureStateIdx < NoofTextureStateBinds_; ++TextureStateIdx )
		{
			BcU32 TextureStateID = TextureStateBinds_[ TextureStateIdx ];
			TTextureStateValue& TextureStateValue = TextureStateValues_[ TextureStateID ];

			if( TextureStateValue.Dirty_ && (GLint)TextureStateID < Version_.MaxTextureSlots_ )
			{
				RsTexture* pTexture = TextureStateValue.pTexture_;			
				const RsSamplerState* SamplerState = TextureStateValue.pSamplerState_;
				const RsTextureType InternalType = pTexture ? pTexture->getDesc().Type_ : RsTextureType::TEX2D;
				const GLenum TextureType = RsUtilsGL::GetTextureType( InternalType );

				GL( ActiveTexture( GL_TEXTURE0 + TextureStateID ) );
				if( pTexture != nullptr )
				{
					RsTextureGL* TextureGL = pTexture->getHandle< RsTextureGL* >();
					BcAssert( TextureGL != nullptr );
					GL( BindTexture( TextureType, TextureGL->getHandle() ) );
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
						GL( BindSampler( TextureStateID, SamplerObject ) );
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
						GL( TexParameteri( TextureType, GL_TEXTURE_MIN_FILTER, RsUtilsGL::GetTextureFiltering( SamplerStateDesc.MinFilter_ ) ) );
						GL( TexParameteri( TextureType, GL_TEXTURE_MAG_FILTER, RsUtilsGL::GetTextureFiltering( SamplerStateDesc.MagFilter_ ) ) );
						GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_S, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressU_ ) ) );
						GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_T, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressV_ ) ) );	
#if !defined( RENDER_USE_GLES )
						GL( TexParameteri( TextureType, GL_TEXTURE_WRAP_R, RsUtilsGL::GetTextureSampling( SamplerStateDesc.AddressW_ ) ) );	
						GL( TexParameteri( TextureType, GL_TEXTURE_COMPARE_MODE, GL_NONE ) );
						GL( TexParameteri( TextureType, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ) );
#endif
					
					}
				}

				TextureStateValue.Dirty_ = BcFalse;
			}
		}
	}

	// Reset binds.
	NoofTextureStateBinds_ = 0;

	

	// Bind program and primitive.
	if( ( Program_ != nullptr &&
		  VertexDeclaration_ != nullptr ) &&
		( ProgramDirty_ || VertexBuffersDirty_ ) )
	{
		PSY_PROFILER_SECTION( ProgramRoot, "Program" );
		const auto& ProgramVertexAttributeList = Program_->getVertexAttributeList();
		const auto& VertexDeclarationDesc = VertexDeclaration_->getDesc();
		const auto& PrimitiveVertexElementList = VertexDeclarationDesc.Elements_;

		// Bind program if we need to.
		RsProgramGL* ProgramGL = Program_->getHandle< RsProgramGL* >();
		if( ProgramDirty_ )
		{
			GL( UseProgram( ProgramGL->getHandle() ) );
			ProgramDirty_ = BcFalse;
		}

		{
			PSY_PROFILER_SECTION( VtxRoot, "Vertex attributes" );

			// Cached vertex handle for binding.
			GLuint BoundVertexHandle = 0;

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
					auto VertexBufferBinding = VertexBuffers_[ FoundElement->StreamIdx_ ];
					auto VertexBuffer = VertexBufferBinding.Buffer_;
					auto VertexStride = VertexBufferBinding.Stride_;
			
					// Bind up new vertex buffer if we need to.
					BcAssertMsg( FoundElement->StreamIdx_ < VertexBuffers_.size(), "Stream index out of bounds for primitive." );
					BcAssertMsg( VertexBuffer != nullptr, "Vertex buffer not bound!" );
					auto VertexBufferGL = VertexBuffer->getHandle< RsBufferGL* >();
					BcAssert( VertexBufferGL );
					GLuint VertexHandle = VertexBufferGL->Handle_;
					if( BoundVertexHandle != VertexHandle )
					{
						GL( BindBuffer( GL_ARRAY_BUFFER, VertexHandle ) );
						BoundVertexHandle = VertexHandle;
					}

					// Enable array.
					VertexBufferActiveNextState_[ Attribute.Channel_ ] = true;

					// Bind.
					BcU64 CalcOffset = FoundElement->Offset_;

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
			VertexBuffersDirty_ = BcFalse;
		}
	}

	if( UniformBuffersDirty_ && Program_ )
	{
		RsProgramGL* ProgramGL = Program_->getHandle< RsProgramGL* >();

#if !defined( RENDER_USE_GLES )	
		// Bind up uniform buffers, or uniforms.
		if( Version_.SupportUniformBuffers_ )
		{
			PSY_PROFILER_SECTION( UBORoot, "UBO" );
			BcU32 BindingPoint = 0;
			for( BcU32 Idx = 0; Idx < UniformBuffers_.size(); ++Idx )
			{
				auto Buffer = UniformBuffers_[ Idx ];
				auto& BindingInfo = UniformBufferBindingInfo_[ Idx ];
				if( BindingInfo.Dirty_ && Buffer != nullptr )
				{
					auto BufferGL = Buffer->getHandle< RsBufferGL* >();
					BcAssert( BufferGL );
					GL( BindBufferRange( GL_UNIFORM_BUFFER, Idx, BufferGL->Handle_, 0, Buffer->getDesc().SizeBytes_ ) );
					BindingInfo.Dirty_ = BcFalse;
				}
			}
		}
#endif

		// Copy uniform buffers into program uniforms as required.
		ProgramGL->copyUniformBuffersToUniforms( UniformBuffers_.size(), UniformBuffers_.data() );
	}

	if( IndexBufferDirty_ )
	{
		// Bind indices.
		PSY_PROFILER_SECTION( IndicesRoot, "Indices" );
		auto IndexBufferGL = IndexBuffer_ ? IndexBuffer_->getHandle< RsBufferGL* >() : nullptr;
		GLuint IndicesHandle = IndexBufferGL != nullptr ? IndexBufferGL->Handle_ : 0;
		GL( BindBuffer( GL_ELEMENT_ARRAY_BUFFER, IndicesHandle ) );
		IndexBufferDirty_ = BcFalse;
	}

	// TODO: Redundant state.
	{
		PSY_PROFILER_SECTION( FBRoot, "FB, view, scissor" );
		if( FrameBufferDirty_ )
		{
			if( FrameBuffer_ != nullptr )
			{
				GL( BindFramebuffer( GL_FRAMEBUFFER, FrameBuffer_->getHandle< GLuint >() ) );
			}
			else
			{
				GL( BindFramebuffer( GL_FRAMEBUFFER, 0 ) );
			}
			FrameBufferDirty_ = BcFalse;
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
	GL( Clear( 
		( EnableClearColour ? GL_COLOR_BUFFER_BIT : 0 ) | 
		( EnableClearDepth ? GL_DEPTH_BUFFER_BIT : 0 ) | 
		( EnableClearStencil ? GL_STENCIL_BUFFER_BIT : 0 ) ) );
	
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextGL::drawPrimitives( RsTopologyType TopologyType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	PSY_PROFILER_SECTION( UpdateRoot, "RsContextGL::drawPrimitives" );
	++NoofDrawCalls_;
	flushState();
	BcAssert( Program_ != nullptr );
	GL( DrawArrays( RsUtilsGL::GetTopologyType( TopologyType ), IndexOffset, NoofIndices ) );

	
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
		GL( DrawElements( RsUtilsGL::GetTopologyType( TopologyType ), NoofIndices, GL_UNSIGNED_SHORT, (void*)( IndexOffset * sizeof( BcU16 ) ) ) );
	}
#if !defined( RENDER_USE_GLES )
	else if( Version_.SupportDrawElementsBaseVertex_ )
	{
		GL( DrawElementsBaseVertex( RsUtilsGL::GetTopologyType( TopologyType ), NoofIndices, GL_UNSIGNED_SHORT, (void*)( IndexOffset * sizeof( BcU16 ) ), VertexOffset ) );
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
#if !defined( RENDER_USE_GLES )
	// Grab current width + height.
	auto FBWidth = Width_;
	auto FBHeight = Height_;
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
	FrameBufferDirty_ = BcTrue;

	// Set read buffer
	GL( ReadBuffer( GL_COLOR_ATTACHMENT0 + Idx ) );

	const auto& TextureDesc = Texture->getDesc();
	auto TypeGL = RsUtilsGL::GetTextureType( TextureDesc.Type_ );
	const auto& FormatGL = RsUtilsGL::GetTextureFormat( TextureDesc.Format_ );

	RsTextureGL* DestTextureGL = Texture->getHandle< RsTextureGL* >();
	if( DestTextureGL->getHandle() != 0 )
	{
		// Bind texture.
		GL( BindTexture( TypeGL, DestTextureGL->getHandle() ) );
		GL( CopyTexImage2D( 
			TypeGL, 
			0, 
			FormatGL.InternalFormat_,
			0,
			0,
			TextureDesc.Width_,
			TextureDesc.Height_,
			0 ) );
		
		GL( BindTexture( TypeGL, 0 ) );
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
#if !defined( RENDER_USE_GLES )
	// Grab current width + height.
	auto FBWidth = Width_;
	auto FBHeight = Height_;
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

	FrameBufferDirty_ = BcTrue;
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
#endif // !defined( RENDER_USE_GLES )	
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextGL::setViewport( const class RsViewport& Viewport )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcAssert( Viewport.width() > 0 );
	BcAssert( Viewport.height() > 0 );

	auto FBWidth = Width_;
	auto FBHeight = Height_;
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

	const BcS32 SX = X;
	const BcS32 SY = FBHeight - ( Height + Y );
	const BcS32 SW = Width;
	const BcS32 SH = Height;

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
// dispatchCompute
void RsContextGL::dispatchCompute( class RsProgram* Program, RsDispatchBindings& Bindings, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups )
{
	RsProgramGL* ProgramGL = Program->getHandle< RsProgramGL* >();
	GL( UseProgram( ProgramGL->getHandle() ) );

	// Bind stuff.
	GLbitfield Barrier = 0;

	for( BcU32 Idx = 0; Idx < Bindings.ShaderResourceSlots_.size(); ++Idx )
	{
		auto& SRVSlot = Bindings.ShaderResourceSlots_[ Idx ];
		switch( SRVSlot.Type_ )
		{
		case RsShaderResourceType::INVALID:
			break;
		case RsShaderResourceType::BUFFER:
			{
				BcAssert( ( SRVSlot.Buffer_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
				RsBufferGL* BufferGL = SRVSlot.Buffer_->getHandle< RsBufferGL* >(); 
				GL( BindBufferBase( GL_SHADER_STORAGE_BUFFER, Idx, BufferGL->Handle_ ) );
			}
			break;
		case RsShaderResourceType::TEXTURE:
			{
				BcAssert( ( SRVSlot.Texture_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
				RsTextureGL* TextureGL = SRVSlot.Texture_->getHandle< RsTextureGL* >(); 
				GL( BindImageTexture( Idx, TextureGL->getHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, 
					RsUtilsGL::GetImageFormat( SRVSlot.Texture_->getDesc().Format_ ) ) );
			};
			break;
		default:
			BcBreakpoint;
		}
	}

	for( BcU32 Idx = 0; Idx < Bindings.UnorderedAccessSlots_.size(); ++Idx )
	{
		auto& UAVSlot = Bindings.UnorderedAccessSlots_[ Idx ];
		switch( UAVSlot.Type_ )
		{
		case RsUnorderedAccessType::INVALID:
			break;
		case RsUnorderedAccessType::BUFFER:
			{
				BcAssert( ( UAVSlot.Buffer_->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
				RsBufferGL* BufferGL = UAVSlot.Buffer_->getHandle< RsBufferGL* >(); 
				GL( BindBufferBase( GL_SHADER_STORAGE_BUFFER, Idx, BufferGL->Handle_ ) );
				Barrier |= GL_SHADER_STORAGE_BARRIER_BIT;
			}
			break;
		case RsUnorderedAccessType::TEXTURE:
			{
				BcAssert( ( UAVSlot.Texture_->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
				RsTextureGL* TextureGL = UAVSlot.Texture_->getHandle< RsTextureGL* >(); 
				GL( BindImageTexture( Idx, TextureGL->getHandle(), 0, GL_FALSE, 0, GL_READ_WRITE, 
					RsUtilsGL::GetImageFormat( UAVSlot.Texture_->getDesc().Format_ ) ) );
				Barrier |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
			};
			break;
		default:
			BcBreakpoint;
		}
	}

	// Dispatch.
	GL( DispatchCompute( XGroups, YGroups, ZGroups ) );

	// Barrier.
	if( Barrier )
	{
		GL( MemoryBarrier( Barrier ) );
	}

	// Program dirty, need to rebind it later.
	// Once stateless, won't be a problem.
	ProgramDirty_ = BcTrue;

	// Uniform buffers dirty, need to rebind later.
	UniformBuffersDirty_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// getOpenGLVersion
const RsOpenGLVersion& RsContextGL::getOpenGLVersion() const
{
	return Version_;
}

////////////////////////////////////////////////////////////////////////////////
// setRenderStateDesc
void RsContextGL::setRenderStateDesc( const RsRenderStateDesc& Desc, BcBool Force )
{
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
