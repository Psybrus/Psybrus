#include "System/Renderer/RsCore.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "System/Debug/DsCore.h"

#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsRenderNode.h"

#include "System/Scene/Rendering/ScnPostProcessComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// ScnPostProcessVertex
REFLECTION_DEFINE_BASIC( ScnPostProcessVertex );

void ScnPostProcessVertex::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Position_", &ScnPostProcessVertex::Position_ ),
		new ReField( "UV_", &ScnPostProcessVertex::UV_ ),
	};
	ReRegisterClass< ScnPostProcessVertex >( Fields );
}

ScnPostProcessVertex::ScnPostProcessVertex()
{

}

ScnPostProcessVertex::ScnPostProcessVertex( const MaVec4d& Position, const MaVec2d& UV ):
	Position_( Position ),
	UV_( UV )
{

}

//////////////////////////////////////////////////////////////////////////
// ScnPostProcessUniforms
REFLECTION_DEFINE_BASIC( ScnPostProcessUniforms );

void ScnPostProcessUniforms::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Name_", &ScnPostProcessUniforms::Name_ ),
		new ReField( "Data_", &ScnPostProcessUniforms::Data_ ),
		new ReField( "Buffer_", &ScnPostProcessUniforms::Buffer_, bcRFF_TRANSIENT ),
	};
	ReRegisterClass< ScnPostProcessUniforms >( Fields );
}

ScnPostProcessUniforms::ScnPostProcessUniforms():
	Name_( "" ),
	Data_(),
	Buffer_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// ScnPostProcessTexture
REFLECTION_DEFINE_BASIC( ScnPostProcessNode );

void ScnPostProcessNode::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "InputTextures_", &ScnPostProcessNode::InputTextures_, bcRFF_SHALLOW_COPY ),
		new ReField( "InputSamplers_", &ScnPostProcessNode::InputSamplers_ ),
		new ReField( "OutputTextures_", &ScnPostProcessNode::OutputTextures_, bcRFF_SHALLOW_COPY ),
		new ReField( "Shader_", &ScnPostProcessNode::Shader_, bcRFF_SHALLOW_COPY ),
		new ReField( "RenderState_", &ScnPostProcessNode::RenderState_ ),
		new ReField( "Uniforms_", &ScnPostProcessNode::Uniforms_ ),
	};
	auto& Class = ReRegisterClass< ScnPostProcessNode >( Fields );

	// Add editor.
	Class.addAttribute( 
		new DsImGuiFieldEditor( 
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				auto Node = static_cast< ScnPostProcessNode* >( Object );
				for( auto& Uniforms : Node->Uniforms_ )
				{
					DsCore::pImpl()->drawObjectEditor( 
						ThisFieldEditor, 
						Uniforms.Data_.getData< BcU8 >(),
						ReManager::GetClass( Uniforms.Name_ ), Flags );
				}

				// Defaults.
				DsCore::pImpl()->drawObjectEditor( ThisFieldEditor, Object, Class, Flags );
			} ) );
}

ScnPostProcessNode::ScnPostProcessNode()
{

}

//////////////////////////////////////////////////////////////////////////
// ScnPostProcessComponent
REFLECTION_DEFINE_DERIVED( ScnPostProcessComponent );

void ScnPostProcessComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Input_", &ScnPostProcessComponent::Input_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Output_", &ScnPostProcessComponent::Output_, bcRFF_IMPORTER | bcRFF_SHALLOW_COPY ),
		new ReField( "Nodes_", &ScnPostProcessComponent::Nodes_, bcRFF_IMPORTER ),

		//new ReField( "FrameBuffers_", &ScnPostProcessComponent::FrameBuffers_, bcRFF_TRANSIENT ),
		//new ReField( "RenderStates_", &ScnPostProcessComponent::RenderStates_, bcRFF_TRANSIENT ),
		//new ReField( "SamplerStates_", &ScnPostProcessComponent::SamplerStates_, bcRFF_TRANSIENT ),
		//new ReField( "VertexDeclaration_", &ScnPostProcessComponent::VertexDeclaration_, bcRFF_TRANSIENT ),
		//new ReField( "VertexBuffer_", &ScnPostProcessComponent::VertexBuffer_, bcRFF_TRANSIENT ),
		//new ReField( "UniformBuffers_", &ScnPostProcessComponent::UniformBuffers_, bcRFF_TRANSIENT ),
	};
	
	ReRegisterClass< ScnPostProcessComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPostProcessComponent::ScnPostProcessComponent():
	Input_(),
	Nodes_(),
	VertexDeclaration_(),
	VertexBuffer_()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
