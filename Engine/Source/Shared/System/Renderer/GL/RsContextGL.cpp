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
	GL_FLOAT,			// rsVDT_FLOAT32 = 0,
	GL_HALF_FLOAT,		// rsVDT_FLOAT16,
	GL_FIXED,			// rsVDT_FIXED,
	GL_BYTE,			// rsVDT_BYTE,
	GL_BYTE,			// rsVDT_BYTE_NORM,
	GL_UNSIGNED_BYTE,	// rsVDT_UBYTE,
	GL_UNSIGNED_BYTE,	// rsVDT_UBYTE_NORM,
	GL_SHORT,			// rsVDT_SHORT,
	GL_SHORT,			// rsVDT_SHORT_NORM,
	GL_UNSIGNED_SHORT,	// rsVDT_USHORT,
	GL_UNSIGNED_SHORT,	// rsVDT_USHORT_NORM,
	GL_INT,				// rsVDT_INT,
	GL_INT,				// rsVDT_INT_NORM,
	GL_UNSIGNED_INT,	// rsVDT_UINT,
	GL_UNSIGNED_INT		// rsVDT_UINT_NORM,
};

static GLboolean gVertexDataNormalised[] = 
{
	GL_FALSE,			// rsVDT_FLOAT32 = 0,
	GL_FALSE,			// rsVDT_FLOAT16,
	GL_FALSE,			// rsVDT_FIXED,
	GL_FALSE,			// rsVDT_BYTE,
	GL_TRUE,			// rsVDT_BYTE_NORM,
	GL_FALSE,			// rsVDT_UBYTE,
	GL_TRUE,			// rsVDT_UBYTE_NORM,
	GL_FALSE,			// rsVDT_SHORT,
	GL_TRUE,			// rsVDT_SHORT_NORM,
	GL_FALSE,			// rsVDT_USHORT,
	GL_TRUE,			// rsVDT_USHORT_NORM,
	GL_FALSE,			// rsVDT_INT,
	GL_TRUE,			// rsVDT_INT_NORM,
	GL_FALSE,			// rsVDT_UINT,
	GL_TRUE				// rsVDT_UINT_NORM,
};

static GLboolean gVertexDataSize[] = 
{
	4,					// rsVDT_FLOAT32 = 0,
	2,					// rsVDT_FLOAT16,
	4,					// rsVDT_FIXED,
	1,					// rsVDT_BYTE,
	1,					// rsVDT_BYTE_NORM,
	1,					// rsVDT_UBYTE,
	1,					// rsVDT_UBYTE_NORM,
	2,					// rsVDT_SHORT,
	2,					// rsVDT_SHORT_NORM,
	2,					// rsVDT_USHORT,
	2,					// rsVDT_USHORT_NORM,
	4,					// rsVDT_INT,
	4,					// rsVDT_INT_NORM,
	4,					// rsVDT_UINT,
	4					// rsVDT_UINT_NORM,
};

