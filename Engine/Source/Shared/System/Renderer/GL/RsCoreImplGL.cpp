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

#include "System/Renderer/GL/RsCoreImplGL.h"

#include "System/Renderer/GL/RsFrameGL.h"

#include "System/Renderer/GL/RsTextureGL.h"
#include "System/Renderer/GL/RsRenderTargetGL.h"
#include "System/Renderer/GL/RsRenderBufferGL.h"
#include "System/Renderer/GL/RsFrameBufferGL.h"
#include "System/Renderer/GL/RsVertexBufferGL.h"
#include "System/Renderer/GL/RsIndexBufferGL.h"
#include "System/Renderer/GL/RsShaderGL.h"
#include "System/Renderer/GL/RsProgramGL.h"
#include "System/Renderer/GL/RsPrimitiveGL.h"
#include "System/Renderer/GL/RsStateBlockGL.h"

#include "System/SysKernel.h"

#if PLATFORM_OSX
#include "OsViewOSX.h"
#endif

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
	BcAssert( BcIsGameThread() );
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
	const BcChar* pVersionString = reinterpret_cast< const BcChar* >( glGetString( GL_VERSION ) );
	const GLubyte* pExtensionString = glGetString( GL_EXTENSIONS );

	BcPrintf( " - glew initialised.\n" );
	BcPrintf( " - Version: %s\n", pVersionString );
	BcPrintf( " - Extensions: %s\n", pExtensionString );

	// Frame buffer object.
	if( !glewIsExtensionSupported ("GL_ARB_framebuffer_object" ) && !glewIsExtensionSupported( "GL_EXT_framebuffer_object" ) )
	{
		BcMessageBox( "ERROR", "Missing ARB_framebuffer_object or EXT_framebuffer_object extension. Can not continue.", bcMBT_OK, bcMBI_ERROR );
		exit(1);
	}

	// GLSL.
	if( !glewIsExtensionSupported( "GL_ARB_shader_objects" ) && !glewIsExtensionSupported( "GL_ARB_vertex_shader" ) && !glewIsExtensionSupported( "GL_ARB_fragment_shader" ) )
	{
		BcMessageBox( "ERROR", "Missing ARB_shader_objects, ARB_vertex_shader and ARB_fragment_shader extensions. Can not continue.", bcMBT_OK, bcMBI_ERROR );
		exit(1);
	}

	// Texture compression.
	if( !glewIsExtensionSupported( "GL_ARB_texture_compression" ) )
	{
		BcMessageBox( "ERROR", "Missing ARB_texture_compression extension. Can not continue.", bcMBT_OK, bcMBI_ERROR );
		exit(1);
	}

	// Vertex buffer objects.
	if( !glewIsExtensionSupported( "GL_ARB_vertex_buffer_object" ) )
	{
		BcMessageBox( "ERROR", "Missing ARB_vertex_buffer_object extension. Can not continue.", bcMBT_OK, bcMBI_ERROR );
		exit(1);
	}

	// Framebuffer object extension fix up.
	if( !glewIsExtensionSupported( "GL_ARB_framebuffer_object" ) && glewIsExtensionSupported( "GL_EXT_framebuffer_object" ) )
	{
		BcPrintf( " - WORKAROUND: Using EXT_framebuffer_object in place of ARB_framebuffer_object.\n" );

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

	// Make default context current and setup defaults.
	RsContextGL* pContext = static_cast< RsContextGL* >( ContextMap_[ NULL ] );
	if( pContext != NULL )
	{
		// Make current.
		pContext->makeCurrent();

		// Setup default viewport.
		glViewport( 0, 0, pContext->getWidth(), pContext->getHeight() );
		
		// Allocate a state block for rendering.
		pStateBlock_ = new RsStateBlockGL();
	
		//
		pStateBlock_->setRenderState( rsRS_DEPTH_WRITE_ENABLE, 1, BcTrue );
		pStateBlock_->bind();
	
		// Clear.
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	
		// Line smoothing.
		glEnable( GL_LINE_SMOOTH );
		glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsCoreImplGL::update()
{
	BcAssert( BcIsGameThread() );
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
	BcAssert( BcIsGameThread() );
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
}

//////////////////////////////////////////////////////////////////////////
// getContext
//virtual
RsContext* RsCoreImplGL::getContext( OsClient* pClient )
{
	BcAssert( BcIsGameThread() );
	TContextMapIterator It = ContextMap_.find( pClient );

	if( It != ContextMap_.end() )
	{
		return It->second;
	}
	else
	{
		if( pClient != NULL )
		{
			RsContextGL* pResource = new RsContextGL( pClient, ContextMap_[ NULL ] );
			createResource( pResource );

			// If we have no default context, set it.
			if( ContextMap_[ NULL ] == NULL )
			{
				ContextMap_[ NULL ] = pResource;
			}

			// Store mapped to client.
			ContextMap_[ pClient ] = pResource;

			return pResource;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// destroyContext
//virtual
void RsCoreImplGL::destroyContext( OsClient* pClient )
{
	BcAssert( BcIsGameThread() );
	TContextMapIterator It = ContextMap_.find( pClient );

	if( It != ContextMap_.end() )
	{
		// If we're destroying the default context, NULL it.
		if( ContextMap_[ NULL ] == It->second )
		{
			ContextMap_[ NULL ] = NULL;
		}
		
		// Destory resource.
		destroyResource( It->second );

		// Erase from context map.
		ContextMap_.erase( It );
	}
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplGL::createTexture( BcU32 Width, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	RsTextureGL* pResource = new RsTextureGL( Width, Levels, Format, pData );
	createResource( pResource );
	return pResource;
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
// createTexture
//virtual 
RsTexture* RsCoreImplGL::createTexture( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	RsTextureGL* pResource = new RsTextureGL( Width, Height, Depth, Levels, Format, pData );
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
// createProgram @deprecated
//virtual
RsProgram* RsCoreImplGL::createProgram( RsShader* pVertexShader, RsShader* pFragmentShader )
{
	RsShader* Shaders[] = 
	{
		pVertexShader,
		pFragmentShader
	};

	RsProgramGL* pResource = new RsProgramGL( 2, &Shaders[ 0 ] );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImplGL::createProgram( BcU32 NoofShaders, RsShader** ppShaders )
{
	RsProgramGL* pResource = new RsProgramGL( NoofShaders, ppShaders );
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
	BcScopedLock< BcMutex > Lock( ResourceLock_ ); // HACK: Should be invoking on game thread if not in game thread!

	pResource->preDestroy();

	// Make default context current.
	RsContextGL* pContext = ContextMap_[ NULL ];
	if( pContext != NULL )
	{
		RsContextGL::MakeCurrentDelegate Delegate( RsContextGL::MakeCurrentDelegate::bind< RsContextGL, &RsContextGL::makeCurrent >( ( pContext ) ) );
		SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
	}

	// Call destroy and wait.
	{
		SysResource::DestroyDelegate Delegate( SysResource::DestroyDelegate::bind< SysResource, &SysResource::destroy >( pResource ) );
		SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
	}
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void RsCoreImplGL::updateResource( RsResource* pResource )
{
	BcScopedLock< BcMutex > Lock( ResourceLock_ ); // HACK: Should be invoking on game thread if not in game thread!

	// Make default context current.
	RsContextGL* pContext = ContextMap_[ NULL ];
	if( pContext != NULL )
	{
		RsContextGL::MakeCurrentDelegate Delegate( RsContextGL::MakeCurrentDelegate::bind< RsContextGL, &RsContextGL::makeCurrent >( ( pContext ) ) );
		SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
	}

	// Call update.
	{
		SysResource::UpdateDelegate Delegate( SysResource::UpdateDelegate::bind< SysResource, &SysResource::update >( pResource ) );
		SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
	}
}

//////////////////////////////////////////////////////////////////////////
// createResource
void RsCoreImplGL::createResource( RsResource* pResource )
{
	BcScopedLock< BcMutex > Lock( ResourceLock_ ); // HACK: Should be invoking on game thread if not in game thread!

	// Make default context current.
	RsContextGL* pContext = ContextMap_[ NULL ];
	if( pContext != NULL )
	{
		RsContextGL::MakeCurrentDelegate Delegate( RsContextGL::MakeCurrentDelegate::bind< RsContextGL, &RsContextGL::makeCurrent >( ( pContext ) ) );
		SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
	}

	// Call create.
	{
		SysResource::CreateDelegate Delegate( SysResource::CreateDelegate::bind< SysResource, &SysResource::create >( pResource ) );
		SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate );
	}
}

//////////////////////////////////////////////////////////////////////////
// allocateFrame
RsFrame* RsCoreImplGL::allocateFrame( RsContext* pContext )
{
	BcAssert( BcIsGameThread() );
	if( pContext != NULL )
	{
		return new RsFrameGL( pContext );
	}
	else
	{
		return new RsFrameGL( ContextMap_[ NULL ] );
	}	
}

//////////////////////////////////////////////////////////////////////////
// queueFrame
void RsCoreImplGL::queueFrame( RsFrame* pFrame )
{
	BcAssert( BcIsGameThread() );
	BcDelegate< void(*)( RsFrameGL* ) > Delegate( BcDelegate< void(*)( RsFrameGL* ) >::bind< RsCoreImplGL, &RsCoreImplGL::queueFrame_threaded >( this ) );
	SysKernel::pImpl()->enqueueDelegateJob( RsCore::WORKER_MASK, Delegate, (RsFrameGL*)pFrame );
}

//////////////////////////////////////////////////////////////////////////
// queueFrame_threaded
void RsCoreImplGL::queueFrame_threaded( RsFrameGL* pFrame )
{
	// Render frame.
	pFrame->render();

	// Now free.
	delete pFrame;
}

//////////////////////////////////////////////////////////////////////////
// getStateBlock
RsStateBlock* RsCoreImplGL::getStateBlock()
{
	return pStateBlock_;
}

