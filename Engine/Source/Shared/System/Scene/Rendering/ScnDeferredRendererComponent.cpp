#include "System/Renderer/RsCore.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Scene/ScnCore.h"

#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsRenderNode.h"

#include "System/Scene/Rendering/ScnDeferredRendererComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

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
// ScnDeferredRendererVertex
REFLECTION_DEFINE_BASIC( ScnDeferredRendererVertex );

void ScnDeferredRendererVertex::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Position_", &ScnDeferredRendererVertex::Position_ ),
		new ReField( "UV_", &ScnDeferredRendererVertex::UV_ ),
		new ReField( "Screen_", &ScnDeferredRendererVertex::Screen_ ),
	};
	ReRegisterClass< ScnDeferredRendererVertex >( Fields );
}

ScnDeferredRendererVertex::ScnDeferredRendererVertex()
{

}

ScnDeferredRendererVertex::ScnDeferredRendererVertex( const MaVec4d& Position, const MaVec2d& UV, const MaVec2d& Screen ):
	Position_( Position ),
	UV_( UV ),
	Screen_( Screen )
{

}

//////////////////////////////////////////////////////////////////////////
// ScnDeferredRendererComponent
REFLECTION_DEFINE_DERIVED( ScnDeferredRendererComponent );

void ScnDeferredRendererComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Textures_", &ScnDeferredRendererComponent::Textures_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "Shaders_", &ScnDeferredRendererComponent::Shaders_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnDeferredRendererComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnDeferredRendererComponent::ScnDeferredRendererComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
ScnDeferredRendererComponent::~ScnDeferredRendererComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnDeferredRendererComponent::onAttach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
		[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
		{
			recreateResources();
			return evtRET_PASS;
		} );

	recreateResources();

	// TODO
	//View_ = Parent->attach< ScnViewComponent >();

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnDeferredRendererComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// visit
void ScnDeferredRendererComponent::visit( class ScnLightComponent* Component )
{
	LightComponents_.emplace_back( Component );
}

#if 0 
//////////////////////////////////////////////////////////////////////////
// render
void ScnDeferredRendererComponent::render( ScnRenderContext & RenderContext )
{
	RsRenderSort Sort = RenderContext.Sort_;
	Sort.Layer_ = 0;

	auto* ViewRenderData = static_cast< ScnDeferredLightingViewRenderData* >( RenderContext.ViewRenderData_ );

	// Grab albedo texture for size data.
	MaVec2d UVSize( 1.0f, 1.0f );
	auto AlbedoTexIt = Textures_.find( "aAlbedoTex" );
	if( AlbedoTexIt != Textures_.end() )
	{
		auto AlbedoTex = AlbedoTexIt->second;
		auto Rect = AlbedoTex->getRect( 0 );			
		UVSize.x( Rect.W_ );
		UVSize.y( Rect.H_ );
	}

	// Update vertex buffer.
	const BcU32 VertexBufferSize = 4 * VertexDeclaration_->getDesc().getMinimumStride();
	const auto& Features = RsCore::pImpl()->getContext( 0 )->getFeatures();
	const auto RTOrigin = Features.RTOrigin_;
	RsCore::pImpl()->updateBuffer( 
		VertexBuffer_.get(),
		0, VertexBufferSize,
		RsResourceUpdateFlags::ASYNC,
		[ RTOrigin, UVSize ]( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			auto Vertices = reinterpret_cast< ScnDeferredRendererVertex* >( Lock.Buffer_ );

			// TODO: Pass in separate UVs for what is intended to be a render target source?
			if( RTOrigin == RsFeatureRenderTargetOrigin::BOTTOM_LEFT )
			{
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 1.0f - UVSize.y() ), MaVec2d( -1.0f, -1.0f ) );
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), 1.0f - UVSize.y() ), MaVec2d( 1.0f, -1.0f ) );
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 1.0f ), MaVec2d( -1.0f, 1.0f ) );
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), 1.0f ), MaVec2d( 1.0f, 1.0f ) );
			}
			else
			{
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d( -1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, UVSize.y() ), MaVec2d( -1.0f, 1.0f ) );
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d(  1.0f, -1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), UVSize.y() ), MaVec2d( 1.0f, 1.0f ) );
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d( -1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( 0.0f, 0.0f ), MaVec2d( -1.0f, -1.0f ) );
				*Vertices++ = ScnDeferredRendererVertex( 
					MaVec4d(  1.0f,  1.0f,  0.0f,  1.0f ), MaVec2d( UVSize.x(), 0.0f ), MaVec2d( 1.0f, -1.0f ) );
			}
		} );	
	
	// TODO: Use ScnLightProcessor when implemented.
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

				Context->updateBuffer( UniformBuffer, 0, sizeof( LightUniformData ), RsResourceUpdateFlags::ASYNC,
					[ &LightUniformData ]( RsBuffer* Buffer, RsBufferLock Lock )
					{
						BcAssert( Buffer->getDesc().SizeBytes_ == sizeof( LightUniformData ) );
						memcpy( Lock.Buffer_, &LightUniformData, sizeof( LightUniformData ) );
					} );

				Context->drawPrimitives( 
					GeometryBinding,
					ProgramBinding,
					RenderState,
					FrameBuffer,
					&Viewport,
					nullptr,
					RsTopologyType::TRIANGLE_STRIP, 0, 4, 0, 1  );
			} );
	}

	LightComponents_.clear();
}
#endif