static GLenum gPrimitiveType[] =
{
	GL_POINTS,			// rsPT_POINTLIST = 0,
	GL_LINES,			// rsPT_LINELIST,
	GL_LINE_STRIP,		// rsPT_LINESTRIP,
	GL_TRIANGLES,		// rsPT_TRIANGLELIST,
	GL_TRIANGLE_STRIP,	// rsPT_TRIANGLESTRIP,
	GL_TRIANGLE_FAN,	// rsPT_TRIANGLEFAN,
	GL_PATCHES			// rsPT_PATCHES,
};

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextGL::RsContextGL( OsClient* pClient, RsContextGL* pParent ):
	RsContext( pParent ),
	pParent_( pParent ),
	pClient_( pClient ),
	ScreenshotRequested_( BcFalse ),
	OwningThread_( BcErrorCode )
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
	std::pair< BcU32, BcU32 > versions[] = 
	{
		std::pair< BcU32, BcU32 >( 4, 4 ),
		std::pair< BcU32, BcU32 >( 4, 3 ),
		std::pair< BcU32, BcU32 >( 4, 2 ),
		std::pair< BcU32, BcU32 >( 4, 1 ),
		std::pair< BcU32, BcU32 >( 4, 0 ),
		std::pair< BcU32, BcU32 >( 3, 3 ),
		std::pair< BcU32, BcU32 >( 3, 2 ),
	};

	HGLRC ParentContext = pParent_ != NULL ? pParent_->WindowRC_ : NULL;
	bool success = false;
	for( auto version : versions )
	{
		if( createProfile( version.first, version.second, BcTrue, ParentContext ) )
		{
			BcPrintf( "RsContextGL: Created OpenGL %u.%u Core Profile.\n", version.first, version.second );
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
bool RsContextGL::createProfile( BcU32 Maj, BcU32 Min, BcBool IsCore, HGLRC ParentContext )
{
	int ContextAttribs[] = 
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, Maj,
		WGL_CONTEXT_MINOR_VERSION_ARB, Min,
		WGL_CONTEXT_PROFILE_MASK_ARB, IsCore ? WGL_CONTEXT_CORE_PROFILE_BIT_ARB : WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
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
	setRenderState( rsRS_DEPTH_WRITE_ENABLE,		1,					BcTrue );
	setRenderState( rsRS_DEPTH_TEST_ENABLE,			0,					BcTrue );
	setRenderState( rsRS_DEPTH_TEST_COMPARE,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_WRITE_MASK,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_ENABLE,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_FUNC_COMPARE,	0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_FUNC_REF,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_FUNC_MASK,	0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_OP_SFAIL,		0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_OP_DPFAIL,	0,					BcTrue );
	setRenderState( rsRS_STENCIL_TEST_OP_DPPASS,	0,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_MASK_0,		15,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_MASK_1,		0,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_MASK_2,		0,					BcTrue );
	setRenderState( rsRS_COLOR_WRITE_MASK_3,		0,					BcTrue );
	setRenderState( rsRS_BLEND_MODE,				0,					BcTrue );
	
	// Setup default texture states.
	RsTextureParams TextureParams = 
	{
		rsTFM_LINEAR, rsTFM_LINEAR, rsTSM_WRAP, rsTSM_WRAP
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
void RsContextGL::setRenderState( eRsRenderState State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( State < NOOF_RENDERSTATES )
	{
		TRenderStateValue& RenderStateValue = RenderStateValues_[ State ];
		
		const BcBool WasDirty = RenderStateValue.Dirty_;
		
		RenderStateValue.Dirty_ |= ( RenderStateValue.Value_ != Value ) || Force;
		RenderStateValue.Value_ = Value;
		
		// If it wasn't dirty, we need to set it.
		if( WasDirty == BcFalse && RenderStateValue.Dirty_ == BcTrue )
		{
			BcAssert( NoofRenderStateBinds_ < NOOF_RENDERSTATES );
			RenderStateBinds_[ NoofRenderStateBinds_++ ] = State;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextGL::getRenderState( eRsRenderState State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( State < NOOF_RENDERSTATES )
	{
		const TRenderStateValue& RenderStateValue = RenderStateValues_[ State ];

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
				case rsRS_DEPTH_WRITE_ENABLE:
					glDepthMask( Value );
					break;
				case rsRS_DEPTH_TEST_ENABLE:
					Value ? glEnable( GL_DEPTH_TEST ) : glDisable( GL_DEPTH_TEST );
					break;
				case rsRS_DEPTH_TEST_COMPARE:
					glDepthFunc( gCompareMode[ Value ] );
					break;
				case rsRS_STENCIL_WRITE_MASK:
					glStencilMask( Value );
					break;
				case rsRS_STENCIL_TEST_ENABLE:
					Value ? glEnable( GL_STENCIL_TEST ) : glDisable( GL_STENCIL_TEST );
					break;
				case rsRS_STENCIL_TEST_FUNC_COMPARE:
				case rsRS_STENCIL_TEST_FUNC_REF:
				case rsRS_STENCIL_TEST_FUNC_MASK:
					bindStencilFunc();
					break;
				case rsRS_STENCIL_TEST_OP_SFAIL:
				case rsRS_STENCIL_TEST_OP_DPFAIL:
				case rsRS_STENCIL_TEST_OP_DPPASS:
					bindStencilOp();
					break;
				case rsRS_BLEND_MODE:
					bindBlendMode( (eRsBlendingMode)Value );
					break;
				case rsRS_COLOR_WRITE_MASK_0:
				case rsRS_COLOR_WRITE_MASK_1:
				case rsRS_COLOR_WRITE_MASK_2:
				case rsRS_COLOR_WRITE_MASK_3:
					glColorMaski( RenderStateID - rsRS_COLOR_WRITE_MASK_0, ( Value & 0x8 ) >> 3, ( Value & 0x4 ) >> 2,( Value & 0x2 ) >> 1, ( Value & 0x1 ) );
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
			const eRsTextureType InternalType = pTexture ? pTexture->type() : rsTT_2D;
			const GLenum TextureType = gTextureTypes[ InternalType ];

			glActiveTexture( GL_TEXTURE0 + TextureStateID );
			glBindTexture( TextureType, pTexture ? pTexture->getHandle< GLuint >() : 0 );
			glTexParameteri( TextureType, GL_TEXTURE_MIN_FILTER, gTextureFiltering[ Params.MinFilter_ ] );
			glTexParameteri( TextureType, GL_TEXTURE_MAG_FILTER, gTextureFiltering[ Params.MagFilter_ ] );
			glTexParameteri( TextureType, GL_TEXTURE_WRAP_S, gTextureSampling[ Params.UMode_ ] );
			glTexParameteri( TextureType, GL_TEXTURE_WRAP_T, gTextureSampling[ Params.VMode_ ] );	
			glTexParameteri( TextureType, GL_TEXTURE_WRAP_R, gTextureSampling[ Params.WMode_ ] );	

			RsGLCatchError;

			TextureStateValue.Dirty_ = BcFalse;
		}
	}
	
	// Reset binds.
	NoofRenderStateBinds_ = 0;
	NoofTextureStateBinds_ = 0;

	RsGLCatchError;
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextGL::clear( const RsColour& Colour )
{
	glClearColor( Colour.r(), Colour.g(), Colour.b(), Colour.a() );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );	
}

//////////////////////////////////////////////////////////////////////////
// setPrimitive
void RsContextGL::setPrimitive( class RsProgram* Program, class RsPrimitive* Primitive )
{
	// Bind primitive to program.
	const auto& ProgramVertexAttributeList = Program->getVertexAttributeList();
	const auto& PrimitiveDesc = Primitive->getDesc();
	const auto& VertexDeclarationDesc = PrimitiveDesc.VertexDeclaration_->getDesc();
	const auto& PrimitiveVertexElementList = VertexDeclarationDesc.Elements_;

	// Bind program.
	Program->bind( nullptr );

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
				BcU32 CalcOffset = Element.Offset_ * gVertexDataSize[ Element.DataType_ ];

				glVertexAttribPointer( Attribute.Channel_, 
					Element.Components_,
					gVertexDataTypes[ Element.DataType_ ],
					gVertexDataNormalised[ Element.DataType_ ],
					VertexBuffer->getVertexStride(),
					(GLvoid*)CalcOffset );

				break;	
			}
		}
	}

	// Bind indices.
	GLuint IndicesHandle = PrimitiveDesc.IndexBuffer_->getHandle< GLuint >();
	if( IndicesHandle != 0 )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, IndicesHandle );
	}
}

//////////////////////////////////////////////////////////////////////////
// draw
void RsContextGL::draw( eRsPrimitiveType PrimitiveType, BcU32 Offset, BcU32 NoofIndices )
{
	//
}

//////////////////////////////////////////////////////////////////////////
// bindStencilFunc
void RsContextGL::bindStencilFunc()
{
	TRenderStateValue& CompareValue = RenderStateValues_[ rsRS_STENCIL_TEST_FUNC_COMPARE ];
	TRenderStateValue& RefValue = RenderStateValues_[ rsRS_STENCIL_TEST_FUNC_REF ];
	TRenderStateValue& MaskValue = RenderStateValues_[ rsRS_STENCIL_TEST_FUNC_MASK ];

	glStencilFunc( gCompareMode[ CompareValue.Value_ ], RefValue.Value_, MaskValue.Value_ );

	CompareValue.Dirty_ = BcFalse;
	RefValue.Dirty_ = BcFalse;
	MaskValue.Dirty_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// bindStencilOp
void RsContextGL::bindStencilOp()
{
	TRenderStateValue& SFailValue = RenderStateValues_[ rsRS_STENCIL_TEST_OP_SFAIL ];
	TRenderStateValue& DPFailValue = RenderStateValues_[ rsRS_STENCIL_TEST_OP_DPFAIL ];
	TRenderStateValue& DPPassValue = RenderStateValues_[ rsRS_STENCIL_TEST_OP_DPPASS ];

	glStencilOp( gStencilOp[ SFailValue.Value_ ], gStencilOp[ DPFailValue.Value_ ], gStencilOp[ DPPassValue.Value_ ] );

	SFailValue.Dirty_ = BcFalse;
	DPFailValue.Dirty_ = BcFalse;
	DPPassValue.Dirty_ = BcFalse;
}


//////////////////////////////////////////////////////////////////////////
// bindBlendMode
void RsContextGL::bindBlendMode( eRsBlendingMode BlendMode )
{
	switch( BlendMode )
	{
		case rsBM_NONE:
			glDisable( GL_BLEND );
			glBlendFunc( GL_ONE, GL_ZERO );
			break;
					
		case rsBM_BLEND:
			glEnable( GL_BLEND );
			glBlendEquation( GL_FUNC_ADD );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
			break;
					
		case rsBM_ADD:
			glEnable( GL_BLEND );
			glBlendEquation( GL_FUNC_ADD );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;

		case rsBM_SUBTRACT:
			glEnable( GL_BLEND );
			glBlendEquation( GL_FUNC_REVERSE_SUBTRACT );
			glBlendFunc( GL_SRC_ALPHA, GL_ONE );
			break;
			
		default:
			BcBreakpoint;
			break;
	}
}
