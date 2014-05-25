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
#include "System/Renderer/GL/RsTextureGL.h"
#include "System/Renderer/GL/RsShaderGL.h"
#include "System/Renderer/GL/RsProgramGL.h"
#include "System/Renderer/GL/RsPrimitiveGL.h"
#include "System/Renderer/GL/RsVertexBufferGL.h"
#include "System/Renderer/GL/RsIndexBufferGL.h"
#include "System/Renderer/GL/RsTextureGL.h"

#include "System/Renderer/RsVertexDeclaration.h"

#include "System/Os/OsClient.h"

#include "Import/Img/Img.h"

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

static GLenum gPrimitiveType[] =
{
	GL_POINTS,			// RsPrimitiveType::POINTLIST = 0,
	GL_LINES,			// RsPrimitiveType::LINELIST,
	GL_LINE_STRIP,		// RsPrimitiveType::LINESTRIP,
	GL_TRIANGLES,		// RsPrimitiveType::TRIANGLELIST,
	GL_TRIANGLE_STRIP,	// RsPrimitiveType::TRIANGLESTRIP,
	GL_TRIANGLE_FAN,	// RsPrimitiveType::TRIANGLEFAN,
	GL_PATCHES			// RsPrimitiveType::PATCHES,
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
	PrimitiveDirty_( BcTrue ),
	Program_( nullptr ),
	Primitive_( nullptr )
{
	BcMemZero( &RenderStateValues_[ 0 ], sizeof( RenderStateValues_ ) );
	BcMemZero( &TextureStateValues_[ 0 ], sizeof( TextureStateValues_ ) );
	BcMemZero( &RenderStateBinds_[ 0 ], sizeof( RenderStateBinds_ ) );
	BcMemZero( &TextureStateBinds_[ 0 ], sizeof( TextureStateBinds_ ) );

	NoofRenderStateBinds_ = 0;
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
			Version_.Minor_ >= 0 &&
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
// swapBuffers
void RsContextGL::swapBuffers()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( ScreenshotRequested_ == BcFalse )
	{
		glFlush();
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
		RsOpenGLVersion( 4, 4, RsOpenGLType::CORE, RsShaderCodeType::GLSL_440 ),
		RsOpenGLVersion( 4, 3, RsOpenGLType::CORE, RsShaderCodeType::GLSL_430 ),
		RsOpenGLVersion( 4, 2, RsOpenGLType::CORE, RsShaderCodeType::GLSL_420 ),
		RsOpenGLVersion( 4, 1, RsOpenGLType::CORE, RsShaderCodeType::GLSL_410 ),
		RsOpenGLVersion( 4, 0, RsOpenGLType::CORE, RsShaderCodeType::GLSL_400 ),
		RsOpenGLVersion( 3, 3, RsOpenGLType::CORE, RsShaderCodeType::GLSL_330 ),
		RsOpenGLVersion( 3, 2, RsOpenGLType::CORE, RsShaderCodeType::GLSL_150 ),
	};

	HGLRC ParentContext = pParent_ != NULL ? pParent_->WindowRC_ : NULL;
	bool Success = false;
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

		// Make current.
		wglMakeCurrent( WindowDC_, WindowRC_ );
	}
#endif

	// Create + bind global VAO.
	glGenVertexArrays( 1, &GlobalVAO_ );
	glBindVertexArray( GlobalVAO_ );

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	//
	RsGLCatchError;

	// Set default state.
	setDefaultState();

	// Clear screen and flip.
	clear( RsColour( 0.0f, 0.0f, 0.0f, 0.0f ) );

	// Swap buffers.
	swapBuffers();
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
}

