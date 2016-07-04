#include "System/Renderer/RsCore.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Scene/ScnCore.h"

#include "System/Renderer/RsFeatures.h"
#include "System/Renderer/RsFrame.h"
#include "System/Renderer/RsRenderNode.h"

#include "System/Scene/Rendering/ScnDeferredRendererComponent.h"
#include "System/Scene/Rendering/ScnEnvironmentProbeComponent.h"
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
		new ReField( "Enabled_", &ScnDeferredRendererComponent::Enabled_, bcRFF_IMPORTER ),
		new ReField( "Width_", &ScnDeferredRendererComponent::Width_, bcRFF_IMPORTER ),
		new ReField( "Height_", &ScnDeferredRendererComponent::Height_, bcRFF_IMPORTER ),
		new ReField( "LightShaders_", &ScnDeferredRendererComponent::LightShaders_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "ReflectionShader_", &ScnDeferredRendererComponent::ReflectionShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "ResolveShader_", &ScnDeferredRendererComponent::ResolveShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "ResolveX_", &ScnDeferredRendererComponent::ResolveX_, bcRFF_IMPORTER ),
		new ReField( "ResolveY_", &ScnDeferredRendererComponent::ResolveY_, bcRFF_IMPORTER ),
		new ReField( "ResolveW_", &ScnDeferredRendererComponent::ResolveW_, bcRFF_IMPORTER ),
		new ReField( "ResolveH_", &ScnDeferredRendererComponent::ResolveH_, bcRFF_IMPORTER ),
		new ReField( "Near_", &ScnDeferredRendererComponent::Near_, bcRFF_IMPORTER ),
		new ReField( "Far_", &ScnDeferredRendererComponent::Far_, bcRFF_IMPORTER ),
		new ReField( "HorizontalFOV_", &ScnDeferredRendererComponent::HorizontalFOV_, bcRFF_IMPORTER ),
		new ReField( "VerticalFOV_", &ScnDeferredRendererComponent::VerticalFOV_, bcRFF_IMPORTER ),
		new ReField( "ReflectionCubemap_", &ScnDeferredRendererComponent::ReflectionCubemap_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "UseEnvironmentProbes_", &ScnDeferredRendererComponent::UseEnvironmentProbes_, bcRFF_IMPORTER ),
				
		new ReField( "Textures_", &ScnDeferredRendererComponent::Textures_, bcRFF_TRANSIENT ),
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


	// Create textures.
	Textures_[ TEX_GBUFFER_ALBEDO ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "Albedo" );
	Textures_[ TEX_GBUFFER_MATERIAL ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "Material" );
	Textures_[ TEX_GBUFFER_NORMAL ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "Normal" );
	Textures_[ TEX_GBUFFER_VELOCITY ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R8G8B8A8, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "Velocity" );
	Textures_[ TEX_GBUFFER_DEPTH ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::D24S8, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::DEPTH_STENCIL, "Depth" );
	Textures_[ TEX_HDR ] = ScnTexture::New2D( Width_, Height_, 1, RsTextureFormat::R16FG16FB16FA16F, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "HDR" );

	// Create views.
	OpaqueView_ = getParentEntity()->attach< ScnViewComponent >(
		"OpaqueView", 4, &Textures_[ TEX_GBUFFER_ALBEDO ], Textures_[ TEX_GBUFFER_DEPTH ],
		0x1, ScnShaderPermutationFlags::RENDER_DEFERRED, RsRenderSortPassFlags::OPAQUE, !!Enabled_ );
	TransparentView_ = getParentEntity()->attach< ScnViewComponent >(
		"TransparentView", 1, &Textures_[ TEX_HDR ], Textures_[ TEX_GBUFFER_DEPTH ],
		0x1, ScnShaderPermutationFlags::RENDER_DEFERRED, RsRenderSortPassFlags::TRANSPARENT, !!Enabled_ );
	OverlayView_ = getParentEntity()->attach< ScnViewComponent >(
		"OverlayView", 0, nullptr, nullptr,
		0x1, ScnShaderPermutationFlags::RENDER_DEFERRED, RsRenderSortPassFlags::OVERLAY, !!Enabled_ );

	OpaqueView_->registerViewCallback( this );
	TransparentView_->registerViewCallback( this );
	OverlayView_->registerViewCallback( this );

	OpaqueView_->setClearParams( RsColour::BLACK, true, true, true );
	TransparentView_->setClearParams( RsColour::BLACK, true, false, false );
	OverlayView_->setClearParams( RsColour::BLACK, false, false, false );

	setProjectionParams( Near_, Far_, HorizontalFOV_, VerticalFOV_ );

	// Recreate all resources.
	recreateResources();
	
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnDeferredRendererComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );

	// Already detached.
	OpaqueView_ = nullptr;
	TransparentView_ = nullptr;
	OverlayView_ = nullptr;

	// Free all textures.
	for( auto& Texture : Textures_ )
	{
		if( Texture )
		{
			Texture->markDestroy();
			Texture = nullptr;
		}
	}
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// visit
void ScnDeferredRendererComponent::visit( class ScnLightComponent* Component )
{
	LightComponents_.emplace_back( Component );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnDeferredRendererComponent::render( RsFrame* Frame, RsFrameBuffer* ResolveTarget, RsRenderSort Sort )
{
	RsFrameBuffer* OldResolveTarget = ResolveTarget_;
	ResolveTarget_ = ResolveTarget;
	ScnViewProcessor::pImpl()->renderView( OpaqueView_, Frame, Sort );
	Sort.Viewport_++;
	ScnViewProcessor::pImpl()->renderView( TransparentView_, Frame, Sort );
	Sort.Viewport_++;
	ScnViewProcessor::pImpl()->renderView( OverlayView_, Frame, Sort );
	ResolveTarget_ = OldResolveTarget;
}

//////////////////////////////////////////////////////////////////////////
// setProjectionParams
void ScnDeferredRendererComponent::setProjectionParams( BcF32 Near, BcF32 Far, BcF32 HorizonalFOV, BcF32 VerticalFOV )
{
	OpaqueView_->setProjectionParams( Near_, Far_, HorizontalFOV_, VerticalFOV_ );
	TransparentView_->setProjectionParams( Near_, Far_, HorizontalFOV_, VerticalFOV_ );
	OverlayView_->setProjectionParams( Near_, Far_, HorizontalFOV_, VerticalFOV_ );
}

//////////////////////////////////////////////////////////////////////////
// recreateResources
void ScnDeferredRendererComponent::recreateResources()
{
	for( auto & LightProgramBinding : LightProgramBindings_ )
	{
		LightProgramBinding.reset();
	}
	ReflectionProgramBinding_.reset();
	ResolveProgramBinding_.reset();

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

	// Create samplers.
	RsSamplerStateDesc SamplerStateDesc;
	SamplerStateDesc.MinFilter_ = RsTextureFilteringMode::NEAREST;
	SamplerStateDesc.MagFilter_ = RsTextureFilteringMode::NEAREST;
	NearestSamplerState_ = RsCore::pImpl()->createSamplerState( SamplerStateDesc, getFullName().c_str() );

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
	AdditiveRenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, getFullName().c_str() );
	BlendState.Enable_ = BcFalse;
	ResolveRenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, getFullName().c_str() );

	// Reset view render data.
	ScnViewProcessor::pImpl()->resetViewRenderData( this );
}

