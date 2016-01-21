#include "System/Renderer/RsCore.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Scene/ScnCore.h"

#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsRenderNode.h"

#include "System/Scene/Rendering/ScnDeferredLightingComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Scene/Rendering/ScnRenderingVisitor.h"

#include "System/Debug/DsCore.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Define resource.
class ScnDeferredLightingViewRenderData : 
	public ScnViewRenderData
{
public:
	ScnDeferredLightingViewRenderData()
	{
	}

	virtual ~ScnDeferredLightingViewRenderData()
	{
	}

	std::array< RsProgramBindingUPtr, scnLT_MAX > ProgramBindings_;
};

//////////////////////////////////////////////////////////////////////////
// ScnDeferredLightingVertex
REFLECTION_DEFINE_BASIC( ScnDeferredLightingVertex );

void ScnDeferredLightingVertex::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Position_", &ScnDeferredLightingVertex::Position_ ),
		new ReField( "UV_", &ScnDeferredLightingVertex::UV_ ),
	};
	ReRegisterClass< ScnDeferredLightingVertex >( Fields );
}

ScnDeferredLightingVertex::ScnDeferredLightingVertex()
{

}

ScnDeferredLightingVertex::ScnDeferredLightingVertex( const MaVec4d& Position, const MaVec2d& UV ):
	Position_( Position ),
	UV_( UV )
{

}

//////////////////////////////////////////////////////////////////////////
// ScnDeferredLightingComponent
REFLECTION_DEFINE_DERIVED( ScnDeferredLightingComponent );

void ScnDeferredLightingComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Textures_", &ScnDeferredLightingComponent::Textures_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "Shaders_", &ScnDeferredLightingComponent::Shaders_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnDeferredLightingComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnDeferredLightingComponent::ScnDeferredLightingComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
ScnDeferredLightingComponent::~ScnDeferredLightingComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// createViewRenderData
//virtual
class ScnViewRenderData* ScnDeferredLightingComponent::createViewRenderData( class ScnViewComponent* View )
{
	ScnDeferredLightingViewRenderData* ViewRenderData = new ScnDeferredLightingViewRenderData();

	const ScnShaderPermutationFlags Permutation = 
		ScnShaderPermutationFlags::RENDER_POST_PROCESS |
		ScnShaderPermutationFlags::PASS_MAIN |
		ScnShaderPermutationFlags::MESH_STATIC_2D |
		ScnShaderPermutationFlags::LIGHTING_NONE;

	// Create program binding.
	RsProgramBindingDesc ProgramBindingDesc;
	for( size_t LightTypeIdx = 0; LightTypeIdx < Shaders_.size(); ++LightTypeIdx )
	{
		auto Shader = Shaders_[ LightTypeIdx ];
		auto Program = Shader->getProgram( Permutation );

		for( auto& Texture : Textures_ )
		{
			BcU32 SRVSlot = Program->findShaderResourceSlot( Texture.first.c_str() );
			BcU32 SamplerSlot = Program->findSamplerSlot( Texture.first.c_str() );
			ProgramBindingDesc.setShaderResourceView( SRVSlot, Texture.second->getTexture() );
			ProgramBindingDesc.setSamplerState( SamplerSlot, SamplerState_.get() );
		}

		// TODO: Tailored uniform buffer for deferred.
		{
			BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderLightUniformBlockData" );
			if( UniformSlot != BcErrorCode )
			{
				ProgramBindingDesc.setUniformBuffer( UniformSlot, UniformBuffer_.get() );
			}
		}
		{
			BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderViewUniformBlockData" );
			if( UniformSlot != BcErrorCode )
			{
				ProgramBindingDesc.setUniformBuffer( UniformSlot, View->getViewUniformBuffer() );
			}
		}

		ViewRenderData->ProgramBindings_[ LightTypeIdx ] = RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, getFullName().c_str() );
	}

	return ViewRenderData;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnDeferredLightingComponent::onAttach( ScnEntityWeakRef Parent )
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
void ScnDeferredLightingComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getAABB
MaAABB ScnDeferredLightingComponent::getAABB() const
{
	return MaAABB();
}