//////////////////////////////////////////////////////////////////////////
// createProfile
bool RsContextGL::createProfile( RsOpenGLVersion Version, HGLRC ParentContext )
{
	BcAssert( Version.Type_ == RsOpenGLType::CORE ||
	          Version.Type_ == RsOpenGLType::COMPATIBILITY );
	
	int ContextAttribs[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, Version.Major_,
		WGL_CONTEXT_MINOR_VERSION_ARB, Version.Minor_,
		WGL_CONTEXT_PROFILE_MASK_ARB, Version.Type_ == RsOpenGLType::CORE ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
		NULL
	};

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

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextGL::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Setup default render states.
	setRenderState( RsRenderStateType::DEPTH_WRITE_ENABLE,			1,					BcTrue );
	setRenderState( RsRenderStateType::DEPTH_TEST_ENABLE,			0,					BcTrue );
	setRenderState( RsRenderStateType::DEPTH_TEST_COMPARE,			0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_WRITE_MASK,			0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_TEST_ENABLE,			0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_TEST_FUNC_COMPARE,	0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_TEST_FUNC_REF,		0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_TEST_FUNC_MASK,		0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_TEST_OP_SFAIL,		0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_TEST_OP_DPFAIL,		0,					BcTrue );
	setRenderState( RsRenderStateType::STENCIL_TEST_OP_DPPASS,		0,					BcTrue );
	setRenderState( RsRenderStateType::COLOR_WRITE_MASK_0,			15,					BcTrue );
	setRenderState( RsRenderStateType::COLOR_WRITE_MASK_1,			0,					BcTrue );
	setRenderState( RsRenderStateType::COLOR_WRITE_MASK_2,			0,					BcTrue );
	setRenderState( RsRenderStateType::COLOR_WRITE_MASK_3,			0,					BcTrue );
	setRenderState( RsRenderStateType::BLEND_MODE,					0,					BcTrue );
	
	// Setup default texture states.
	RsTextureParams TextureParams = 
	{
		RsTextureFilteringMode::LINEAR, RsTextureFilteringMode::LINEAR, RsTextureSamplingMode::WRAP, RsTextureSamplingMode::WRAP
	};

	for( BcU32 Sampler = 0; Sampler < NOOF_TEXTURESTATES; ++Sampler )
	{
		setTextureState( Sampler, NULL, TextureParams, BcTrue );
	}

	flushState();
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextGL::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	NoofRenderStateBinds_ = 0;
	for( BcU32 Idx = 0; Idx < NOOF_RENDERSTATES; ++Idx )
	{
		TRenderStateValue& RenderStateValue = RenderStateValues_[ Idx ];

		RenderStateValue.Dirty_ = BcTrue;
		
		BcAssert( NoofRenderStateBinds_ < NOOF_RENDERSTATES );
		RenderStateBinds_[ NoofRenderStateBinds_++ ] = Idx;
	}
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextGL::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	NoofTextureStateBinds_ = 0;
	for( BcU32 Idx = 0; Idx < NOOF_TEXTURESTATES; ++Idx )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Idx ];
		
		TextureStateValue.Dirty_ = BcTrue;
		
		BcAssert( NoofTextureStateBinds_ < NOOF_TEXTURESTATES );
		TextureStateBinds_[ NoofTextureStateBinds_++ ] = Idx;
	}
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextGL::setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( (BcU32)State < NOOF_RENDERSTATES )
	{
		TRenderStateValue& RenderStateValue = RenderStateValues_[ (BcU32)State ];
		
		const BcBool WasDirty = RenderStateValue.Dirty_;
		
		RenderStateValue.Dirty_ |= ( RenderStateValue.Value_ != Value ) || Force;
		RenderStateValue.Value_ = Value;
		
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && RenderStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofRenderStateBinds_ < NOOF_RENDERSTATES );
			RenderStateBinds_[ NoofRenderStateBinds_++ ] = (BcU32)State;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextGL::getRenderState( RsRenderStateType State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( (BcU32)State < NOOF_RENDERSTATES )
	{
		const TRenderStateValue& RenderStateValue = RenderStateValues_[ (BcU32)State ];

		return RenderStateValue.Value_;
	}

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setTextureState
void RsContextGL::setTextureState( BcU32 Sampler, RsTexture* pTexture, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( Sampler < NOOF_TEXTURESTATES )
	{
		TTextureStateValue& TextureStateValue = TextureStateValues_[ Sampler ];
		
		const BcBool WasDirty = TextureStateValue.Dirty_;
		
		TextureStateValue.Dirty_ |= ( TextureStateValue.pTexture_ != pTexture || TextureStateValue.Params_ != Params ) || Force;
		TextureStateValue.pTexture_ = pTexture;
		TextureStateValue.Params_ = Params;
	
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && TextureStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofTextureStateBinds_ < NOOF_TEXTURESTATES );
			TextureStateBinds_[ NoofTextureStateBinds_++ ] = Sampler;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextGL::setProgram( class RsProgram* Program )
{
	if( Program_ != Program )
	{
		Program_ = Program;
		ProgramDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setPrimitive
void RsContextGL::setPrimitive( class RsPrimitive* Primitive )
{
	if( Primitive_ != Primitive )
	{
		Primitive_ = Primitive;
		PrimitiveDirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextGL::flushState()
{
	// Bind render states.
	for( BcU32 RenderStateIdx = 0; RenderStateIdx < NoofRenderStateBinds_; ++RenderStateIdx )
	{
		BcU32 RenderStateID = RenderStateBinds_[ RenderStateIdx ];
		TRenderStateValue& RenderStateValue = RenderStateValues_[ RenderStateID ];
		
		// Some states are dependant, and set in batches.
		if( RenderStateValue.Dirty_ )
		{
			const BcS32 Value = RenderStateValue.Value_;
		
			switch( RenderStateID )
			{
				case RsRenderStateType::DEPTH_WRITE_ENABLE:
					glDepthMask( Value );
					break;
				case RsRenderStateType::DEPTH_TEST_ENABLE:
					Value ? glEnable( GL_DEPTH_TEST ) : glDisable( GL_DEPTH_TEST );
					break;
				case RsRenderStateType::DEPTH_TEST_COMPARE:
					glDepthFunc( gCompareMode[ Value ] );
					break;
				case RsRenderStateType::STENCIL_WRITE_MASK:
					glStencilMask( Value );
					break;
				case RsRenderStateType::STENCIL_TEST_ENABLE:
					Value ? glEnable( GL_STENCIL_TEST ) : glDisable( GL_STENCIL_TEST );
					break;
				case RsRenderStateType::STENCIL_TEST_FUNC_COMPARE:
				case RsRenderStateType::STENCIL_TEST_FUNC_REF:
				case RsRenderStateType::STENCIL_TEST_FUNC_MASK:
					bindStencilFunc();
					break;
				case RsRenderStateType::STENCIL_TEST_OP_SFAIL:
				case RsRenderStateType::STENCIL_TEST_OP_DPFAIL:
				case RsRenderStateType::STENCIL_TEST_OP_DPPASS:
					bindStencilOp();
					break;
				case RsRenderStateType::BLEND_MODE:
					bindBlendMode( (RsBlendingMode)Value );
					break;
				case RsRenderStateType::COLOR_WRITE_MASK_0:
				case RsRenderStateType::COLOR_WRITE_MASK_1:
				case RsRenderStateType::COLOR_WRITE_MASK_2:
				case RsRenderStateType::COLOR_WRITE_MASK_3:
					glColorMaski( RenderStateID - (BcU32)RsRenderStateType::COLOR_WRITE_MASK_0, ( Value & 0x8 ) >> 3, ( Value & 0x4 ) >> 2,( Value & 0x2 ) >> 1, ( Value & 0x1 ) );
					break;
			}
			
			// No longer dirty.
			RenderStateValue.Dirty_ = BcFalse;

			// Catch errors.
	 		RsGLCatchError;
		}
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
			const RsTextureType InternalType = pTexture ? pTexture->type() : RsTextureType::TEX2D;
			const GLenum TextureType = gTextureTypes[ (BcU32)InternalType ];

			glActiveTexture( GL_TEXTURE0 + TextureStateID );
			glBindTexture( TextureType, pTexture ? pTexture->getHandle< GLuint >() : 0 );
			glTexParameteri( TextureType, GL_TEXTURE_MIN_FILTER, gTextureFiltering[ (BcU32)Params.MinFilter_ ] );
			glTexParameteri( TextureType, GL_TEXTURE_MAG_FILTER, gTextureFiltering[ (BcU32)Params.MagFilter_ ] );
			glTexParameteri( TextureType, GL_TEXTURE_WRAP_S, gTextureSampling[ (BcU32)Params.UMode_ ] );
			glTexParameteri( TextureType, GL_TEXTURE_WRAP_T, gTextureSampling[ (BcU32)Params.VMode_ ] );	
			glTexParameteri( TextureType, GL_TEXTURE_WRAP_R, gTextureSampling[ (BcU32)Params.WMode_ ] );	

			RsGLCatchError;

			TextureStateValue.Dirty_ = BcFalse;
		}
	}
	
	// Reset binds.
	NoofRenderStateBinds_ = 0;
	NoofTextureStateBinds_ = 0;

	RsGLCatchError;

	// Bind program and primitive.
	if( ( Program_ != nullptr ||
		  Primitive_ != nullptr ) &&
		( ProgramDirty_ || PrimitiveDirty_ ) )
	{
		const auto& ProgramVertexAttributeList = Program_->getVertexAttributeList();
		const auto& PrimitiveDesc = Primitive_->getDesc();
		const auto& VertexDeclarationDesc = PrimitiveDesc.VertexDeclaration_->getDesc();
		const auto& PrimitiveVertexElementList = VertexDeclarationDesc.Elements_;

		// Bind program.
		Program_->bind( nullptr );

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
					auto VertexBuffer = PrimitiveDesc.VertexBuffers_[ Element.StreamIdx_ ];
				
					// Bind up new vertex buffer if we need to.
					BcAssertMsg( Element.StreamIdx_ < PrimitiveDesc.VertexBuffers_.size(), "Stream index out of bounds for primitive." );
					BcAssertMsg( VertexBuffer != nullptr, "Vertex buffer not bound!" );
					GLuint VertexHandle = PrimitiveDesc.VertexBuffers_[ Element.StreamIdx_ ]->getHandle< GLuint >();
					if( BoundVertexHandle != VertexHandle )
					{
						glBindBuffer( GL_ARRAY_BUFFER, VertexHandle );
						BoundVertexHandle = VertexHandle;
					}

					// Enable array.
					glEnableVertexAttribArray( Attribute.Channel_ );

					// Bind.
					BcU32 CalcOffset = Element.Offset_;

					glVertexAttribPointer( Attribute.Channel_, 
						Element.Components_,
						gVertexDataTypes[ (BcU32)Element.DataType_ ],
						gVertexDataNormalised[ (BcU32)Element.DataType_ ],
						VertexBuffer->getVertexStride(),
						(GLvoid*)CalcOffset );
					break;	
				}
			}
		}

		// Bind indices.
		GLuint IndicesHandle = PrimitiveDesc.IndexBuffer_ != nullptr ? PrimitiveDesc.IndexBuffer_->getHandle< GLuint >() : 0;
		if( IndicesHandle != 0 )
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IndicesHandle );
		}

		ProgramDirty_ = BcFalse;
		PrimitiveDirty_ = BcFalse;
		RsGLCatchError;
	}

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextGL::clear( const RsColour& Colour )
{
	flushState();
	glClearColor( Colour.r(), Colour.g(), Colour.b(), Colour.a() );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );	
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextGL::drawPrimitives( RsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices )
{
	flushState();
	glDrawArrays( gPrimitiveType[ (BcU32)PrimitiveType ], Offset, NoofIndices );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextGL::drawIndexedPrimitives( RsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices )
{
	flushState();
	glDrawElements( gPrimitiveType[ (BcU32)PrimitiveType ], NoofIndices, GL_UNSIGNED_SHORT, (void*)( Offset * sizeof( BcU16 ) ) );
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
	TRenderStateValue& CompareValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_COMPARE ];
	TRenderStateValue& RefValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_REF ];
	TRenderStateValue& MaskValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_MASK ];

	glStencilFunc( gCompareMode[ CompareValue.Value_ ], RefValue.Value_, MaskValue.Value_ );

	CompareValue.Dirty_ = BcFalse;
	RefValue.Dirty_ = BcFalse;
	MaskValue.Dirty_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// bindStencilOp
void RsContextGL::bindStencilOp()
{
	TRenderStateValue& SFailValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_SFAIL ];
	TRenderStateValue& DPFailValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPFAIL ];
	TRenderStateValue& DPPassValue = RenderStateValues_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPPASS ];

	glStencilOp( gStencilOp[ SFailValue.Value_ ], gStencilOp[ DPFailValue.Value_ ], gStencilOp[ DPPassValue.Value_ ] );

	SFailValue.Dirty_ = BcFalse;
	DPFailValue.Dirty_ = BcFalse;
	DPPassValue.Dirty_ = BcFalse;
}


//////////////////////////////////////////////////////////////////////////
// bindBlendMode
void RsContextGL::bindBlendMode( RsBlendingMode BlendMode )
{
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
}
