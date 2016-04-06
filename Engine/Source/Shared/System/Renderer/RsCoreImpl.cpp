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

#include "Base/BcProfiler.h"

#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsProgram.h"

#include "System/SysKernel.h"

#include "System/Renderer/Null/RsContextNull.h"

#if WITH_GL
#include "System/Renderer/GL/RsContextGL.h"
#endif

#if WITH_DX12
#include "System/Renderer/D3D12/RsContextD3D12.h"
#endif

#if WITH_VK
#include "System/Renderer/VK/RsContextVK.h"
#endif

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
RsCoreImpl::RsCoreImpl():
	FrameTime_( 0.0f )
{
	if( GCommandLine_.hasArg( '\0', "norenderthread" ) )
	{
		RsCore::JOB_QUEUE_ID = -1;
	}
	else
	{
		// Create our job queue.
		// - 1 thread if we have 2 or more hardware threads.
		RsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 2 );
	}
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
	PSY_PROFILER_SECTION( UpdateRoot, "RsCoreImpl::update" );

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

			// Order of priority:
			// - OpenGL.
			// - D3D12. (disable, unstable)
			// - Vulkan. (disabled, incomplete)
			RsAPI API = RsAPI::OPENGL;

			if( GCommandLine_.hasArg( '\0', "null_renderer" ) )
			{
				API = RsAPI::NULL_RENDERER;
			}
			else if( GCommandLine_.hasArg( '\0', "d3d12" ) )
			{
				API = RsAPI::D3D12;
			}
			else if( GCommandLine_.hasArg( '\0', "vk" ) )
			{
				API = RsAPI::VULKAN;
			}
			else if( GCommandLine_.hasArg( '\0', "gl" ) || GCommandLine_.hasArg( '\0', "gles" )  )
			{
				API = RsAPI::OPENGL;
			}

			switch( API )
			{
			case RsAPI::NULL_RENDERER:
				pResource = new RsContextNull( pClient, nullptr );
				break;
#if WITH_GL
			case RsAPI::OPENGL:
				pResource = new RsContextGL( pClient, nullptr );
				break;
#endif
#if WITH_DX12
			case RsAPI::D3D12:
				pResource = new RsContextD3D12( pClient, nullptr );
				break;
#endif
#if WITH_VK
			case RsAPI::VULKAN:
				pResource = new RsContextVK( pClient, nullptr );
				break;
#endif
			default:
				BcAssertMsg( false, "Invalid renderer selected. Check it has been built in, or valid for platform." );
				break;
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
		const RsRenderStateDesc& Desc, 
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );
	RsRenderStateUPtr Resource( new RsRenderState( Context, Desc ) );
	Resource->setDebugName( DebugName );

	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createRenderState( Resource ) )
			{
				PSY_LOG( "Failed RsRenderState creation: %s", Resource->getDebugName() );
			}
		} );

	// Return resource.
	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
RsSamplerStateUPtr RsCoreImpl::createSamplerState( 
		const RsSamplerStateDesc& Desc, 
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );
	RsSamplerStateUPtr Resource( new RsSamplerState( Context, Desc ) );
	Resource->setDebugName( DebugName );

	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createSamplerState( Resource ) )
			{
				PSY_LOG( "Failed RsSamplerState creation: %s", Resource->getDebugName() );
			}
		} );
	
	// Return resource.
	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
RsFrameBufferUPtr RsCoreImpl::createFrameBuffer( 
		const RsFrameBufferDesc& Desc, 
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );
	RsFrameBufferUPtr Resource( new RsFrameBuffer( Context, Desc ) );
	Resource->setDebugName( DebugName );

	SysFence Fence( 1 );
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get(), &Fence ]
		{
			if( !Context->createFrameBuffer( Resource ) )
			{
				PSY_LOG( "Failed RsFrameBuffer creation: %s", Resource->getDebugName() );
			}
			Fence.decrement();
		} );
	Fence.wait();

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AliveFrameBuffers_.insert( Resource.get() );
#endif
	
	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
//virtual 
RsTextureUPtr RsCoreImpl::createTexture( 
		const RsTextureDesc& Desc, 
	const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );
	const auto& Features = Context->getFeatures();

	// Check if format is supported one.
	if( !Features.TextureFormat_[ (int)Desc.Format_ ] )
	{
		PSY_LOG( "ERROR: No support for %u format.", Desc.Format_ );
		return nullptr;
	}

	RsTextureUPtr Resource( new RsTexture( Context, Desc ) ); 
	Resource->setDebugName( DebugName );

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createTexture( Resource ) )
			{
				PSY_LOG( "Failed RsTexture creation: %s", Resource->getDebugName() );
			}
		} );

	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
