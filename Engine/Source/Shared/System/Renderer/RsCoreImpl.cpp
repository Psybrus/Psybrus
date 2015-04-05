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
#include "System/Renderer/Null/RsContextNull.h"

#if PLATFORM_WINDOWS
#include "System/Renderer/D3D11/RsContextD3D11.h"
#include "System/Renderer/D3D12/RsContextD3D12.h"
#endif

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

	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ this ]
		{
#if PLATFORM_OSX
			// Do the context switch.
			OsViewOSX_Interface::MakeContextCurrent();
#endif

			// Make default context current and setup defaults.
			RsContext* pContext = ContextMap_[ NULL ];
			if( pContext != NULL )
			{

			}
		} );

	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsCoreImpl::update()
{
	BcAssert( BcIsGameThread() );

	// Increment fence so we know how far we're getting ahead of ourselves.
	RenderSyncFence_.increment();

	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ this ]
		{
			// Decrement when we've done our update.
			RenderSyncFence_.decrement();
		} );

	// Wait for frames if we fall more than 1 update cycle behind.
	RenderSyncFence_.wait( 1 );
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void RsCoreImpl::close()
{
	BcAssert( BcIsGameThread() );

	// Flush out all resources.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ this ]
		{
			for( auto ResourceDeletionFunc : ResourceDeletionList_ )
			{
				ResourceDeletionFunc();
			}
			ResourceDeletionList_.clear();
		} );

	SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	//
	destroyContext( nullptr );
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
#if PLATFORM_WINDOWS
			if( SysArgs_.find( "-d3d11" ) != std::string::npos)
			{
				pResource = new RsContextD3D11( pClient, nullptr );
			}
			else if( SysArgs_.find( "-d3d12" ) != std::string::npos)
			{
				pResource = new RsContextD3D12( pClient, nullptr );
			}
			else
#endif
			if( SysArgs_.find( "-null" ) != std::string::npos)
			{
				pResource = new RsContextNull( pClient, nullptr );
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
		auto* pResource = It->second;

		// Pre destroy.
		pResource->preDestroy();

		SysKernel::pImpl()->pushFunctionJob(
			RsCore::JOB_QUEUE_ID,
			[ pResource ]()
			{
				pResource->destroy();
				delete pResource;
			} );

		SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

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
// createRenderState
RsRenderStateUPtr RsCoreImpl::createRenderState( 
	const RsRenderStateDesc& Desc )
{
	BcAssert( BcIsGameThread() );

	auto Context = getContext( nullptr );
	RsRenderState* pResource = new RsRenderState( Context, Desc );

	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createRenderState( pResource );
		} );

	// Return resource.
	return RsRenderStateUPtr( pResource );
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
RsSamplerStateUPtr RsCoreImpl::createSamplerState( 
	const RsSamplerStateDesc& Desc )
{
	BcAssert( BcIsGameThread() );

	auto Context = getContext( nullptr );
	RsSamplerState* pResource = new RsSamplerState( Context, Desc );

	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createSamplerState( pResource );
		} );
	
	// Return resource.
	return RsSamplerStateUPtr( pResource );
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
RsFrameBufferUPtr RsCoreImpl::createFrameBuffer( 
	const RsFrameBufferDesc& Desc )
{
	BcAssert( BcIsGameThread() );

	auto Context = getContext( nullptr );
	RsFrameBuffer* pResource = new RsFrameBuffer( Context, Desc );

	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createFrameBuffer( pResource );
		} );
	
	// Return resource.
	return RsFrameBufferUPtr( pResource );
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTexture* RsCoreImpl::createTexture( const RsTextureDesc& Desc )
{
	BcAssert( BcIsGameThread() );

	auto Context = getContext( nullptr );
	RsTexture* pResource = new RsTexture( Context, Desc );

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createTexture( pResource );
		} );

	// Return resource.
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
//virtual
RsVertexDeclaration* RsCoreImpl::createVertexDeclaration( const RsVertexDeclarationDesc& Desc )
{
	auto Context = getContext( nullptr );
	RsVertexDeclaration* pResource = new RsVertexDeclaration( Context, Desc );

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createVertexDeclaration( pResource );
		} );

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

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createBuffer( pResource );
		} );
	
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
	
	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob( 
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createShader( pResource );
		} );

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

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, pResource ]
		{
			Context->createProgram( pResource );
		} );

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

	// Pre destroy.
	pResource->preDestroy();

	// Call destroy and wait.
	ResourceDeletionList_.push_back(
		[ pResource ]()
		{
			pResource->destroy();
			delete pResource;
		} );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( 
	RsRenderState* RenderState )
{
	BcAssert( BcIsGameThread() );
	if( RenderState == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ RenderState ]()
		{
			RenderState->getContext()->destroyRenderState( RenderState );
			delete RenderState;
		} );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( 
	RsSamplerState* SamplerState )
{
	BcAssert( BcIsGameThread() );
	if( SamplerState == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ SamplerState ]()
		{
			SamplerState->getContext()->destroySamplerState( SamplerState );
			delete SamplerState;
		} );
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

	ResourceDeletionList_.push_back(
		[ Buffer ]()
		{
			auto Context = Buffer->getContext();
			auto RetVal = Context->destroyBuffer( Buffer );
			delete Buffer;
			BcUnusedVar( RetVal );
		} );
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

	ResourceDeletionList_.push_back(
		[ Texture ]()
		{
			auto Context = Texture->getContext();
			auto RetVal = Context->destroyTexture( Texture );
			delete Texture;
			BcUnusedVar( RetVal );
		} );
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

	ResourceDeletionList_.push_back(
		[ Shader ]()
		{
			auto Context = Shader->getContext();
			auto RetVal = Context->destroyShader( Shader );
			delete Shader;
			BcUnusedVar( RetVal );
		} );
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

	ResourceDeletionList_.push_back(
		[ Program ]()
		{
			auto Context = Program->getContext();
			auto RetVal = Context->destroyProgram( Program );
			delete Program;
			BcUnusedVar( RetVal );
		} );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( 
		RsVertexDeclaration* VertexDeclaration )
{
	BcAssert( BcIsGameThread() );
	if( VertexDeclaration == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ VertexDeclaration ]()
		{
			auto Context = VertexDeclaration->getContext();
			auto RetVal = Context->destroyVertexDeclaration( VertexDeclaration );
			delete VertexDeclaration;
			BcUnusedVar( RetVal );
		} );
}