//////////////////////////////////////////////////////////////////////////
// visit
void ScnDeferredLightingComponent::visit( class ScnLightComponent* Component )
{
	LightComponents_.emplace_back( Component );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnDeferredLightingComponent::render( ScnRenderContext & RenderContext )
{
	RsRenderSort Sort = RenderContext.Sort_;
	Sort.Layer_ = 0;

	auto* ViewRenderData = static_cast< ScnDeferredLightingViewRenderData* >( RenderContext.ViewRenderData_ );

	// Gather lights that intersect with our view.
	ScnCore::pImpl()->visitView( this, RenderContext.pViewComponent_ );

	// Render all lights.
	for( size_t Idx = 0; Idx < LightComponents_.size(); ++Idx )
	{
		auto LightComponent = LightComponents_[ Idx ];
		auto ProgramBinding = ViewRenderData->ProgramBindings_[ scnLT_SPOT ].get(); // TODO: Get from component.
		
		// 
		ScnShaderLightUniformBlockData LightUniformData = {};
		LightComponent->setLightUniformBlockData( 0, LightUniformData );
		LightComponent->setLightUniformBlockData( 1, LightUniformData );
		LightComponent->setLightUniformBlockData( 2, LightUniformData );
		LightComponent->setLightUniformBlockData( 3, LightUniformData );
		
		RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
			[ 
				UniformBuffer = UniformBuffer_.get(),
				LightUniformData = LightUniformData,
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = ProgramBinding,
				RenderState = RenderState_.get(),
				FrameBuffer = RenderContext.pViewComponent_->getFrameBuffer(),
				Viewport = RenderContext.pViewComponent_->getViewport()
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
#if 1
				Context->updateBuffer( UniformBuffer, 0, sizeof( LightUniformData ), RsResourceUpdateFlags::ASYNC,
					[ &LightUniformData ]( RsBuffer* Buffer, RsBufferLock Lock )
					{
						BcAssert( Buffer->getDesc().SizeBytes_ == sizeof( LightUniformData ) );
						memcpy( Lock.Buffer_, &LightUniformData, sizeof( LightUniformData ) );
					} );
#endif

				Context->drawPrimitives( 
					GeometryBinding,
					ProgramBinding,
					RenderState,
					FrameBuffer,
					&Viewport,
					nullptr,
					RsTopologyType::TRIANGLE_STRIP, 0, 4 );
			} );
	}

	LightComponents_.clear();
}

//////////////////////////////////////////////////////////////////////////
// recreateResources
void ScnDeferredLightingComponent::recreateResources()
{
	RsContext* Context = RsCore::pImpl()->getContext( nullptr );

	if( VertexDeclaration_ == nullptr )
	{
		VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration(
			RsVertexDeclarationDesc( 2 )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnDeferredLightingVertex*)0)->Position_),  4, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnDeferredLightingVertex*)0)->UV_), 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) ),
			getFullName().c_str() );
	}

	if( VertexBuffer_ == nullptr )
	{
		BcU32 VertexBufferSize = 4 * VertexDeclaration_->getDesc().getMinimumStride();
		VertexBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::VERTEX,
				RsResourceCreationFlags::STREAM, 
				VertexBufferSize ),
			getFullName().c_str() );

		const auto& Features = RsCore::pImpl()->getContext( 0 )->getFeatures();
		const auto RTOrigin = Features.RTOrigin_;
		RsCore::pImpl()->updateBuffer( 
			VertexBuffer_.get(),
			0, VertexBufferSize,
			RsResourceUpdateFlags::ASYNC,
			[ RTOrigin ]( RsBuffer* Buffer, const RsBufferLock& Lock )
			{
				auto Vertices = reinterpret_cast< ScnDeferredLightingVertex* >( Lock.Buffer_ );

				// TODO: Pass in separate UVs for what is intended to be a render target source?
				if( RTOrigin == RsFeatureRenderTargetOrigin::BOTTOM_LEFT )
				{
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 0.0f ) );
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 1.0f, 0.0f ) );
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 1.0f ) );
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 1.0f, 1.0f ) );
				}
				else
				{
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 1.0f ) );
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 1.0f, 1.0f ) );
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 0.0f ) );
					*Vertices++ = ScnDeferredLightingVertex( 
						MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 1.0f, 0.0f ) );
				}
			} );		
	}

	if( UniformBuffer_ == nullptr )
	{
		auto BufferDesc = RsBufferDesc( 
			RsBufferType::UNIFORM, RsResourceCreationFlags::STREAM, sizeof( ScnShaderLightUniformBlockData ) );
		UniformBuffer_ = RsCore::pImpl()->createBuffer( BufferDesc, getFullName().c_str() );
	}

	if( GeometryBinding_ == nullptr )
	{
		RsGeometryBindingDesc GeometryBindingDesc;
		GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
		GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), sizeof( ScnDeferredLightingVertex ) );
		GeometryBinding_ = RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, getFullName().c_str() );
	}

	// Create sampler.
	RsSamplerStateDesc SamplerStateDesc;
	SamplerStateDesc.MinFilter_ = RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR;
	SamplerStateDesc.MagFilter_ = RsTextureFilteringMode::LINEAR;
	SamplerState_ = RsCore::pImpl()->createSamplerState( SamplerStateDesc, getFullName().c_str() );

	// Create render state.
	RsRenderStateDesc RenderStateDesc;
	auto& BlendState = RenderStateDesc.BlendState_.RenderTarget_[ 0 ];
	BlendState.Enable_ = BcTrue;
	BlendState.SrcBlend_ = RsBlendType::ONE;
	BlendState.SrcBlendAlpha_ = RsBlendType::ONE;
	BlendState.DestBlend_ = RsBlendType::ONE;
	BlendState.DestBlendAlpha_ = RsBlendType::ONE;
	BlendState.BlendOp_ = RsBlendOp::ADD;
	BlendState.BlendOpAlpha_ = RsBlendOp::ADD;
	RenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, getFullName().c_str() );

	resetViewRenderData( nullptr );
}