ScnPostProcessComponent::~ScnPostProcessComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnPostProcessComponent::onAttach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
		[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
		{
			recreateResources();
			return evtRET_PASS;
		} );

	recreateResources();

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnPostProcessComponent::onDetach( ScnEntityWeakRef Parent )
{
	RenderFence_.wait();

	OsCore::pImpl()->unsubscribeAll( this );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getAABB
MaAABB ScnPostProcessComponent::getAABB() const
{
	return MaAABB();
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnPostProcessComponent::render( ScnRenderContext& RenderContext )
{
	auto* InputFrameBuffer = RenderContext.View_->getFrameBuffer();
	RsRenderSort Sort = RenderContext.Sort_;
	Sort.Layer_ = 0;
	RenderFence_.increment();
	RenderContext.pFrame_->queueRenderNode( Sort,
		[ 
			this, 
			InputFrameBuffer
		]
		( RsContext* Context )
		{
			PSY_PROFILER_SECTION( RenderRoot, "ScnPostProcessComponentRenderNode::render" );

			auto InputTexture = Input_ != nullptr ? Input_->getTexture() : nullptr;
			auto OutputTexture = Output_ != nullptr ? Output_->getTexture() : nullptr;

			RsFrameBuffer* FrameBuffer = InputFrameBuffer;
			if( FrameBuffer == nullptr )
			{
				FrameBuffer = Context->getBackBuffer();
			}

			// Copy FB into usable texture.
			// TODO: Determine if this step is required base on nodes.
			if( InputTexture != nullptr )
			{
				Context->copyTexture( FrameBuffer->getDesc().RenderTargets_[ 0 ].Texture_, InputTexture );
			}

			ScnShaderPermutationFlags Permutation = 
				ScnShaderPermutationFlags::RENDER_POST_PROCESS |
				ScnShaderPermutationFlags::PASS_MAIN |
				ScnShaderPermutationFlags::MESH_STATIC_2D;

			// Iterate over nodes to process.
			for( size_t NodeIdx = 0; NodeIdx < Nodes_.size(); ++NodeIdx )
			{
 				auto& Node = Nodes_[ NodeIdx ];
 				auto& FrameBuffer = FrameBuffers_[ NodeIdx ];
 				auto& RenderState = RenderStates_[ NodeIdx ];
 				auto& ProgramBinding = ProgramBindings_[ NodeIdx ];

				// Grab first input texture for size data.
				MaVec2d UVSize( 1.0f, 1.0f );
				auto TextureIt = Node.InputTextures_.begin();
				if( TextureIt != Node.InputTextures_.end() )
				{
					auto Texture = TextureIt->second;
					auto Rect = Texture->getRect( 0 );			
					UVSize.x( Rect.W_ );
					UVSize.y( Rect.H_ );
				}

				// Update vertex buffer.
				const BcU32 VertexBufferSize = 4 * VertexDeclaration_->getDesc().getMinimumStride();
				const auto& Features = Context->getFeatures();
				const auto RTOrigin = Features.RTOrigin_;
				Context->updateBuffer( 
					VertexBuffer_.get(),
					0, VertexBufferSize,
					RsResourceUpdateFlags::ASYNC,
					[ RTOrigin, UVSize ]( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						auto Vertices = reinterpret_cast< ScnPostProcessVertex* >( Lock.Buffer_ );

						// TODO: Pass in separate UVs for what is intended to be a render target source?
						if( RTOrigin == RsFeatureRenderTargetOrigin::BOTTOM_LEFT )
						{
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 1.0f - UVSize.y() ) );
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), 1.0f - UVSize.y() ) );
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 1.0f ) );
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), 1.0f ) );
						}
						else
						{
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, UVSize.y() ) );
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), UVSize.y() ) );
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 0.0f ) );
							*Vertices++ = ScnPostProcessVertex( 
								MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), 0.0f ) );
						}
					} );
					
				// Setup config uniform block.
				ScnShaderPostProcessConfigData* ConfigUniformBlock = nullptr;
				auto FoundConfigBlockIt = std::find_if( Node.Uniforms_.begin(), Node.Uniforms_.end(),
					[ this ]( const ScnPostProcessUniforms& Uniforms )
					{
						return Uniforms.Name_ == "ScnShaderPostProcessConfigData";
					} );
				if( FoundConfigBlockIt != Node.Uniforms_.end() )
				{
					ConfigUniformBlock = FoundConfigBlockIt->Data_.getData< ScnShaderPostProcessConfigData >();
					const auto& FrameBufferDesc = FrameBuffer->getDesc(); 
					for( size_t Idx = 0; Idx < FrameBufferDesc.RenderTargets_.size(); ++Idx )
					{
						RsTexture* RenderTarget = FrameBufferDesc.RenderTargets_[ Idx ].Texture_;
						if( RenderTarget )
						{
							const auto& Desc = RenderTarget->getDesc();
							ConfigUniformBlock->OutputDimensions_[ Idx ] = 
								MaVec4d( Desc.Width_, Desc.Height_, Desc.Depth_, Desc.Levels_ );
						}
					}
					Context->updateBuffer( 
						FoundConfigBlockIt->Buffer_, 
						0, sizeof( ScnShaderPostProcessConfigData ),
						RsResourceUpdateFlags::ASYNC,
						[ ConfigUniformBlock ]( RsBuffer* Buffer, const RsBufferLock& Lock )
						{
							memcpy( Lock.Buffer_, ConfigUniformBlock, sizeof( ScnShaderPostProcessConfigData ) );
						} );
				}

				// Draw.
				Context->drawPrimitives( 
					GeometryBinding_.get(),
					ProgramBinding.get(),
					RenderState.get(),
					FrameBuffer.get(),
					nullptr,
					nullptr,
					RsTopologyType::TRIANGLE_STRIP, 0, 4, 0, 1  );
			}

			// Copy back to FB.
			// TODO: Determine if this step is required based on nodes.
			if( OutputTexture != nullptr )
			{
				Context->copyTexture( OutputTexture, FrameBuffer->getDesc().RenderTargets_[ 0 ].Texture_ );
			}

			RenderFence_.decrement();
		} );
}

