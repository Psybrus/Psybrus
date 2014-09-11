/**************************************************************************
*
* File:		RsCoreImpl.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsCoreImpl.h"

#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsProgram.h"

#include "System/SysKernel.h"

#include "System/Renderer/GL/RsContextGL.h"
#include "System/Renderer/D3D11/RsContextD3D11.h"

#include "Psybrus.h" // cyclic.

//////////////////////////////////////////////////////////////////////////
// Creator
SYS_CREATOR( RsCoreImpl );

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( RsCoreImpl );

void RsCoreImpl::StaticRegisterClass()
{
	ReRegisterClass< RsCoreImpl, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsCoreImpl::RsCoreImpl()
{
	// Create our job queue.
	// - 1 thread if we have 2 or more hardware threads.
	RsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 2 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsCoreImpl::~RsCoreImpl()
{

}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void RsCoreImpl::open()
{
	BcAssert( BcIsGameThread() );
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImpl, &RsCoreImpl::open_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
}

//////////////////////////////////////////////////////////////////////////
// open_threaded
void RsCoreImpl::open_threaded()
{
#if PLATFORM_OSX
	// Do the context switch.
	OsViewOSX_Interface::MakeContextCurrent();
#endif

	// Make default context current and setup defaults.
	RsContext* pContext = ContextMap_[ NULL ];
	if( pContext != NULL )
	{
		//
		pContext->setRenderState( RsRenderStateType::DEPTH_WRITE_ENABLE, 1, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsCoreImpl::update()
{
	BcAssert( BcIsGameThread() );
	// Increment fence so we know how far we're getting ahead of ourselves.
	RenderSyncFence_.increment();

	// Queue update job.
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImpl, &RsCoreImpl::update_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );

	// Wait for frames if we fall more than 1 update cycle behind.
	RenderSyncFence_.wait( 1 );
}

//////////////////////////////////////////////////////////////////////////
// update_threaded
void RsCoreImpl::update_threaded()
{
	// Decrement when we've done our update.
	RenderSyncFence_.decrement();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void RsCoreImpl::close()
{
	BcAssert( BcIsGameThread() );
	BcDelegate< void(*)() > Delegate( BcDelegate< void(*)() >::bind< RsCoreImpl, &RsCoreImpl::close_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	destroyContext( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// close_threaded
void RsCoreImpl::close_threaded()
{
	
}

//////////////////////////////////////////////////////////////////////////
// getContext
//virtual
RsContext* RsCoreImpl::getContext( OsClient* pClient )
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
			RsContext* pResource = nullptr;
			if( SysArgs_.find( "-d3d11" ) != std::string::npos)
			{
				pResource = new RsContextD3D11( pClient, nullptr );
			}
			else
			{
				pResource = new RsContextGL( pClient, nullptr );
			}
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
void RsCoreImpl::destroyContext( OsClient* pClient )
{
	BcAssert( BcIsGameThread() );
	TContextMapIterator It = ContextMap_.find( pClient );

	if( It != ContextMap_.end() )
	{
		// Destory resource.
		destroyResource( It->second );

		// If we're destroying the default context, NULL it.
		if( ContextMap_[ nullptr ] == It->second )
		{
			ContextMap_[ nullptr ] = nullptr;
		}

		// Erase from context map.
		ContextMap_.erase( It );
	}
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImpl::createTexture( const RsTextureDesc& Desc )
{
	BcAssert( BcIsGameThread() );

	auto Context = getContext( nullptr );
	RsTexture* pResource = new RsTexture( Context, Desc );

	typedef BcDelegate< bool(*)( RsTexture* ) > CreateDelegate;

	// Call create on render thread.
	CreateDelegate Delegate( CreateDelegate::bind< RsResourceInterface, &RsResourceInterface::createTexture >( Context ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, pResource );
	
	// Return resource.
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
//virtual
RsVertexDeclaration* RsCoreImpl::createVertexDeclaration( const RsVertexDeclarationDesc& Desc )
{
	RsVertexDeclaration* pResource = new RsVertexDeclaration( getContext( NULL ), Desc );
	createResource( pResource );
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
//virtual 
RsBuffer* RsCoreImpl::createBuffer( const RsBufferDesc& Desc )
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
RsShader* RsCoreImpl::createShader( const RsShaderDesc& Desc, void* pShaderData, BcU32 ShaderDataSize )
{
	auto Context = getContext( nullptr );
	RsShader* pResource = new RsShader( Context, Desc, pShaderData, ShaderDataSize );
	
	typedef BcDelegate< bool(*)( RsShader* ) > CreateDelegate;

	// Call create on render thread.
	CreateDelegate Delegate( CreateDelegate::bind< RsResourceInterface, &RsResourceInterface::createShader >( Context ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, pResource );

	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgram* RsCoreImpl::createProgram( 
	std::vector< RsShader* > Shaders, 
	RsProgramVertexAttributeList VertexAttributes )
{
	auto Context = getContext( nullptr );

	BcAssert( Shaders.size() > 0 );

	RsProgram* pResource = new RsProgram(
		Context, 
		std::move( Shaders ), 
		std::move( VertexAttributes ) );

	typedef BcDelegate< bool(*)( RsProgram* ) > CreateDelegate;

	// Call create on render thread.
	CreateDelegate Delegate( CreateDelegate::bind< RsResourceInterface, &RsResourceInterface::createProgram >( Context ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, pResource );

	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );
	if( pResource == nullptr )
	{
		return;
	}

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
void RsCoreImpl::destroyResource( RsBuffer* Buffer )
{
	BcAssert( BcIsGameThread() );
	if( Buffer == nullptr )
	{
		return;
	}

	// Flush render thread before destroy.
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	typedef BcDelegate< bool(*)( RsBuffer* ) > DestroyDelegate;
	DestroyDelegate Delegate( DestroyDelegate::bind< RsCoreImpl, &RsCoreImpl::destroyBuffer_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Buffer );
}

bool RsCoreImpl::destroyBuffer_threaded( 
	RsBuffer* Buffer )
{
	auto Context = Buffer->getContext();
	auto retVal = Context->destroyBuffer( Buffer );
	delete Buffer;
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsTexture* Texture )
{
	BcAssert( BcIsGameThread() );
	if( Texture == nullptr )
	{
		return;
	}

	// Flush render thread before destroy.
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	typedef BcDelegate< bool(*)( RsTexture* ) > DestroyDelegate;
	DestroyDelegate Delegate( DestroyDelegate::bind< RsCoreImpl, &RsCoreImpl::destroyTexture_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Texture );

}

bool RsCoreImpl::destroyTexture_threaded( 
	RsTexture* Texture )
{
	auto Context = Texture->getContext();
	auto retVal = Context->destroyTexture( Texture );
	delete Texture;
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( 
		RsShader* Shader )
{
	BcAssert( BcIsGameThread() );
	if( Shader == nullptr )
	{
		return;
	}

	// Flush render thread before destroy.
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	typedef BcDelegate< bool(*)( RsShader* ) > DestroyDelegate;
	DestroyDelegate Delegate( DestroyDelegate::bind< RsCoreImpl, &RsCoreImpl::destroyShader_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Shader );

}

bool RsCoreImpl::destroyShader_threaded( 
		RsShader* Shader )
{
	auto Context = Shader->getContext();
	auto retVal = Context->destroyShader( Shader );
	delete Shader;
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( 
		RsProgram* Program )
{
	BcAssert( BcIsGameThread() );
	if( Program == nullptr )
	{
		return;
	}


	// Flush render thread before destroy.
	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	typedef BcDelegate< bool(*)( RsProgram* ) > DestroyDelegate;
	DestroyDelegate Delegate( DestroyDelegate::bind< RsCoreImpl, &RsCoreImpl::destroyProgram_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Program );
}

bool RsCoreImpl::destroyProgram_threaded( 
		RsProgram* Program )
{
	auto Context = Program->getContext();
	auto retVal = Context->destroyProgram( Program );
	delete Program;
	return retVal;
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void RsCoreImpl::updateResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );
	BcAssert( pResource != nullptr );
	
	// Call update.
	{
		SysSystem::UpdateDelegate Delegate( SysSystem::UpdateDelegate::bind< SysResource, &SysResource::update >( pResource ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	}
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsCoreImpl::updateBuffer( 
	class RsBuffer* Buffer,
	BcSize Offset,
	BcSize Size,
	RsResourceUpdateFlags Flags,
	RsBufferUpdateFunc UpdateFunc )
{
	BcAssert( Size > 0 );
	BcAssert( Buffer != nullptr );

	// Check if flags allow async.
	if( ( Flags & RsResourceUpdateFlags::ASYNC ) == RsResourceUpdateFlags::NONE )
	{
		BcBreakpoint; // TODO: Implement this path?
	}
	else
	{
		RsCoreImpl::UpdateBufferAsync Cmd =
		{
			Buffer,
			Offset,
			Size,
			Flags,
			UpdateFunc
		};

		typedef BcDelegate< bool(*)( UpdateBufferAsync ) > UpdateDelegate;
		UpdateDelegate Delegate( UpdateDelegate::bind< RsCoreImpl, &RsCoreImpl::updateBuffer_threaded >( this ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Cmd );
	}

	return true;
}
	
bool RsCoreImpl::updateBuffer_threaded( 
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
// updateTexture
bool RsCoreImpl::updateTexture( 
	class RsTexture* Texture,
	const RsTextureSlice& Slice,
	RsResourceUpdateFlags Flags,
	RsTextureUpdateFunc UpdateFunc )
{
	BcAssert( Texture != nullptr );

	// Check if flags allow async.
	if( ( Flags & RsResourceUpdateFlags::ASYNC ) == RsResourceUpdateFlags::NONE )
	{
		BcBreakpoint; // TODO: Implement this path?
	}
	else
	{
		RsCoreImpl::UpdateTextureAsync Cmd =
		{
			Texture,
			Slice,
			Flags,
			UpdateFunc
		};

		typedef BcDelegate< bool(*)( UpdateTextureAsync ) > UpdateDelegate;
		UpdateDelegate Delegate( UpdateDelegate::bind< RsCoreImpl, &RsCoreImpl::updateTexture_threaded >( this ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, Cmd );
	}

	return true;
}
	
bool RsCoreImpl::updateTexture_threaded( 
	UpdateTextureAsync Cmd )
{
	auto Context = Cmd.Texture_->getContext();
	return Context->updateTexture( 
		Cmd.Texture_,
		Cmd.Slice_,
		Cmd.Flags_,
		Cmd.UpdateFunc_ );
}

//////////////////////////////////////////////////////////////////////////
// createResource
void RsCoreImpl::createResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );
	BcAssert( pResource != nullptr );

	// Call create.
	{
		SysSystem::CreateDelegate Delegate( SysSystem::CreateDelegate::bind< SysResource, &SysResource::create >( pResource ) );
		SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate );
	}
}

//////////////////////////////////////////////////////////////////////////
// allocateFrame
RsFrame* RsCoreImpl::allocateFrame( RsContext* pContext )
{
	BcAssert( BcIsGameThread() );
	if( pContext != NULL )
	{
		return new RsFrame( pContext );
	}
	else
	{
		return new RsFrame( ContextMap_[ NULL ] );
	}	
}

//////////////////////////////////////////////////////////////////////////
// queueFrame
void RsCoreImpl::queueFrame( RsFrame* pFrame )
{
	BcAssert( BcIsGameThread() );
	BcDelegate< void(*)( RsFrame* ) > Delegate( BcDelegate< void(*)( RsFrame* ) >::bind< RsCoreImpl, &RsCoreImpl::queueFrame_threaded >( this ) );
	SysKernel::pImpl()->pushDelegateJob( RsCore::JOB_QUEUE_ID, Delegate, (RsFrame*)pFrame );
}

//////////////////////////////////////////////////////////////////////////
// queueFrame_threaded
void RsCoreImpl::queueFrame_threaded( RsFrame* pFrame )
{
	// Render frame.
	pFrame->render();

	// Now free.
	delete pFrame;
}

