/**************************************************************************
*
* File:		RsCoreImplGL.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "RsCoreImplGL.h"

#include "RsFrameGL.h"

#include "RsTextureGL.h"
#include "RsRenderTargetGL.h"
#include "RsRenderBufferGL.h"
#include "RsFrameBufferGL.h"
#include "RsVertexBufferGL.h"
#include "RsIndexBufferGL.h"
#include "RsShaderGL.h"
#include "RsProgramGL.h"
#include "RsPrimitiveGL.h"
#include "RsStateBlockGL.h"

#include "SysKernel.h"

#if PLATFORM_OSX
#include "OsViewOSX.h"
#elif PLATFORM_WINDOWS
extern BcHandle GWindowDC_;
extern BcHandle GWindowRC_;
#endif

// NEILO HACK.
extern BcU32 GResolutionWidth;
extern BcU32 GResolutionHeight;

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( RsCoreImplGL );

//////////////////////////////////////////////////////////////////////////
// Ctor
RsCoreImplGL::RsCoreImplGL()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsCoreImplGL::~RsCoreImplGL()
{

}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void RsCoreImplGL::open()
{
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImplGL, &RsCoreImplGL::open_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );

	// Wait for the render thread to complete.
	SysFence Fence;
	Fence.queue( RsCore::WORKER_MASK );
	Fence.wait();
}

//////////////////////////////////////////////////////////////////////////
// open_threaded
void RsCoreImplGL::open_threaded()
{
#if PLATFORM_OSX
	// Do the context switch.
	OsViewOSX_Interface::MakeContextCurrent();
#elif PLATFORM_WINDOWS
	// Pixel format.
	static  PIXELFORMATDESCRIPTOR pfd =                 // pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),                  // Size Of This Pixel Format Descriptor
		3,												// Version Number
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
	if ( !(PixelFormat = ::ChoosePixelFormat( (HDC)GWindowDC_, &pfd ) ) )
	{
		BcPrintf( "Can't create pixel format.\n" );
	}
	
	if( !::SetPixelFormat( (HDC)GWindowDC_, PixelFormat, &pfd ) )               // Are We Able To Set The Pixel Format?
	{
	    BcPrintf( "Can't Set The PixelFormat." );
	}
	
	// Create a rendering context.
	GWindowRC_ = (BcHandle)wglCreateContext( (HDC)GWindowDC_ );
	BcAssertMsg( GWindowRC_ != NULL, "RsCoreImplGL: Render context is NULL!" );

	// Do the context switch.
	wglMakeCurrent( (HDC)GWindowDC_, (HGLRC)GWindowRC_ );

	// Clear screen and flip.
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	::SwapBuffers( (HDC)GWindowDC_ );


	// Init GLee.
	GLeeInit();

	// Framebuffer object extension fix up.
	if( !GLEE_ARB_framebuffer_object && GLEE_EXT_framebuffer_object )
	{
		BcPrintf( "RsCoreImplGL: WORKAROUND: Using EXT_framebuffer_object in place of ARB_framebuffer_object.\n" );

		glIsRenderbuffer = glIsRenderbufferEXT;
		glBindRenderbuffer = glBindRenderbufferEXT;
		glDeleteRenderbuffers = glDeleteRenderbuffersEXT;
		glGenRenderbuffers = glGenRenderbuffersEXT;
		glRenderbufferStorage = glRenderbufferStorageEXT;
		glGetRenderbufferParameteriv = glGetRenderbufferParameterivEXT;
		glIsFramebuffer = glIsFramebufferEXT;
		glBindFramebuffer = glBindFramebufferEXT;
		glDeleteFramebuffers = glDeleteFramebuffersEXT;
		glGenFramebuffers = glGenFramebuffersEXT;
		glCheckFramebufferStatus = glCheckFramebufferStatusEXT;
		glFramebufferTexture1D = glFramebufferTexture1DEXT;
		glFramebufferTexture2D = glFramebufferTexture2DEXT;
		glFramebufferTexture3D = glFramebufferTexture3DEXT;
		glFramebufferRenderbuffer = glFramebufferRenderbufferEXT;
		glGetFramebufferAttachmentParameteriv = glGetFramebufferAttachmentParameterivEXT;
		glGenerateMipmap = glGenerateMipmapEXT;
		
		/* Unsupported by EXT :(
		glBlitFramebuffer
		glRenderbufferStorageMultisample
		glFramebufferTextureLayer
		*/
	}
#endif

	// NOTE: GL renderer uses SDL in this implementation.
	// TODO: Move into a higher level so this GL renderer
	//       can be used on any other platform.
	
	// Setup default viewport.
	glViewport( 0, 0, GResolutionWidth, GResolutionHeight );
		
	// Allocate a state block for rendering.
	pStateBlock_ = new RsStateBlockGL();
	
	//
	pStateBlock_->setRenderState( rsRS_DEPTH_WRITE_ENABLE, 1, BcTrue );
	pStateBlock_->bind();

	// Clear.
	glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	
	// Line smoothing.
	glEnable( GL_LINE_SMOOTH );
	glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsCoreImplGL::update()
{
	// Increment fence so we know how far we're getting ahead of ourselves.
	RenderSyncFence_.increment();

	// Queue update job.
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImplGL, &RsCoreImplGL::update_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );

	// Wait for frames if we fall more than 1 update cycle behind.
	RenderSyncFence_.wait( 1 );
}

