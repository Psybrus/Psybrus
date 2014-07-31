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
#include "System/Renderer/GL/RsShaderGL.h"
#include "System/Renderer/GL/RsProgramGL.h"

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
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
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
		//
		pContext->setRenderState( RsRenderStateType::DEPTH_WRITE_ENABLE, 1, BcTrue );
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
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );

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
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
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
RsTexture* RsCoreImplGL::createTexture( const RsTextureDesc& Desc, void* pData )
{
	RsTextureGL* pResource = new RsTextureGL( getContext( NULL ), Desc, pData );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createRenderTarget
//virtual
RsRenderTarget*	RsCoreImplGL::createRenderTarget( const RsRenderTargetDesc& Desc )
{
	RsRenderBufferGL* pColourBuffer = new RsRenderBufferGL( getContext( NULL ), Desc.ColourFormats_[ 0 ], Desc.Width_, Desc.Height_ );
	RsRenderBufferGL* pDepthStencilBuffer = new RsRenderBufferGL( getContext( NULL ), Desc.DepthStencilFormat_, Desc.Width_, Desc.Height_ );
	RsFrameBufferGL* pFrameBuffer = new RsFrameBufferGL( getContext( NULL ) );
	RsTextureGL* pTexture = new RsTextureGL( getContext( NULL ), RsTextureDesc( RsTextureType::TEX2D, RsTextureFormat::R8G8B8A8, 1, Desc.Width_, Desc.Height_, 0 ), NULL );

	createResource( pColourBuffer );
	createResource( pDepthStencilBuffer );
	createResource( pFrameBuffer );
	createResource( pTexture );

	// Create the render target.
	RsRenderTargetGL* pRenderTarget = new RsRenderTargetGL( getContext( NULL ), Desc, pColourBuffer, pDepthStencilBuffer, pFrameBuffer, pTexture );	
	createResource( pRenderTarget );
	
	return pRenderTarget;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
//virtual
RsVertexDeclaration* RsCoreImplGL::createVertexDeclaration( const RsVertexDeclarationDesc& Desc )
{
	RsVertexDeclaration* pResource = new RsVertexDeclaration( getContext( NULL ), Desc );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
//virtual 
RsBuffer* RsCoreImplGL::createBuffer( const RsBufferDesc& Desc )
{
	BcAssert( BcIsGameThread() );

	auto Context = getContext( nullptr );
	RsBuffer* pResource = new RsBuffer( Context, Desc );

	typedef BcDelegate< bool(*)( RsBuffer* ) > CreateDelegate;

	// Call create on render thread.
	CreateDelegate Delegate( CreateDelegate::bind< RsResourceInterface, &RsResourceInterface::createBuffer >( Context ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, pResource );
	
	// Return resource.
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createShader
//virtual
RsShader* RsCoreImplGL::createShader( RsShaderType ShaderType, RsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize )
{
	RsShaderGL* pResource = new RsShaderGL( getContext( NULL ), ShaderType, ShaderDataType, pShaderData, ShaderDataSize );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImplGL::createProgram( BcU32 NoofShaders, RsShader** ppShaders, BcU32 NoofVertexAttributes, RsProgramVertexAttribute* pVertexAttributes )
{
	RsProgramGL* pResource = new RsProgramGL( getContext( NULL ), NoofShaders, ppShaders, NoofVertexAttributes, pVertexAttributes );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImplGL::destroyResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );

	// Flush render thread before destroy.
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	// Pre destroy.
	pResource->preDestroy();

	// Call destroy and wait.
	{
		SysSystem::DestroyDelegate Delegate( SysSystem::DestroyDelegate::bind< SysResource, &SysResource::destroy >( pResource ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	}

	// Now flush to ensure it's finished being destroyed.
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImplGL::destroyResource( RsBuffer* Buffer )
{
	BcAssert( BcIsGameThread() );

	// Flush render thread before destroy.
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	typedef BcDelegate< bool(*)( RsBuffer* ) > DestroyDelegate;
	DestroyDelegate Delegate( DestroyDelegate::bind< RsCoreImplGL, &RsCoreImplGL::destroyBuffer_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Buffer );

}

bool RsCoreImplGL::destroyBuffer_threaded( 
	RsBuffer* Buffer )
{
	auto Context = Buffer->getContext();
	return Context->destroyBuffer( Buffer );
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void RsCoreImplGL::updateResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );
	
	// Call update.
	{
		SysSystem::UpdateDelegate Delegate( SysSystem::UpdateDelegate::bind< SysResource, &SysResource::update >( pResource ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	}
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsCoreImplGL::updateBuffer( 
	class RsBuffer* Buffer,
	BcSize Offset,
	BcSize Size,
	RsBufferUpdateFlags Flags,
	RsUpdateBufferFunc UpdateFunc )
{
	// Check if flags allow async.
	if( ( Flags & RsBufferUpdateFlags::ASYNC ) == RsBufferUpdateFlags::NONE )
	{
		RsCoreImplGL::UpdateBufferSync Cmd = 
		{
			Buffer,
			Offset,
			Size,
			Flags,
			nullptr // TODO: Allocate from a temporary buffer. False on failure.
		};
		UpdateBufferSyncOps_.push_back( Cmd );
		BcBreakpoint; // TODO: Implement this path.
	}
	else
	{
		RsCoreImplGL::UpdateBufferAsync Cmd =
		{
			Buffer,
			Offset,
			Size,
			Flags,
			UpdateFunc
		};

		// TODO: Push into a queue to sort and batch.
#if 1
		typedef BcDelegate< bool(*)( UpdateBufferAsync ) > UpdateDelegate;
		UpdateDelegate Delegate( UpdateDelegate::bind< RsCoreImplGL, &RsCoreImplGL::updateBuffer_threaded >( this ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Cmd );
#else
		UpdateBufferAsyncOps_.push_back( Cmd );
#endif
	}

	return true;
}
	
bool RsCoreImplGL::updateBuffer_threaded( 
	UpdateBufferAsync Cmd )
{
	auto Context = Cmd.Buffer_->getContext();
	return Context->updateBuffer( 
		Cmd.Buffer_,
		Cmd.Offset_,
		Cmd.Size_,
		Cmd.Flags_,
		Cmd.UpdateFunc_ );
}

//////////////////////////////////////////////////////////////////////////
// createResource
void RsCoreImplGL::createResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );

	// Call create.
	{
		SysSystem::CreateDelegate Delegate( SysSystem::CreateDelegate::bind< SysResource, &SysResource::create >( pResource ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
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
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, (RsFrameGL*)pFrame );
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