//virtual
RsVertexDeclarationUPtr RsCoreImpl::createVertexDeclaration( 
		const RsVertexDeclarationDesc& Desc, 
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );
	RsVertexDeclarationUPtr Resource( new RsVertexDeclaration( Context, Desc ) );
	Resource->setDebugName( DebugName );

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createVertexDeclaration( Resource ) )
			{
				PSY_LOG( "Failed RsVertexDeclaration creation: %s", Resource->getDebugName() );
			}
		} );

	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
//virtual 
RsBufferUPtr RsCoreImpl::createBuffer( 
		const RsBufferDesc& Desc, 
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );
	RsBufferUPtr Resource( new RsBuffer( Context, Desc ) );
	Resource->setDebugName( DebugName );

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createBuffer( Resource ) )
			{
				PSY_LOG( "Failed RsBuffer creation: %s", Resource->getDebugName() );
			}
		} );
	
	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createShader
//virtual
RsShaderUPtr RsCoreImpl::createShader( 
		const RsShaderDesc& Desc, 
		void* pShaderData, BcU32 ShaderDataSize,
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );
	RsShaderUPtr Resource( new RsShader( Context, Desc, pShaderData, ShaderDataSize ) );
	Resource->setDebugName( DebugName );
	
	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob( 
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createShader( Resource ) )
			{
				PSY_LOG( "Failed RsShader creation: %s", Resource->getDebugName() );
			}
		} );

	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
//virtual
RsProgramUPtr RsCoreImpl::createProgram( 
		std::vector< RsShader* > Shaders, 
		RsProgramVertexAttributeList VertexAttributes,
		RsProgramParameterList ParameterList,
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );

	auto Context = getContext( nullptr );

	BcAssert( Shaders.size() > 0 );
	
	RsProgramUPtr Resource( new RsProgram(
		Context, 
		std::move( Shaders ), 
		std::move( VertexAttributes ),
		std::move( ParameterList ) ) );
	Resource->setDebugName( DebugName );

	// Call create on render thread.
	SysFence Fence;
	Fence.increment();
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get(), &Fence ]
		{
			if( !Context->createProgram( Resource ) )
			{
				PSY_LOG( "Failed RsProgram creation: %s", Resource->getDebugName() );
			}
			Fence.decrement();
		} );
	Fence.wait();

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AlivePrograms_.insert( Resource.get() );
#endif

	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createProgramBinding
//virtual
RsProgramBindingUPtr RsCoreImpl::createProgramBinding( 
		RsProgram* Program,
		const RsProgramBindingDesc& ProgramBindingDesc,
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );
	auto Context = getContext( nullptr );

	BcAssert( Program );

	RsProgramBindingUPtr Resource( new RsProgramBinding(
		Context, 
		Program,
		ProgramBindingDesc ) );
	Resource->setDebugName( DebugName );

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createProgramBinding( Resource ) )
			{
				PSY_LOG( "Failed RsProgramBinding creation: %s", Resource->getDebugName() );
			}
		} );

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AliveProgramBindings_.insert( Resource.get() );
#endif
	return Resource;
}

