/**************************************************************************
*
* File:		RsCoreImplD3D11.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/D3D11/RsCoreImplD3D11.h"

#include "System/Renderer/D3D11/RsContextD3D11.h"

#include "System/SysKernel.h"


//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( RsCoreImplD3D11 );

//////////////////////////////////////////////////////////////////////////
// Ctor
RsCoreImplD3D11::RsCoreImplD3D11()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsCoreImplD3D11::~RsCoreImplD3D11()
{

}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void RsCoreImplD3D11::open()
{
	BcAssert( BcIsGameThread() );
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImplD3D11, &RsCoreImplD3D11::open_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
}

//////////////////////////////////////////////////////////////////////////
// open_threaded
void RsCoreImplD3D11::open_threaded()
{
#if PLATFORM_OSX
	// Do the context switch.
	OsViewOSX_Interface::MakeContextCurrent();
#endif

	// Make default context current and setup defaults.
	RsContextD3D11* pContext = static_cast< RsContextD3D11* >( ContextMap_[ NULL ] );
	if( pContext != NULL )
	{
		//
		pContext->setRenderState( rsRS_DEPTH_WRITE_ENABLE, 1, BcTrue );
		pContext->flushState();
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsCoreImplD3D11::update()
{
	BcAssert( BcIsGameThread() );
	// Increment fence so we know how far we're getting ahead of ourselves.
	RenderSyncFence_.increment();

	// Queue update job.
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImplD3D11, &RsCoreImplD3D11::update_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );

	// Wait for frames if we fall more than 1 update cycle behind.
	RenderSyncFence_.wait( 1 );
}

//////////////////////////////////////////////////////////////////////////
// update_threaded
void RsCoreImplD3D11::update_threaded()
{
	// Decrement when we've done our update.
	RenderSyncFence_.decrement();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void RsCoreImplD3D11::close()
{
	BcAssert( BcIsGameThread() );
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImplD3D11, &RsCoreImplD3D11::close_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
}

//////////////////////////////////////////////////////////////////////////
// close_threaded
void RsCoreImplD3D11::close_threaded()
{

}

//////////////////////////////////////////////////////////////////////////
// getContext
//virtual
RsContext* RsCoreImplD3D11::getContext( OsClient* pClient )
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
			RsContextD3D11* pResource = new RsContextD3D11( pClient, ContextMap_[ NULL ] );
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
void RsCoreImplD3D11::destroyContext( OsClient* pClient )
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
RsTexture* RsCoreImplD3D11::createTexture( BcU32 Width, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplD3D11::createTexture( BcU32 Width, BcU32 Height, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImplD3D11::createTexture( BcU32 Width, BcU32 Height, BcU32 Depth, BcU32 Levels, eRsTextureFormat Format, void* pData )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createRenderTarget
//virtual
RsRenderTarget*	RsCoreImplD3D11::createRenderTarget( const RsRenderTargetDesc& Desc )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createVertexBuffer
//virtual 
RsVertexBuffer* RsCoreImplD3D11::createVertexBuffer( const RsVertexBufferDesc& Desc, void* pVertexData )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createIndexBuffer
//virtual 
RsIndexBuffer* RsCoreImplD3D11::createIndexBuffer( const RsIndexBufferDesc& Desc, void* pIndexData )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createUniformBuffer
//virtual 
RsUniformBuffer* RsCoreImplD3D11::createUniformBuffer( const RsUniformBufferDesc& Desc, void* pBufferData )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createShader
//virtual
RsShader* RsCoreImplD3D11::createShader( eRsShaderType ShaderType, eRsShaderDataType ShaderDataType, void* pShaderData, BcU32 ShaderDataSize )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createProgram @deprecated
//virtual
RsProgram* RsCoreImplD3D11::createProgram( RsShader* pVertexShader, RsShader* pFragmentShader )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImplD3D11::createProgram( BcU32 NoofShaders, RsShader** ppShaders )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// createPrimitive
//virtual
RsPrimitive* RsCoreImplD3D11::createPrimitive( RsVertexBuffer* pVertexBuffer, RsIndexBuffer* pIndexBuffer )
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImplD3D11::destroyResource( RsResource* pResource )
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
// updateResource
void RsCoreImplD3D11::updateResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );
	
	// Call update.
	{
		SysSystem::UpdateDelegate Delegate( SysSystem::UpdateDelegate::bind< SysResource, &SysResource::update >( pResource ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	}
}

//////////////////////////////////////////////////////////////////////////
// createResource
void RsCoreImplD3D11::createResource( RsResource* pResource )
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
RsFrame* RsCoreImplD3D11::allocateFrame( RsContext* pContext )
{
	BcAssert( BcIsGameThread() );
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// queueFrame
void RsCoreImplD3D11::queueFrame( RsFrame* pFrame )
{
	BcAssert( BcIsGameThread() );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// queueFrame_threaded
void RsCoreImplD3D11::queueFrame_threaded( RsFrameD3D11* pFrame )
{
	// Render frame.
	//pFrame->render();

	// Now free.
	//delete pFrame;
}

