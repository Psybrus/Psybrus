#include "System/Renderer/RsCore.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsRenderNode.h"

#include "System/Scene/Rendering/ScnPostProcessComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/ScnEntity.h"

#include "System/Scene/Rendering/ScnRenderingVisitor.h"

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
	};
	ReRegisterClass< ScnPostProcessNode >( Fields );
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
	};
	
	ReRegisterClass< ScnPostProcessComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( 2000 ) );
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
void ScnPostProcessComponent::render(
	class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	class ScnPostProcessComponentRenderNode: public RsRenderNode
	{
	public:
		void render() override
		{
			PSY_PROFILER_SECTION( RenderRoot, "ScnPostProcessComponentRenderNode::render" );

			auto InputTexture = Component_->Input_->getTexture();
			auto OutputTexture = Component_->Output_->getTexture();
			if( OutputTexture != nullptr )
			{
				// Copy FB into usable texture.
				// TODO: Determine if this step is required base on nodes.
				pContext_->copyFrameBufferRenderTargetToTexture( InputFrameBuffer_, 0, InputTexture );

				ScnShaderPermutationFlags Permutation = 
					ScnShaderPermutationFlags::RENDER_POST_PROCESS |
					ScnShaderPermutationFlags::PASS_MAIN |
					ScnShaderPermutationFlags::MESH_STATIC_2D |
					ScnShaderPermutationFlags::LIGHTING_NONE;

				// Iterate over nodes to process.
				for( size_t NodeIdx = 0; NodeIdx < Component_->Nodes_.size(); ++NodeIdx )
				{
 					auto& Node = Component_->Nodes_[ NodeIdx ];
 					auto& FrameBuffer = Component_->FrameBuffers_[ NodeIdx ];
 					auto& RenderState = Component_->RenderStates_[ NodeIdx ];

 					// Bind program.
					auto Program = Node.Shader_->getProgram( Permutation );
					pContext_->setProgram( Program );
					// Bind samplers + textures.
					// TODO: Remove the findTextureSlot & findSamplerSlot calls. Do ahead of time.
					for( auto& InputTexture : Node.InputTextures_ )
					{
						BcU32 Slot = Program->findTextureSlot( InputTexture.first.c_str() );
						pContext_->setTexture( Slot, InputTexture.second->getTexture() );
					}

					for( auto& InputSampler : Component_->SamplerStates_ )
					{
						BcU32 Slot = Program->findSamplerSlot( InputSampler.first.c_str() );
						pContext_->setSamplerState( Slot, InputSampler.second.get() );
					}

					// Draw.
					pContext_->setFrameBuffer( FrameBuffer.get() );
					pContext_->setRenderState( RenderState.get() );
					pContext_->setVertexDeclaration( Component_->VertexDeclaration_.get() );
					pContext_->setVertexBuffer( 0, Component_->VertexBuffer_.get(), sizeof( ScnPostProcessVertex ) );
					pContext_->drawPrimitives( RsTopologyType::TRIANGLE_STRIP, 0, 4 );
				}

				// Copy back to FB.
				// TODO: Determine if this step is required based on nodes.
				pContext_->copyTextureToFrameBufferRenderTarget( OutputTexture, InputFrameBuffer_, 0 );

				Component_->RenderFence_.decrement();
			}
		}

		RsFrameBuffer* InputFrameBuffer_;
		ScnPostProcessComponent* Component_;
	};

	Sort.Pass_ = RS_SORT_PASS_POSTPROCESS;

	auto RenderNode = pFrame->newObject< ScnPostProcessComponentRenderNode >();
	RenderNode->Sort_ = Sort;
	RenderNode->InputFrameBuffer_ = pViewComponent->getFrameBuffer();
	RenderNode->Component_ = this;

	RenderFence_.increment();
	pFrame->addRenderNode( RenderNode );
}

//////////////////////////////////////////////////////////////////////////
// recreateResources
void ScnPostProcessComponent::recreateResources()
{
	RsContext* Context = RsCore::pImpl()->getContext( nullptr );
	auto Width = Context->getWidth();
	auto Height = Context->getHeight();

	if( VertexDeclaration_ == nullptr )
	{
		VertexDeclaration_.reset( RsCore::pImpl()->createVertexDeclaration(
			RsVertexDeclarationDesc( 2 )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnPostProcessVertex*)0)->Position_),  4, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnPostProcessVertex*)0)->UV_), 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) ) ) );
	}

	if( VertexBuffer_ == nullptr )
	{
		BcU32 VertexBufferSize = 4 * VertexDeclaration_->getDesc().getMinimumStride();
		VertexBuffer_.reset( RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::VERTEX,
				RsResourceCreationFlags::STREAM, 
				VertexBufferSize ) ) );
		RsCore::pImpl()->updateBuffer( 
			VertexBuffer_.get(),
			0, VertexBufferSize,
			RsResourceUpdateFlags::ASYNC,
			[]( RsBuffer* Buffer, const RsBufferLock& Lock )
			{
				auto Vertices = reinterpret_cast< ScnPostProcessVertex* >( Lock.Buffer_ );
				*Vertices++ = ScnPostProcessVertex( 
					MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 0.0f ) );
				*Vertices++ = ScnPostProcessVertex( 
					MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 1.0f, 0.0f ) );
				*Vertices++ = ScnPostProcessVertex( 
					MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 1.0f ) );
				*Vertices++ = ScnPostProcessVertex( 
					MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 1.0f, 1.0f ) );
			} );		
	}

	// Create framebuffers + render states for nodes.
	FrameBuffers_.clear();
	RenderStates_.clear();
	for( auto& Node : Nodes_ )
	{
		RsFrameBufferDesc Desc( 8 );

		for( auto& SamplerSlot : Node.InputSamplers_ )
		{
			auto SamplerState = RsCore::pImpl()->createSamplerState( SamplerSlot.second );
			SamplerStates_[ SamplerSlot.first ] = std::move( SamplerState );			
		}

		for( auto& TextureSlot : Node.OutputTextures_ )
		{
			if( TextureSlot.second )
			{
				Desc.setRenderTarget( TextureSlot.first, TextureSlot.second->getTexture() );
			}
		}

		FrameBuffers_.push_back( RsCore::pImpl()->createFrameBuffer( Desc ) );		
		RenderStates_.push_back( RsCore::pImpl()->createRenderState( Node.RenderState_ ) );
	}
}