//////////////////////////////////////////////////////////////////////////
// createGeometryBinding
//virtual
RsGeometryBindingUPtr RsCoreImpl::createGeometryBinding( 
		const RsGeometryBindingDesc& GeometryBindingDesc,
		const BcChar* DebugName )
{
	BcAssert( BcIsGameThread() );
	BcAssert( DebugName && DebugName[0] != '\0' );
	auto Context = getContext( nullptr );

	RsGeometryBindingUPtr Resource( new RsGeometryBinding(
		Context, 
		GeometryBindingDesc ) );
	Resource->setDebugName( DebugName );

	// Call create on render thread.
	SysKernel::pImpl()->pushFunctionJob(
		RsCore::JOB_QUEUE_ID,
		[ Context, Resource = Resource.get() ]
		{
			if( !Context->createGeometryBinding( Resource ) )
			{
				PSY_LOG( "Failed RsGeometryBinding creation: %s", Resource->getDebugName() );
			}
		} );

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AliveGeometryBindings_.insert( Resource.get() );
#endif
	return Resource;
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
void RsCoreImpl::destroyResource( RsRenderState* RenderState )
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
void RsCoreImpl::destroyResource( RsSamplerState* SamplerState )
{
	BcAssert( BcIsGameThread() );
	if( SamplerState == nullptr )
	{
		return;
	}


	ResourceDeletionList_.push_back(
		[ this, SamplerState ]()
		{
#if !PSY_PRODUCTION
			std::lock_guard< std::mutex > Lock( AliveLock_ );
			for( const auto* ProgramBinding : AliveProgramBindings_ )
			{
				const auto& Desc = ProgramBinding->getDesc();
				for( const auto& BindingSamplerState : Desc.SamplerStates_ )
				{
					BcAssertMsg( BindingSamplerState != SamplerState, "RsSamplerState %s is currently being used in RsProgramBinding %s.",
						SamplerState->getDebugName(),
						ProgramBinding->getDebugName() );
				}
			}
#endif

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
		[ this, Buffer ]()
		{
#if !PSY_PRODUCTION
			std::lock_guard< std::mutex > Lock( AliveLock_ );
			for( const auto* GeometryBinding : AliveGeometryBindings_ )
			{
				const auto& Desc = GeometryBinding->getDesc();
				for( const auto& VertexBufferBinding : Desc.VertexBuffers_ )
				{
					BcAssertMsg( VertexBufferBinding.Buffer_ != Buffer, "RsBuffer %s is currently being used in RsGeometryBinding %s.", 
						Buffer->getDebugName(),
						GeometryBinding->getDebugName() );
				}
			}

			for( const auto* ProgramBinding : AliveProgramBindings_ )
			{
				const auto& Desc = ProgramBinding->getDesc();
				for( const auto& SRVSlot : Desc.ShaderResourceSlots_ )
				{
					BcAssertMsg( SRVSlot.Buffer_ != Buffer, "RsBuffer %s is currently being used in RsProgramBinding %s as a SRV.", 
						Buffer->getDebugName(),
						ProgramBinding->getDebugName() );
				}

				for( const auto& UAVSlot : Desc.UnorderedAccessSlots_ )
				{
					BcAssertMsg( UAVSlot.Buffer_ != Buffer, "RsBuffer %s is currently being used in RsProgramBinding %s as a UAV.", 
						Buffer->getDebugName(),
						ProgramBinding->getDebugName() );
				}

				for( const auto& UniformBuffer : Desc.UniformBuffers_ )
				{
					BcAssertMsg( UniformBuffer.Buffer_ != Buffer, "RsBuffer %s is currently being used in RsProgramBinding %s as a uniform buffer.", 
						Buffer->getDebugName(),
						ProgramBinding->getDebugName() );
				}
			}
#endif
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
		[ this, Texture ]()
		{
#if !PSY_PRODUCTION
			std::lock_guard< std::mutex > Lock( AliveLock_ );
			for( const auto* FrameBuffer : AliveFrameBuffers_ )
			{
				const auto& Desc = FrameBuffer->getDesc();
				for( const auto& RTTexture : Desc.RenderTargets_ )
				{
					BcAssertMsg( RTTexture != Texture, "RsTexture %s is currently being used in RsFrameBuffer %s.", 
						Texture->getDebugName(),
						FrameBuffer->getDebugName() );
				}
				BcAssertMsg( Desc.DepthStencilTarget_ != Texture, "RsTexture %s is currently being used in RsFrameBuffer %s.", 
					Texture->getDebugName(),
					FrameBuffer->getDebugName() );
			}

			for( const auto* ProgramBinding : AliveProgramBindings_ )
			{
				const auto& Desc = ProgramBinding->getDesc();
				for( const auto& SRVSlot : Desc.ShaderResourceSlots_ )
				{
					BcAssertMsg( SRVSlot.Texture_ != Texture, "RsTexture %s is currently being used in RsProgramBinding %s as a SRV.", 
						Texture->getDebugName(),
						ProgramBinding->getDebugName() );
				}

				for( const auto& UAVSlot : Desc.UnorderedAccessSlots_ )
				{
					BcAssertMsg( UAVSlot.Texture_ != Texture, "RsTexture %s is currently being used in RsProgramBinding %s as a UAV.", 
						Texture->getDebugName(),
						ProgramBinding->getDebugName() );
				}
			}
#endif

			auto Context = Texture->getContext();
				auto RetVal = Context->destroyTexture( Texture );
				delete Texture;
				BcUnusedVar( RetVal );
		} );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsFrameBuffer* FrameBuffer )
{
	BcAssert( BcIsGameThread() );
	if( FrameBuffer == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ FrameBuffer ]()
		{
			auto Context = FrameBuffer->getContext();
			auto RetVal = Context->destroyFrameBuffer( FrameBuffer );
			delete FrameBuffer;
			BcUnusedVar( RetVal );
		} );

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AliveFrameBuffers_.erase( FrameBuffer );
#endif
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsShader* Shader )
{
	BcAssert( BcIsGameThread() );
	if( Shader == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ this, Shader ]()
		{
#if !PSY_PRODUCTION
			std::lock_guard< std::mutex > Lock( AliveLock_ );
			for( const auto* Program : AlivePrograms_ )
			{
				const auto& ProgramShaders = Program->getShaders();
				for( const auto& ProgramShader : ProgramShaders )
				{
					BcAssertMsg( ProgramShader != Shader, "RsShader %s is currently being used in RsProgram %s.", 
						Shader->getDebugName(),
						Program->getDebugName() );
				}
			}
#endif
			auto Context = Shader->getContext();
			auto RetVal = Context->destroyShader( Shader );
			delete Shader;
			BcUnusedVar( RetVal );
		} );
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsProgram* Program )
{
	BcAssert( BcIsGameThread() );
	if( Program == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ this, Program ]()
		{
#if !PSY_PRODUCTION
			std::lock_guard< std::mutex > Lock( AliveLock_ );
			for( const auto* ProgramBinding : AliveProgramBindings_ )
			{
				BcAssertMsg( ProgramBinding->getProgram() != Program, "RsProgram is currently being used in RsProgramBinding %s.", ProgramBinding->getDebugName() );
			}
#endif
			auto Context = Program->getContext();
			auto RetVal = Context->destroyProgram( Program );
			delete Program;
			BcUnusedVar( RetVal );
		} );

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AlivePrograms_.erase( Program );
#endif
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsProgramBinding* ProgramBinding )
{
	BcAssert( BcIsGameThread() );
	if( ProgramBinding == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ ProgramBinding ]()
		{
			auto Context = ProgramBinding->getContext();
			auto RetVal = Context->destroyProgramBinding( ProgramBinding );
			delete ProgramBinding;
			BcUnusedVar( RetVal );
		} );

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AliveProgramBindings_.erase( ProgramBinding );
#endif
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsGeometryBinding* GeometryBinding )
{
	BcAssert( BcIsGameThread() );
	if( GeometryBinding == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ GeometryBinding ]()
		{
			auto Context = GeometryBinding->getContext();
			auto RetVal = Context->destroyGeometryBinding( GeometryBinding );
			delete GeometryBinding;
			BcUnusedVar( RetVal );
		} );

