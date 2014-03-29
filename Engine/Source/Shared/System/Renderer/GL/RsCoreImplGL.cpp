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
#endif

	// Make default context current and setup defaults.
	RsContextGL* pContext = static_cast< RsContextGL* >( ContextMap_[ NULL ] );
	if( pContext != NULL )
	{
		// Setup default viewport.
		glViewport( 0, 0, pContext->getWidth(), pContext->getHeight() );
		
		//
		pContext->setRenderState( rsRS_DEPTH_WRITE_ENABLE, 1, BcTrue );
		pContext->flushState();
	
		// Clear.
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
	
		// Line smoothing.
		glEnable( GL_LINE_SMOOTH );
		glHint( GL_LINE_SMOOTH_HINT, GL_NICEST );

		// GGJ2013 HACK.
		glDisable( GL_CULL_FACE );
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
	RsTextureGL* pResource = new RsTextureGL( getContext( NULL ), Width, Levels, Format, pData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplGL::createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	RsTextureGL* pResource = new RsTextureGL( getContext( NULL ), Width, Height, Levels, Format, pData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplGL::createTexture( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	RsTextureGL* pResource = new RsTextureGL( getContext( NULL ), Width, Height, Depth, Levels, Format, pData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createRenderTarget
//virtual
RsRenderTarget*	RsCoreImplGL::createRenderTarget( BcU32 Width, BcU32 Height, eRsColourFormat ColourFormat, eRsDepthStencilFormat DepthStencilFormat )
{
	RsRenderBufferGL* pColourBuffer = new RsRenderBufferGL( getContext( NULL ), ColourFormat, Width, Height );
	RsRenderBufferGL* pDepthStencilBuffer = new RsRenderBufferGL( getContext( NULL ), DepthStencilFormat, Width, Height );
	RsFrameBufferGL* pFrameBuffer = new RsFrameBufferGL( getContext( NULL ) );
	RsTextureGL* pTexture = new RsTextureGL( getContext( NULL ), Width, Height, 1, rsTF_RGBA8, NULL );

	createResource( pColourBuffer );
	createResource( pDepthStencilBuffer );
	createResource( pFrameBuffer );
	createResource( pTexture );

	// Create the render target.
	RsRenderTargetGL* pRenderTarget = new RsRenderTargetGL( getContext( NULL ), ColourFormat, DepthStencilFormat, Width, Height, pColourBuffer, pDepthStencilBuffer, pFrameBuffer, pTexture );	
	createResource( pRenderTarget );
	
	return pRenderTarget;
}

//////////////////////////////////////////////////////////////////////////
// createVertexBuffer
//virtual 
RsVertexBuffer* RsCoreImplGL::createVertexBuffer( BcU32 Descriptor, BcU32 NoofVertices, void* pVertexData )
{
	RsVertexBufferGL* pResource = new RsVertexBufferGL( getContext( NULL ), Descriptor, NoofVertices, pVertexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createIndexBuffer
//virtual 
RsIndexBuffer* RsCoreImplGL::createIndexBuffer( BcU32 NoofIndices, void* pIndexData )
{
	RsIndexBufferGL* pResource = new RsIndexBufferGL( getContext( NULL ), NoofIndices, pIndexData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createShader
//virtual
RsShader* RsCoreImplGL::createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize )
{
	RsShaderGL* pResource = new RsShaderGL( getContext( NULL ), ShaderType, ShaderDataType, pShaderData, ShaderDataSize );
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

	RsProgramGL* pResource = new RsProgramGL( getContext( NULL ), 2, &Shaders[ 0 ] );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImplGL::createProgram( BcU32 NoofShaders, RsShader** ppShaders )
{
	RsProgramGL* pResource = new RsProgramGL( getContext( NULL ), NoofShaders, ppShaders );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createPrimitive
//virtual
RsPrimitive* RsCoreImplGL::createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer )
{
	RsPrimitiveGL* pResource = new RsPrimitiveGL( getContext( NULL ), static_cast< RsVertexBufferGL* >( pVertexBuffer ), static_cast< RsIndexBufferGL* >( pIndexBuffer ) );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImplGL::destroyResource( RsResource* pResource )
{
	BcScopedLock< BcMutex > Lock( ResourceLock_ ); // HACK: Should be invoking on game thread if not in game thread!

	pResource->preDestroy();

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