//////////////////////////////////////////////////////////////////////////
// updateResource
void RsCoreImpl::updateResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );
	BcAssert( pResource != nullptr );
	
	// Call update.
	SysKernel::pImpl()->pushFunctionJob( 
		RsCore::JOB_QUEUE_ID, 
		[ pResource ]()
		{
			pResource->update();
		} );
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

		SysKernel::pImpl()->pushFunctionJob( 
			RsCore::JOB_QUEUE_ID,
			[ Cmd ]()
			{
				auto Context = Cmd.Buffer_->getContext();
				Context->updateBuffer( 
					Cmd.Buffer_,
					Cmd.Offset_,
					Cmd.Size_,
					Cmd.Flags_,
					Cmd.UpdateFunc_ );
			} );
	}

	return true;
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
	BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) == RsResourceBindFlags::NONE );
	BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) == RsResourceBindFlags::NONE );

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

		SysKernel::pImpl()->pushFunctionJob( 
			RsCore::JOB_QUEUE_ID,
			[ Cmd ]()
			{
				auto Context = Cmd.Texture_->getContext();
				Context->updateTexture( 
					Cmd.Texture_,
					Cmd.Slice_,
					Cmd.Flags_,
					Cmd.UpdateFunc_ );
			} );

	}

	return true;
}
	
//////////////////////////////////////////////////////////////////////////
// createResource
void RsCoreImpl::createResource( RsResource* pResource )
{
	BcAssert( BcIsGameThread() );
	BcAssert( pResource != nullptr );

	// Call create.
	SysKernel::pImpl()->pushFunctionJob( 
		RsCore::JOB_QUEUE_ID, 
		[ pResource ]()
		{
			pResource->create();
		} );
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
	SysKernel::pImpl()->pushFunctionJob( 
		RsCore::JOB_QUEUE_ID, 
		[ pFrame ]()
		{
			// Render frame.
			pFrame->render();

			// Now free.
			delete pFrame;
		} );

	for( auto ResourceDeletionFunc : ResourceDeletionList_ )
	{
		SysKernel::pImpl()->pushFunctionJob( RsCore::JOB_QUEUE_ID, ResourceDeletionFunc );
	}

	ResourceDeletionList_.clear();
}