//////////////////////////////////////////////////////////////////////////
// update_threaded
void RsCoreImplGL::update_threaded()
{
	// Decrement when we've done our update.
	RenderSyncFence_.decrement();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void RsCoreImplGL::close()
{
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImplGL, &RsCoreImplGL::close_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );

	// Wait for the render thread to complete.
	SysFence Fence;
	Fence.queue( RsCore::WORKER_MASK );
	Fence.wait();
}

//////////////////////////////////////////////////////////////////////////
// close_threaded
void RsCoreImplGL::close_threaded()
{
	// Free the state block.
	delete pStateBlock_;
	
#if PLATFORM_WINDOWS
	// Destroy rendering context.
	wglMakeCurrent( (HDC)GWindowDC_, NULL );
	wglDeleteContext( (HGLRC)GWindowRC_ );
#endif
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplGL::createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	RsTextureGL* pResource = new RsTextureGL( Width, Height, Levels, Format, pData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createRenderTarget
//virtual
RsRenderTarget*	RsCoreImplGL::createRenderTarget( BcU32 Width, BcU32 Height, eRsColourFormat ColourFormat, eRsDepthStencilFormat DepthStencilFormat )
{
	RsRenderBufferGL* pColourBuffer = new RsRenderBufferGL( ColourFormat, Width, Height );
	RsRenderBufferGL* pDepthStencilBuffer = new RsRenderBufferGL( DepthStencilFormat, Width, Height );
	RsFrameBufferGL* pFrameBuffer = new RsFrameBufferGL();
	RsTextureGL* pTexture = new RsTextureGL( Width, Height, 1, rsTF_RGBA8, NULL );

	createResource( pColourBuffer );
	createResource( pDepthStencilBuffer );
	createResource( pFrameBuffer );
	createResource( pTexture );

	// Create the render target.
	RsRenderTargetGL* pRenderTarget = new RsRenderTargetGL( ColourFormat, DepthStencilFormat, Width, Height, pColourBuffer, pDepthStencilBuffer, pFrameBuffer, pTexture );	
	createResource( pRenderTarget );
	
	return pRenderTarget;
}

//////////////////////////////////////////////////////////////////////////
// createVertexBuffer
//virtual 
RsVertexBuffer* RsCoreImplGL::createVertexBuffer( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData )
{
	RsVertexBufferGL* pResource = new RsVertexBufferGL( Descriptor, NoofVertices, pVertexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createIndexBuffer
//virtual 
RsIndexBuffer* RsCoreImplGL::createIndexBuffer( BcU32 NoofIndices, void* pIndexData )
{
	RsIndexBufferGL* pResource = new RsIndexBufferGL( NoofIndices, pIndexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createShader
//virtual
RsShader* RsCoreImplGL::createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize )
{
	RsShaderGL* pResource = new RsShaderGL( ShaderType, ShaderDataType, pShaderData, ShaderDataSize );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImplGL::createProgram( RsShader* pVertexShader, RsShader* pFragmentShader )
{
	RsProgramGL* pResource = new RsProgramGL( static_cast< RsShaderGL* >( pVertexShader ), static_cast< RsShaderGL* >( pFragmentShader ) );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createPrimitive
//virtual
RsPrimitive* RsCoreImplGL::createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer )
{
	RsPrimitiveGL* pResource = new RsPrimitiveGL( static_cast< RsVertexBufferGL* >( pVertexBuffer ), static_cast< RsIndexBufferGL* >( pIndexBuffer ) );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImplGL::destroyResource( RsResource* pResource )
{
	pResource->preDestroy();
	
	SysResource::DestroyDelegate Delegate( SysResource::DestroyDelegate::bind< SysResource, &SysResource::destroy >( pResource ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void RsCoreImplGL::updateResource( RsResource* pResource )
{
	SysResource::UpdateDelegate Delegate( SysResource::UpdateDelegate::bind< SysResource, &SysResource::update >( pResource ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
}

//////////////////////////////////////////////////////////////////////////
// createResource
void RsCoreImplGL::createResource( RsResource* pResource )
{
	SysResource::CreateDelegate Delegate( SysResource::CreateDelegate::bind< SysResource, &SysResource::create >( pResource ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
}

//////////////////////////////////////////////////////////////////////////
// allocateFrame
RsFrame* RsCoreImplGL::allocateFrame( BcHandle DeviceHandle, BcU32 Width, BcU32 Height )
{
	BcUnusedVar( DeviceHandle );
	BcUnusedVar( Width );
	BcUnusedVar( Height );
	
	return new RsFrameGL( NULL, GResolutionWidth, GResolutionHeight );
}

//////////////////////////////////////////////////////////////////////////
// queueFrame
void RsCoreImplGL::queueFrame( RsFrame* pFrame )
{
	BcDelegate< void(*)( RsFrameGL* ) > Delegate( BcDelegate< void(*)( RsFrameGL* ) >::bind< RsCoreImplGL, &RsCoreImplGL::queueFrame_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate, (RsFrameGL*)pFrame );
}

//////////////////////////////////////////////////////////////////////////
// queueFrame_threaded
void RsCoreImplGL::queueFrame_threaded( RsFrameGL* pFrame )
{
	// Render frame.
	pFrame->render();
}

//////////////////////////////////////////////////////////////////////////
// getStateBlock
RsStateBlock* RsCoreImplGL::getStateBlock()
{
	return pStateBlock_;
}