#if !PSY_PRODUCTION
	std::lock_guard< std::mutex > Lock( AliveLock_ );
	AliveGeometryBindings_.erase( GeometryBinding );
#endif
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void RsCoreImpl::destroyResource( RsVertexDeclaration* VertexDeclaration )
{
	BcAssert( BcIsGameThread() );
	if( VertexDeclaration == nullptr )
	{
		return;
	}

	ResourceDeletionList_.push_back(
		[ this, VertexDeclaration ]()
		{
#if !PSY_PRODUCTION
			std::lock_guard< std::mutex > Lock( AliveLock_ );
			for( const auto* GeometryBinding : AliveGeometryBindings_ )
			{
				const auto& Desc = GeometryBinding->getDesc();
				BcAssertMsg( Desc.VertexDeclaration_ != VertexDeclaration, "RsVertexDeclaration is currently being used in RsGeometryBinding %s.", GeometryBinding->getDebugName() );
			}
#endif
	
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
	BcAssert( Buffer != nullptr );
	BcAssert( Size >= 0 && ( Size + Offset ) <= Buffer->getDesc().SizeBytes_ );

	if( Size == 0 )
	{
		Size = Buffer->getDesc().SizeBytes_;
	}

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
		[ this, pFrame ]()
		{
			BcTimer Timer;

			Timer.mark();

			// Render frame.
			pFrame->render();

			// Now free.
			delete pFrame;

			// Grab frame time.
			FrameTime_ = Timer.time();
		} );

	for( auto ResourceDeletionFunc : ResourceDeletionList_ )
	{
		SysKernel::pImpl()->pushFunctionJob( RsCore::JOB_QUEUE_ID, ResourceDeletionFunc );
	}

	ResourceDeletionList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// getFrameTime
BcF64 RsCoreImpl::getFrameTime() const
{
	return FrameTime_;
}