//////////////////////////////////////////////////////////////////////////
// recreateResources
void ScnDeferredRendererComponent::recreateResources()
{
	// Free all textures.
	for( auto& Texture : Textures_ )
	{
		if( Texture )
		{
			delete Texture.get();
			Texture = nullptr;
		}
	}

	// Free all frame buffers.
	GBuffer_.reset();
	LightingBuffer_.reset();

	// Create textures.
	Textures_[ TEX_GBUFFER_ALBEDO ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, "TEX_GBUFFER_ALBEDO" );
	Textures_[ TEX_GBUFFER_MATERIAL ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, "TEX_GBUFFER_MATERIAL" );
	Textures_[ TEX_GBUFFER_NORMAL ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, "TEX_GBUFFER_NORMAL" );
	Textures_[ TEX_GBUFFER_VELOCITY ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, "TEX_GBUFFER_VELOCITY" );
	Textures_[ TEX_GBUFFER_DEPTH ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::D24S8, "TEX_GBUFFER_DEPTH" );
	Textures_[ TEX_LIGHTING ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R16FG16FB16FA16F, "TEX_LIGHTING" );

	// Create GBuffer.
	{
		RsFrameBufferDesc FBDesc = RsFrameBufferDesc( 4 )
			.setRenderTarget( 0, Textures_[ TEX_GBUFFER_ALBEDO ]->getTexture() )
			.setRenderTarget( 1, Textures_[ TEX_GBUFFER_MATERIAL ]->getTexture() )
			.setRenderTarget( 2, Textures_[ TEX_GBUFFER_NORMAL ]->getTexture() )
			.setRenderTarget( 3, Textures_[ TEX_GBUFFER_VELOCITY ]->getTexture() )
			.setDepthStencilTarget( Textures_[ TEX_GBUFFER_DEPTH ]->getTexture() );
		GBuffer_ = RsCore::pImpl()->createFrameBuffer( FBDesc, "GBuffer" );
	}

	// Create lighting buffer.
	{
		RsFrameBufferDesc FBDesc = RsFrameBufferDesc( 1 )
			.setRenderTarget( 0, Textures_[ TEX_LIGHTING ]->getTexture() )
			.setDepthStencilTarget( Textures_[ TEX_GBUFFER_DEPTH ]->getTexture() );
		LightingBuffer_ = RsCore::pImpl()->createFrameBuffer( FBDesc, "Lighting" );
	}


	if( VertexDeclaration_ == nullptr )
	{
		VertexDeclaration_ = RsCore::pImpl()->createVertexDeclaration(
			RsVertexDeclarationDesc( 2 )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnDeferredRendererVertex*)0)->Position_),  4, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnDeferredRendererVertex*)0)->UV_), 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) )
				.addElement( RsVertexElement( 0, (size_t)(&((ScnDeferredRendererVertex*)0)->Screen_), 2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 1 ) ),
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

	if( UniformBuffer_ == nullptr )
	{
		auto BufferDesc = RsBufferDesc( 
			RsResourceBindFlags::UNIFORM_BUFFER, RsResourceCreationFlags::STREAM, sizeof( ScnShaderLightUniformBlockData ) );
		UniformBuffer_ = RsCore::pImpl()->createBuffer( BufferDesc, getFullName().c_str() );
	}

	if( GeometryBinding_ == nullptr )
	{
		RsGeometryBindingDesc GeometryBindingDesc;
		GeometryBindingDesc.setVertexDeclaration( VertexDeclaration_.get() );
		GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), sizeof( ScnDeferredRendererVertex ) );
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
	auto& DepthStencilState = RenderStateDesc.DepthStencilState_;
	DepthStencilState.DepthTestEnable_ = false;
	DepthStencilState.DepthWriteEnable_ = false;
	RenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, getFullName().c_str() );

	// Reset view render data.
	ScnViewProcessor::pImpl()->resetViewRenderData( this );
}