//////////////////////////////////////////////////////////////////////////
// recreateResources
void ScnPostProcessComponent::recreateResources()
{
	RsContext* Context = RsCore::pImpl()->getContext( nullptr );

	if( VertexDeclaration_ == nullptr )
	{
		VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration(
			RsVertexDeclarationDesc( 2 )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnPostProcessVertex*)0)->Position_),  4, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnPostProcessVertex*)0)->UV_), 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) ),
			getFullName().c_str() );
	}

	if( VertexBuffer_ == nullptr )
	{
		BcU32 VertexBufferSize = 4 * VertexDeclaration_->getDesc().getMinimumStride();
		VertexBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsResourceBindFlags::VERTEX_BUFFER,
				RsResourceCreationFlags::STREAM, 
				VertexBufferSize ),
			getFullName().c_str() );
	}

	if( GeometryBinding_ == nullptr )
	{
		RsGeometryBindingDesc GeometryBindingDesc;
		GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
		GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), sizeof( ScnPostProcessVertex ) );
		GeometryBinding_ = RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, getFullName().c_str() );
	}

	const ScnShaderPermutationFlags Permutation = 
		ScnShaderPermutationFlags::RENDER_POST_PROCESS |
		ScnShaderPermutationFlags::PASS_MAIN |
		ScnShaderPermutationFlags::MESH_STATIC_2D;

	// Create framebuffers, render states, and uniform buffers for nodes.
	FrameBuffers_.clear();
	RenderStates_.clear();
	ProgramBindings_.clear();
	for( auto& Node : Nodes_ )
	{
		RsFrameBufferDesc Desc( 8 );
		RsProgramBindingDesc ProgramBindingDesc;

		auto Program = Node.Shader_->getProgram( Permutation );

		for( auto& SamplerSlot : Node.InputSamplers_ )
		{
			BcU32 Slot = Program->findSamplerSlot( SamplerSlot.first.c_str() );
			RsSamplerStateUPtr SamplerState( RsCore::pImpl()->createSamplerState( SamplerSlot.second, getFullName().c_str() ) );
			ProgramBindingDesc.setSamplerState( Slot, SamplerState.get() );
			SamplerStates_.emplace_back( std::move( SamplerState ) );
		}

		// Set input textures.
		ScnShaderPostProcessConfigData* ConfigUniformBlock = nullptr;
		auto FoundConfigBlockIt = std::find_if( Node.Uniforms_.begin(), Node.Uniforms_.end(),
			[ this ]( const ScnPostProcessUniforms& Uniforms )
			{
				return Uniforms.Name_ == "ScnShaderPostProcessConfigData";
			} );
		if( FoundConfigBlockIt != Node.Uniforms_.end() )
		{
			ConfigUniformBlock = FoundConfigBlockIt->Data_.getData< ScnShaderPostProcessConfigData >();
		}
		BcU32 TextureIdx = 0;
		for( auto& InputTexture : Node.InputTextures_ )
		{
			BcU32 Slot = Program->findShaderResourceSlot( InputTexture.first.c_str() );
			ProgramBindingDesc.setShaderResourceView( Slot, InputTexture.second->getTexture() );
			// If we have a config block, set the textures dimensions.
			// TODO: Deprecate, handle in render system.
			if( ConfigUniformBlock && Slot != BcErrorCode )
			{
				const auto& Desc = InputTexture.second->getTexture()->getDesc();
				ConfigUniformBlock->InputDimensions_[ Slot ] = 
					MaVec4d( Desc.Width_, Desc.Height_, Desc.Depth_, Desc.Levels_ );
			}
		}

		for( auto& TextureSlot : Node.OutputTextures_ )
		{
			if( TextureSlot.second )
			{
				Desc.setRenderTarget( TextureSlot.first, TextureSlot.second->getTexture() );
			}
		}

		for( auto& Uniforms : Node.Uniforms_ )
		{
			BcU32 Slot = Program->findUniformBufferSlot( Uniforms.Name_.c_str() );

			auto& BlockData = Uniforms.Data_;
			auto BufferDesc = RsBufferDesc( 
				RsResourceBindFlags::UNIFORM_BUFFER, RsResourceCreationFlags::DYNAMIC, BlockData.getDataSize() );
			RsBufferUPtr Buffer( RsCore::pImpl()->createBuffer( BufferDesc, getFullName().c_str() ) );
			ProgramBindingDesc.setUniformBuffer( Slot, Buffer.get(), 0, Buffer->getDesc().SizeBytes_ );
			Uniforms.Buffer_ = Buffer.get();
			UniformBuffers_.emplace_back( std::move( Buffer ) );

			if( Slot != BcErrorCode )
			{
				RsCore::pImpl()->updateBuffer( 
					Uniforms.Buffer_, 
					0, Uniforms.Data_.getDataSize(),
					RsResourceUpdateFlags::ASYNC,
					[ &Uniforms ]( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						memcpy( Lock.Buffer_, Uniforms.Data_.getData< BcU8* >(), Uniforms.Data_.getDataSize() );
					} );
			}
		}

		if( Desc.RenderTargets_[ 0 ].Texture_ == nullptr )
		{
			FrameBuffers_.emplace_back( nullptr );	
		}
		else
		{
			FrameBuffers_.emplace_back( RsCore::pImpl()->createFrameBuffer( Desc, getFullName().c_str() ) );	
		}
		RenderStates_.emplace_back( RsCore::pImpl()->createRenderState( Node.RenderState_, getFullName().c_str() ) );	

		ProgramBindings_.emplace_back( RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, getFullName().c_str() ) );
	}
}