//////////////////////////////////////////////////////////////////////////
// setupQuad
void ScnDeferredRendererComponent::setupQuad( RsContext* Context, 
	RsVertexDeclaration* VertexDeclaration,
	RsBuffer* VertexBuffer,
	const MaVec2d& MinPos, 
	const MaVec2d& MaxPos, 
	const MaVec2d& UVSize )
{
	// Update vertex buffer.
	const BcU32 VertexBufferSize = 4 * VertexDeclaration->getDesc().getMinimumStride();
	const auto& Features = Context->getFeatures();
	const auto RTOrigin = Features.RTOrigin_;
	Context->updateBuffer( 
		VertexBuffer,
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
}

//////////////////////////////////////////////////////////////////////////
// renderLights
void ScnDeferredRendererComponent::renderLights( ScnRenderContext& RenderContext )
{
	RsRenderSort Sort = RenderContext.Sort_;
	Sort.Layer_ = 0;

	// HACK: Create lights
	if( LightProgramBindings_[ 0 ] == nullptr )
	{
		// Create program binding for lighting view.
		const ScnShaderPermutationFlags Permutation = 
			ScnShaderPermutationFlags::RENDER_POST_PROCESS |
			ScnShaderPermutationFlags::PASS_MAIN |
			ScnShaderPermutationFlags::MESH_STATIC_2D;

		RsProgramBindingDesc ProgramBindingDesc;
		for( size_t LightTypeIdx = 0; LightTypeIdx < LightShaders_.size(); ++LightTypeIdx )
		{
			auto Shader = LightShaders_[ LightTypeIdx ];
			auto Program = Shader->getProgram( Permutation );

			std::array< const char*, TEX_MAX > TextureNames_ = 
			{
				"aAlbedoTex",
				"aMaterialTex",
				"aNormalTex",
				"aVelocityTex",
				"aDepthTex",
				"aHDRTex"
			};

			for( size_t Idx = 0; Idx < TEX_MAX; ++Idx )
			{
				BcU32 SRVSlot = Program->findShaderResourceSlot( TextureNames_[ Idx ] );
				BcU32 SamplerSlot = Program->findSamplerSlot( TextureNames_[ Idx ] );
				ProgramBindingDesc.setShaderResourceView( SRVSlot, Textures_[ Idx ]->getTexture() );
				ProgramBindingDesc.setSamplerState( SamplerSlot, NearestSamplerState_.get() );
			}

			// TODO: Tailored uniform buffer for deferred.
			{
				BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderLightUniformBlockData" );
				if( UniformSlot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( UniformSlot, UniformBuffer_.get(), 0, sizeof( ScnShaderLightUniformBlockData ) );
				}
			}
			{
				BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderViewUniformBlockData" );
				if( UniformSlot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( UniformSlot, OpaqueView_->getViewUniformBuffer(), 0, sizeof( ScnShaderViewUniformBlockData ) );
				}
			}

			LightProgramBindings_[ LightTypeIdx ] = RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, getFullName().c_str() );
		}
	}

	// Grab albedo texture for size data.
	MaVec2d UVSize( 1.0f, 1.0f );
	auto AlbedoTex = Textures_[ TEX_GBUFFER_ALBEDO ];
	auto Rect = AlbedoTex->getRect( 0 );			
	UVSize.x( Rect.W_ );
	UVSize.y( Rect.H_ );

	// TODO: Use ScnLightProcessor when implemented.
	// Gather lights that intersect with our view.
	ScnCore::pImpl()->visitView( this, RenderContext.pViewComponent_ );


	// Render all lights.
	for( size_t Idx = 0; Idx < LightComponents_.size(); ++Idx )
	{
		auto LightComponent = LightComponents_[ Idx ];
		auto ProgramBinding = LightProgramBindings_[ scnLT_SPOT ].get(); // TODO: Get from component.
		
		// 
		ScnShaderLightUniformBlockData LightUniformData = {};
		LightComponent->setLightUniformBlockData( 0, LightUniformData );
		LightComponent->setLightUniformBlockData( 1, LightUniformData );
		LightComponent->setLightUniformBlockData( 2, LightUniformData );
		LightComponent->setLightUniformBlockData( 3, LightUniformData );
		
		RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
			[ 
				UVSize = UVSize,
				UniformBuffer = UniformBuffer_.get(),
				LightUniformData = LightUniformData,
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = ProgramBinding,
				RenderState = AdditiveRenderState_.get(),
				FrameBuffer = RenderContext.pViewComponent_->getFrameBuffer(),
				Viewport = RenderContext.pViewComponent_->getViewport()
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;

				setupQuad( Context, GeometryBinding->getDesc().VertexDeclaration_,
					GeometryBinding->getDesc().VertexBuffers_[ 0 ].Buffer_,
					MaVec2d( -1.0f, -1.0f ), MaVec2d( 1.0f, 1.0f ), UVSize );
				
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


//////////////////////////////////////////////////////////////////////////
// renderReflection
void ScnDeferredRendererComponent::renderReflection( ScnRenderContext& RenderContext )
{
	// Update reflection.
	if( UseEnvironmentProbes_ )
	{
		auto NearestEnvironmentMap = ScnEnvironmentProbeProcessor::pImpl()->getProbeEnvironmentMap( getParentEntity()->getWorldPosition() );
		if( NearestEnvironmentMap != ReflectionCubemap_ )
		{
			ReflectionCubemap_ = NearestEnvironmentMap;
			ReflectionProgramBinding_.reset();
		}
	}

	// HACK: Create resolve.
	if( ReflectionProgramBinding_ == nullptr )
	{
		// Create program binding for lighting view.
		const ScnShaderPermutationFlags Permutation = 
			ScnShaderPermutationFlags::RENDER_POST_PROCESS |
			ScnShaderPermutationFlags::PASS_MAIN |
			ScnShaderPermutationFlags::MESH_STATIC_2D;

		RsProgramBindingDesc ProgramBindingDesc;
		auto Program = ReflectionShader_->getProgram( Permutation );

		std::array< const char*, TEX_MAX > TextureNames_ = 
		{
			"aAlbedoTex",
			"aMaterialTex",
			"aNormalTex",
			"aVelocityTex",
			"aDepthTex",
			"aHDRTex"
		};

		for( size_t Idx = 0; Idx < TEX_MAX; ++Idx )
		{
			BcU32 SRVSlot = Program->findShaderResourceSlot( TextureNames_[ Idx ] );
			BcU32 SamplerSlot = Program->findSamplerSlot( TextureNames_[ Idx ] );
			ProgramBindingDesc.setShaderResourceView( SRVSlot, Textures_[ Idx ]->getTexture() );
			ProgramBindingDesc.setSamplerState( SamplerSlot, NearestSamplerState_.get() );
		}

		// Reflection cubemap.
		{
			BcU32 SRVSlot = Program->findShaderResourceSlot( "aReflectionTex" );
			BcU32 SamplerSlot = Program->findSamplerSlot( "aReflectionTex" );
			ProgramBindingDesc.setShaderResourceView( SRVSlot, ReflectionCubemap_->getTexture() );
			ProgramBindingDesc.setSamplerState( SamplerSlot, SamplerState_.get() );
		}

		{
			BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderViewUniformBlockData" );
			if( UniformSlot != BcErrorCode )
			{
				ProgramBindingDesc.setUniformBuffer( UniformSlot, OpaqueView_->getViewUniformBuffer(), 0, sizeof( ScnShaderViewUniformBlockData ) );
			}
		}

		ReflectionProgramBinding_ = RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, getFullName().c_str() );
	}

	// Grab albedo texture for size data.
	MaVec2d UVSize( 1.0f, 1.0f );
	auto AlbedoTex = Textures_[ TEX_GBUFFER_ALBEDO ];
	auto Rect = AlbedoTex->getRect( 0 );			
	UVSize.x( Rect.W_ );
	UVSize.y( Rect.H_ );

	RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
		[ 
			UVSize = UVSize,
			UniformBuffer = UniformBuffer_.get(),
			GeometryBinding = GeometryBinding_.get(),
			ProgramBinding = ReflectionProgramBinding_.get(),
			RenderState = AdditiveRenderState_.get(),
			FrameBuffer = RenderContext.pViewComponent_->getFrameBuffer()
		]
		( RsContext* Context )
		{
			PSY_PROFILE_FUNCTION;

			setupQuad( Context, GeometryBinding->getDesc().VertexDeclaration_,
				GeometryBinding->getDesc().VertexBuffers_[ 0 ].Buffer_,
				MaVec2d( -1.0f, -1.0f ), MaVec2d( 1.0f, 1.0f ), UVSize );
			
			Context->drawPrimitives( 
				GeometryBinding,
				ProgramBinding,
				RenderState,
				FrameBuffer,
				nullptr,
				nullptr,
				RsTopologyType::TRIANGLE_STRIP, 0, 4, 0, 1  );
		} );
}

//////////////////////////////////////////////////////////////////////////
// renderResolve
void ScnDeferredRendererComponent::renderResolve( ScnRenderContext& RenderContext )
{
	// HACK: Create resolve.
	if( ResolveProgramBinding_ == nullptr )
	{
		// Create program binding for lighting view.
		const ScnShaderPermutationFlags Permutation = 
			ScnShaderPermutationFlags::RENDER_POST_PROCESS |
			ScnShaderPermutationFlags::PASS_MAIN |
			ScnShaderPermutationFlags::MESH_STATIC_2D;
		
		RsProgramBindingDesc ProgramBindingDesc;
		auto Program = ResolveShader_->getProgram( Permutation );

		std::array< const char*, TEX_MAX > TextureNames_ = 
		{
			"aAlbedoTex",
			"aMaterialTex",
			"aNormalTex",
			"aVelocityTex",
			"aDepthTex",
			"aHDRTex"
		};

		for( size_t Idx = 0; Idx < TEX_MAX; ++Idx )
		{
			BcU32 SRVSlot = Program->findShaderResourceSlot( TextureNames_[ Idx ] );
			BcU32 SamplerSlot = Program->findSamplerSlot( TextureNames_[ Idx ] );
			ProgramBindingDesc.setShaderResourceView( SRVSlot, Textures_[ Idx ]->getTexture() );
			ProgramBindingDesc.setSamplerState( SamplerSlot, NearestSamplerState_.get() );
		}

		{
			BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderViewUniformBlockData" );
			if( UniformSlot != BcErrorCode )
			{
				ProgramBindingDesc.setUniformBuffer( UniformSlot, OpaqueView_->getViewUniformBuffer(), 0, sizeof( ScnShaderViewUniformBlockData ) );
			}
		}

		ResolveProgramBinding_ = RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, getFullName().c_str() );
	}

	// Grab albedo texture for size data.
	MaVec2d UVSize( 1.0f, 1.0f );
	auto AlbedoTex = Textures_[ TEX_GBUFFER_ALBEDO ];
	auto Rect = AlbedoTex->getRect( 0 );			
	UVSize.x( Rect.W_ );
	UVSize.y( Rect.H_ );

	RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
		[ 
			UVSize = UVSize,
			UniformBuffer = UniformBuffer_.get(),
			GeometryBinding = GeometryBinding_.get(),
			ProgramBinding = ResolveProgramBinding_.get(),
			RenderState = ResolveRenderState_.get(),
			ResolveTarget = ResolveTarget_,
			ResolveX = ResolveX_,
			ResolveY = ResolveY_,
			ResolveW = ResolveW_,
			ResolveH = ResolveH_
		]
		( RsContext* Context )
		{
			PSY_PROFILE_FUNCTION;

			setupQuad( Context, GeometryBinding->getDesc().VertexDeclaration_,
				GeometryBinding->getDesc().VertexBuffers_[ 0 ].Buffer_,
				MaVec2d( -1.0f, -1.0f ), MaVec2d( 1.0f, 1.0f ), UVSize );

			auto FrameBuffer = ResolveTarget ? ResolveTarget : Context->getBackBuffer();
			auto Desc = FrameBuffer->getDesc().RenderTargets_[ 0 ].Texture_->getDesc();

			RsViewport Viewport( 
				BcU32( ResolveX * Desc.Width_ ),
				BcU32( ResolveY * Desc.Height_ ),
				BcU32( ResolveW * Desc.Width_ ),
				BcU32( ResolveH * Desc.Height_ ) );
				
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

//////////////////////////////////////////////////////////////////////////
// onViewDrawPreRender
void ScnDeferredRendererComponent::onViewDrawPreRender( ScnRenderContext& RenderContext )
{
	if( RenderContext.pViewComponent_ == OpaqueView_ )
	{
		// begin opaque
	}
	else if( RenderContext.pViewComponent_ == TransparentView_ )
	{
		// begin transparent
		renderLights( RenderContext );
		renderReflection( RenderContext );
	}
	else if( RenderContext.pViewComponent_ == OverlayView_ )
	{
		// begin overlay
		renderResolve( RenderContext );
	}
}

//////////////////////////////////////////////////////////////////////////
// onViewDrawPostRender
void ScnDeferredRendererComponent::onViewDrawPostRender( ScnRenderContext& RenderContext )
{
	if( RenderContext.pViewComponent_ == OpaqueView_ )
	{
		// end opaque

	}
	else if( RenderContext.pViewComponent_ == TransparentView_ )
	{
		// end transparent
	}
	else if( RenderContext.pViewComponent_ == OverlayView_ )
	{
		// end overlay
	}
}
