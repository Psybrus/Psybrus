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
#include "System/Debug/DsUtils.h"

#include "System/SysKernel.h"

#include "Editor/Editor.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Static data.
std::array< const char*, ScnDeferredRendererComponent::TEX_MAX > ScnDeferredRendererComponent::TextureNames_ = 
{
	"aAlbedoTex",
	"aMaterialTex",
	"aNormalTex",
	"aDepthTex",
	"aHDRTex",
	"aLuminanceTex",
	"aLuminance2Tex",
	"aDownsampleBloom0Tex",
	"aDownsampleBloom1Tex",
	"aBloomTex",
	"aBloomWorkTex",
};

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
		new ReField( "DebugDrawMode_", &ScnDeferredRendererComponent::DebugDrawMode_, 0 ),
		new ReField( "Enabled_", &ScnDeferredRendererComponent::Enabled_, bcRFF_IMPORTER ),
		new ReField( "Width_", &ScnDeferredRendererComponent::Width_, bcRFF_IMPORTER ),
		new ReField( "Height_", &ScnDeferredRendererComponent::Height_, bcRFF_IMPORTER ),
		new ReField( "LightShaders_", &ScnDeferredRendererComponent::LightShaders_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "LuminanceComputeShader_", &ScnDeferredRendererComponent::LuminanceComputeShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "LuminanceTransferComputeShader_", &ScnDeferredRendererComponent::LuminanceTransferComputeShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "DownsampleComputeShader_", &ScnDeferredRendererComponent::DownsampleComputeShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "LuminanceShader_", &ScnDeferredRendererComponent::LuminanceShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "LuminanceTransferShader_", &ScnDeferredRendererComponent::LuminanceTransferShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "DownsampleShader_", &ScnDeferredRendererComponent::DownsampleShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "BloomBrightPassShader_", &ScnDeferredRendererComponent::BloomBrightPassShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "BloomHBlurShader_", &ScnDeferredRendererComponent::BloomHBlurShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "BloomVBlurShader_", &ScnDeferredRendererComponent::BloomVBlurShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "ReflectionShader_", &ScnDeferredRendererComponent::ReflectionShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "ResolveShader_", &ScnDeferredRendererComponent::ResolveShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "DebugShader_", &ScnDeferredRendererComponent::DebugShader_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "ResolveX_", &ScnDeferredRendererComponent::ResolveX_, bcRFF_IMPORTER ),
		new ReField( "ResolveY_", &ScnDeferredRendererComponent::ResolveY_, bcRFF_IMPORTER ),
		new ReField( "ResolveW_", &ScnDeferredRendererComponent::ResolveW_, bcRFF_IMPORTER ),
		new ReField( "ResolveH_", &ScnDeferredRendererComponent::ResolveH_, bcRFF_IMPORTER ),
		new ReField( "Near_", &ScnDeferredRendererComponent::Near_, bcRFF_IMPORTER ),
		new ReField( "Far_", &ScnDeferredRendererComponent::Far_, bcRFF_IMPORTER ),
		new ReField( "HorizontalFOV_", &ScnDeferredRendererComponent::HorizontalFOV_, bcRFF_IMPORTER ),
		new ReField( "VerticalFOV_", &ScnDeferredRendererComponent::VerticalFOV_, bcRFF_IMPORTER ),
		new ReField( "BloomUniformBlock_", &ScnDeferredRendererComponent::BloomUniformBlock_, bcRFF_IMPORTER ),
		new ReField( "ToneMappingUniformBlock_", &ScnDeferredRendererComponent::ToneMappingUniformBlock_, bcRFF_IMPORTER ),
		new ReField( "DownsampleUniformBlock_", &ScnDeferredRendererComponent::DownsampleUniformBlock_, bcRFF_IMPORTER ),
		new ReField( "ReflectionCubemap_", &ScnDeferredRendererComponent::ReflectionCubemap_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "UseEnvironmentProbes_", &ScnDeferredRendererComponent::UseEnvironmentProbes_, bcRFF_IMPORTER ),
		new ReField( "Textures_", &ScnDeferredRendererComponent::Textures_, bcRFF_SHALLOW_COPY | bcRFF_CONST ),
	};
	
	auto& Class = ReRegisterClass< ScnDeferredRendererComponent, Super >( Fields );
	Class.addAttribute( new ScnComponentProcessor() );

	// TODO: Move into editor only file.
	// Add editor.
	Class.addAttribute( 
		new DsImGuiFieldEditor( 
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				ScnDeferredRendererComponent* Value = (ScnDeferredRendererComponent*)Object;
				if( Value != nullptr )
				{
					Editor::ObjectEditor( ThisFieldEditor, &Value->BloomUniformBlock_, ScnShaderBloomUniformBlockData::StaticGetClass(), Flags );
					Editor::ObjectEditor( ThisFieldEditor, &Value->ToneMappingUniformBlock_, ScnShaderToneMappingUniformBlockData::StaticGetClass(), Flags );
					Editor::ObjectEditor( ThisFieldEditor, Value, Value->getClass(), Flags );
				}
			} ) );

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
	// Create textures.
	BcS32 HalfWidth = Width_ <= 0 ? Width_ - 1 : Width_ / 2;
	BcS32 HalfHeight = Height_ <= 0 ? Height_ - 1 : Height_ / 2;
	BcS32 QuarterWidth = Width_ <= 0 ? Width_ - 2 : Width_ / 4;
	BcS32 QuarterHeight = Height_ <= 0 ? Height_ - 2 : Height_ / 4;

	Textures_[ TEX_GBUFFER_ALBEDO ] = ScnTexture::New2D( Width_, Height_, 1, RsResourceFormat::R8G8B8A8_UNORM, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "Albedo" );
	Textures_[ TEX_GBUFFER_MATERIAL ] = ScnTexture::New2D( Width_, Height_, 1, RsResourceFormat::R8G8B8A8_UNORM, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "Material" );
	Textures_[ TEX_GBUFFER_NORMAL ] = ScnTexture::New2D( Width_, Height_, 1, RsResourceFormat::R10G10B10A2_UNORM, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "Normal" );
	Textures_[ TEX_GBUFFER_DEPTH ] = ScnTexture::New2D( Width_, Height_, 1, RsResourceFormat::D24_UNORM_S8_UINT, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::DEPTH_STENCIL, "Depth" );
	Textures_[ TEX_HDR ] = ScnTexture::New2D( Width_, Height_, 1, RsResourceFormat::R16G16B16A16_FLOAT, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET, "HDR" );
	Textures_[ TEX_LUMINANCE ] = ScnTexture::New2D( HalfWidth, HalfHeight, 0, RsResourceFormat::R32_FLOAT, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::UNORDERED_ACCESS | RsResourceBindFlags::RENDER_TARGET, "Luminance" );
	Textures_[ TEX_LUMINANCE2 ] = ScnTexture::New2D( 1, 1, 1, RsResourceFormat::R32_FLOAT, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::UNORDERED_ACCESS | RsResourceBindFlags::RENDER_TARGET, "Luminance2" );
	Textures_[ TEX_DOWNSAMPLE_BLOOM_0 ] = ScnTexture::New2D( HalfWidth, HalfHeight, 1, RsResourceFormat::R10G10B10A2_UNORM,
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::UNORDERED_ACCESS | RsResourceBindFlags::RENDER_TARGET, "DownsampleBloom0" );
	Textures_[ TEX_DOWNSAMPLE_BLOOM_1 ] = ScnTexture::New2D( QuarterWidth, QuarterHeight, 1, RsResourceFormat::R10G10B10A2_UNORM,
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::UNORDERED_ACCESS | RsResourceBindFlags::RENDER_TARGET, "DownsampleBloom1" );
	Textures_[ TEX_BLOOM ] = ScnTexture::New2D( QuarterWidth, QuarterHeight, 1, RsResourceFormat::R10G10B10A2_UNORM,
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::UNORDERED_ACCESS | RsResourceBindFlags::RENDER_TARGET, "Bloom" );
	Textures_[ TEX_BLOOM_WORK ] = ScnTexture::New2D( QuarterWidth, QuarterHeight, 1, RsResourceFormat::R10G10B10A2_UNORM,
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::UNORDERED_ACCESS | RsResourceBindFlags::RENDER_TARGET, "BloomWork" );

	// Subscribe for recreation after textures have been created.
	OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
		[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
		{
			recreateResources();
			return evtRET_PASS;
		} );

	// Create views.
	OpaqueView_ = getParentEntity()->attach< ScnViewComponent >(
		"OpaqueView", 3, &Textures_[ TEX_GBUFFER_ALBEDO ], Textures_[ TEX_GBUFFER_DEPTH ],
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

	// Free all framebuffers.
	LuminanceFrameBuffers_.clear();
	for( auto& FrameBuffer : FrameBuffers_ )
	{
		FrameBuffer.reset();
	}

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
	// Create frame buffers.
	FrameBuffers_[ FB_GBUFFER ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 3 )
		.setRenderTarget( 0, Textures_[ TEX_GBUFFER_ALBEDO ]->getTexture() )
		.setRenderTarget( 1, Textures_[ TEX_GBUFFER_MATERIAL ]->getTexture() )
		.setRenderTarget( 2, Textures_[ TEX_GBUFFER_NORMAL ]->getTexture() )
		.setDepthStencilTarget( Textures_[ TEX_GBUFFER_DEPTH ]->getTexture() ), "GBuffer" );

	FrameBuffers_[ FB_HDR ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 1 )
		.setRenderTarget( 0, Textures_[ TEX_HDR ]->getTexture() )
		.setDepthStencilTarget( Textures_[ TEX_GBUFFER_DEPTH ]->getTexture() ), "HDR" );

	FrameBuffers_[ FB_DOWNSAMPLE_BLOOM_0 ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 1 )
		.setRenderTarget( 0, Textures_[ TEX_DOWNSAMPLE_BLOOM_0 ]->getTexture() ), "DownsampleBloom0" );

	FrameBuffers_[ FB_DOWNSAMPLE_BLOOM_1 ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 1 )
		.setRenderTarget( 0, Textures_[ TEX_DOWNSAMPLE_BLOOM_1 ]->getTexture() ), "DownsampleBloom1" );

	FrameBuffers_[ FB_BLOOM ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 1 )
		.setRenderTarget( 0, Textures_[ TEX_BLOOM ]->getTexture() ), "Bloom" );
	
	FrameBuffers_[ FB_BLOOM_WORK ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 1 )
		.setRenderTarget( 0, Textures_[ TEX_BLOOM_WORK ]->getTexture() ), "BloomWork" );

	// Create luminance frame buffers.
	size_t LuminanceLevels = Textures_[ TEX_LUMINANCE ]->getTexture()->getDesc().Levels_;

	LuminanceFrameBuffers_.clear();
	LuminanceFrameBuffers_.resize( LuminanceLevels + 1 );
	for( BcU32 Idx = 0; Idx < LuminanceLevels; ++Idx )
	{
		LuminanceFrameBuffers_[ Idx ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 1 )
			.setRenderTarget( 0, Textures_[ TEX_LUMINANCE ]->getTexture(), Idx ), "Luminance" );
	}

	LuminanceFrameBuffers_[ LuminanceLevels ] = RsCore::pImpl()->createFrameBuffer( RsFrameBufferDesc( 1 )
		.setRenderTarget( 0, Textures_[ TEX_LUMINANCE2 ]->getTexture() ), "Luminance2" );


	// Reset bindings.
	for( auto & LightProgramBinding : LightProgramBindings_ )
	{
		LightProgramBinding.reset();
	}
	ReflectionProgramBinding_.reset();
	ResolveProgramBinding_.reset();

	if( BloomUniformBuffer_ == nullptr )
	{
		BloomUniformBuffer_ = RsCore::pImpl()->createBuffer(
			RsBufferDesc(
				RsResourceBindFlags::UNIFORM_BUFFER,
				RsResourceCreationFlags::STREAM, 
				sizeof( ScnShaderBloomUniformBlockData ) ),
			getFullName().c_str() );
		RsCore::pImpl()->updateBuffer( BloomUniformBuffer_.get(), 0, sizeof( ScnShaderBloomUniformBlockData ), 
			RsResourceUpdateFlags::ASYNC,
			[
				BloomUniformBlock = BloomUniformBlock_
			]
			( RsBuffer* Buffer, const RsBufferLock& Lock )
			{
				memcpy( Lock.Buffer_, &BloomUniformBlock, sizeof( BloomUniformBlock ) );
			} );
	}

	if( ToneMappingUniformBuffer_ == nullptr )
	{
		ToneMappingUniformBuffer_ = RsCore::pImpl()->createBuffer(
			RsBufferDesc(
				RsResourceBindFlags::UNIFORM_BUFFER,
				RsResourceCreationFlags::STREAM, 
				sizeof( ScnShaderToneMappingUniformBlockData ) ),
			getFullName().c_str() );
		RsCore::pImpl()->updateBuffer( ToneMappingUniformBuffer_.get(), 0, sizeof( ScnShaderToneMappingUniformBlockData ), 
			RsResourceUpdateFlags::ASYNC,
			[
				ToneMappingUniformBlock = ToneMappingUniformBlock_
			]
			( RsBuffer* Buffer, const RsBufferLock& Lock )
			{
				memcpy( Lock.Buffer_, &ToneMappingUniformBlock, sizeof( ToneMappingUniformBlock ) );
			} );
	}

	if( DownsampleUniformBuffer_ == nullptr )
	{
		DownsampleUniformBuffer_ = RsCore::pImpl()->createBuffer(
			RsBufferDesc(
				RsResourceBindFlags::UNIFORM_BUFFER,
				RsResourceCreationFlags::STREAM, 
				sizeof( ScnShaderDownsampleUniformBlockData ) ),
			getFullName().c_str() );
		RsCore::pImpl()->updateBuffer( DownsampleUniformBuffer_.get(), 0, sizeof( ScnShaderDownsampleUniformBlockData ), 
			RsResourceUpdateFlags::ASYNC,
			[
				DownsampleUniformBlock = DownsampleUniformBlock_
			]
			( RsBuffer* Buffer, const RsBufferLock& Lock )
			{
				memcpy( Lock.Buffer_, &DownsampleUniformBlock, sizeof( DownsampleUniformBlock ) );
			} );
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

	// Create samplers.
	RsSamplerStateDesc SamplerStateDesc;
	SamplerStateDesc.AddressU_ = RsTextureSamplingMode::CLAMP;
	SamplerStateDesc.AddressV_ = RsTextureSamplingMode::CLAMP;
	SamplerStateDesc.MinFilter_ = RsTextureFilteringMode::NEAREST;
	SamplerStateDesc.MagFilter_ = RsTextureFilteringMode::NEAREST;
	NearestSamplerState_ = RsCore::pImpl()->createSamplerState( SamplerStateDesc, getFullName().c_str() );

	SamplerStateDesc.MinFilter_ = RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR;
	SamplerStateDesc.MagFilter_ = RsTextureFilteringMode::LINEAR;
	SamplerState_ = RsCore::pImpl()->createSamplerState( SamplerStateDesc, getFullName().c_str() );

	// Create render state.
	RsRenderStateDesc RenderStateDesc;

	auto& DepthStencilState = RenderStateDesc.DepthStencilState_;
	DepthStencilState.DepthTestEnable_ = false;
	DepthStencilState.DepthWriteEnable_ = false;

	auto& BlendState = RenderStateDesc.BlendState_.RenderTarget_[ 0 ];
	BlendState.Enable_ = BcTrue;
	BlendState.SrcBlend_ = RsBlendType::ONE;
	BlendState.SrcBlendAlpha_ = RsBlendType::ONE;
	BlendState.DestBlend_ = RsBlendType::ONE;
	BlendState.DestBlendAlpha_ = RsBlendType::ONE;
	BlendState.BlendOp_ = RsBlendOp::ADD;
	BlendState.BlendOpAlpha_ = RsBlendOp::ADD;
	AdditiveRenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, getFullName().c_str() );

	BlendState.SrcBlend_ = RsBlendType::SRC_ALPHA;
	BlendState.SrcBlendAlpha_ = RsBlendType::ONE;
	BlendState.DestBlend_ = RsBlendType::INV_SRC_ALPHA;
	BlendState.DestBlendAlpha_ = RsBlendType::ONE;
	BlendRenderState_ = RsCore::pImpl()->createRenderState( RenderStateDesc, getFullName().c_str() );

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
// setTextures
void ScnDeferredRendererComponent::setTextures( RsProgram* Program, RsProgramBindingDesc& ProgramBindingDesc )
{
	for( size_t Idx = 0; Idx < TEX_MAX; ++Idx )
	{
		BcU32 SRVSlot = Program->findShaderResourceSlot( TextureNames_[ Idx ] );
		BcU32 SamplerSlot = Program->findSamplerSlot( TextureNames_[ Idx ] );
		if( SRVSlot != BcErrorCode )
		{
			ProgramBindingDesc.setShaderResourceView( SRVSlot, Textures_[ Idx ]->getTexture() );
		}

		if( SamplerSlot != BcErrorCode )
		{
			ProgramBindingDesc.setSamplerState( SamplerSlot, SamplerState_.get() );
		}
	}
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

			setTextures( Program, ProgramBindingDesc );
			OpaqueView_->setViewResources( Program, ProgramBindingDesc );

			// TODO: Tailored uniform buffer for deferred.
			{
				BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderLightUniformBlockData" );
				if( UniformSlot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( UniformSlot, UniformBuffer_.get(), 0, sizeof( ScnShaderLightUniformBlockData ) );
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
	ScnCore::pImpl()->visitView( this, RenderContext.View_ );


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
				FrameBuffer = RenderContext.View_->getFrameBuffer(),
				Viewport = RenderContext.View_->getViewport()
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
				PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent::renderLights" );

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
		if( NearestEnvironmentMap && NearestEnvironmentMap != ReflectionCubemap_ )
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

		setTextures( Program, ProgramBindingDesc );
		OpaqueView_->setViewResources( Program, ProgramBindingDesc );

		// Reflection cubemap.
		{
			BcU32 SRVSlot = Program->findShaderResourceSlot( "aReflectionTex" );
			BcU32 SamplerSlot = Program->findSamplerSlot( "aReflectionTex" );
			ProgramBindingDesc.setShaderResourceView( SRVSlot, ReflectionCubemap_->getTexture() );
			ProgramBindingDesc.setSamplerState( SamplerSlot, SamplerState_.get() );
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
			FrameBuffer = RenderContext.View_->getFrameBuffer()
		]
		( RsContext* Context )
		{
			PSY_PROFILE_FUNCTION;
			PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent::renderReflection" );

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
// downsampleHDR
void ScnDeferredRendererComponent::downsampleHDR( ScnRenderContext& RenderContext )
{
	// Setup transfer rate based on timer.
	// TODO: Need to make sure this will be consistent across different frame times. Work out the correct curve for it later.
	ToneMappingUniformBlock_.ToneMappingLuminanceTransferRate_ = BcClamp( SysKernel::pImpl()->getFrameTime() * 0.5f, 0.0f, 1.0f );

	// Update tone mapping params.
	RsCore::pImpl()->updateBuffer( ToneMappingUniformBuffer_.get(), 0, sizeof( ScnShaderToneMappingUniformBlockData ), 
		RsResourceUpdateFlags::ASYNC,
		[
			ToneMappingUniformBlock = ToneMappingUniformBlock_
		]
		( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			memcpy( Lock.Buffer_, &ToneMappingUniformBlock, sizeof( ToneMappingUniformBlock ) );
		} );

	// Downsample all mip levels.
	// TODO: Have compute use be configurable, for now keep it disabled for testing purposes.
	bool UseCompute = false && RenderContext.pFrame_->getContext()->getFeatures().ComputeShaders_;
	if( UseCompute )
	{
		// Generate top level luminance mip.
		{
			auto* LumunanceProgram = LuminanceComputeShader_->getProgram( ScnShaderPermutationFlags::NONE );
			auto InputSRVSlot = LumunanceProgram->findShaderResourceSlot( "aInputTexture" );
			auto OutputUAVSlot = LumunanceProgram->findUnorderedAccessSlot( "aOutputTexture" );
			RsProgramBindingDesc BindingDesc;

			auto* InputTexture = Textures_[ TEX_HDR ]->getTexture();
			auto* OutputTexture = Textures_[ TEX_LUMINANCE ]->getTexture();

			BindingDesc.setShaderResourceView( InputSRVSlot, InputTexture, 0, 1, 0, 1 );
			BindingDesc.setUnorderedAccessView( OutputUAVSlot, OutputTexture, 0, 0, 1 );
			auto ProgramBinding = RsCore::pImpl()->createProgramBinding( LumunanceProgram, BindingDesc, (*getName()).c_str() );

			RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
				[ 
					ProgramBinding = ProgramBinding.get(),
					XGroups = OutputTexture->getDesc().Width_,
					YGroups = OutputTexture->getDesc().Height_
				]
				( RsContext* Context )
				{
					PSY_PROFILE_FUNCTION;
					PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent calc lum" );
					Context->dispatchCompute( ProgramBinding, XGroups, YGroups, 1 );
				} );
		}

		// Downsample.
		{
			auto* DownsampleProgram = DownsampleComputeShader_->getProgram( ScnShaderPermutationFlags::NONE );
			auto InputSRVSlot = DownsampleProgram->findShaderResourceSlot( "aInputTexture" );
			auto OutputUAVSlot = DownsampleProgram->findUnorderedAccessSlot( "aOutputTexture" );
			RsProgramBindingDesc BindingDesc;

			auto* InputTexture = Textures_[ TEX_LUMINANCE ]->getTexture();
			auto* OutputTexture = Textures_[ TEX_LUMINANCE ]->getTexture();

			for( BcU32 Level = 1; Level < OutputTexture->getDesc().Levels_; ++Level )
			{
				BindingDesc.setShaderResourceView( InputSRVSlot, InputTexture, Level - 1, 1, 0, 1 );
				BindingDesc.setUnorderedAccessView( OutputUAVSlot, OutputTexture, Level, 0, 1 );
				auto ProgramBinding = RsCore::pImpl()->createProgramBinding( DownsampleProgram, BindingDesc, (*getName()).c_str() );

				RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
					[ 
						ProgramBinding = ProgramBinding.get(),
						XGroups = std::max( BcU32( 1 ), OutputTexture->getDesc().Width_ >> Level ),
						YGroups = std::max( BcU32( 1 ), OutputTexture->getDesc().Height_ >> Level )
					]
					( RsContext* Context )
					{
						PSY_PROFILE_FUNCTION;
						PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent downsample" );
						Context->dispatchCompute( ProgramBinding, XGroups, YGroups, 1 );
					} );
			}
		}

		// Transfer luminance to 2nd target.
		{
			auto* LuminanceTransferProgram = LuminanceTransferComputeShader_->getProgram( ScnShaderPermutationFlags::NONE );
			auto InputSRVSlot = LuminanceTransferProgram->findShaderResourceSlot( "aInputTexture" );
			auto OutputUAVSlot = LuminanceTransferProgram->findUnorderedAccessSlot( "aOutputTexture" );
			RsProgramBindingDesc BindingDesc;

			auto* InputTexture = Textures_[ TEX_LUMINANCE ]->getTexture();
			auto* OutputTexture = Textures_[ TEX_LUMINANCE2 ]->getTexture();

			BindingDesc.setShaderResourceView( InputSRVSlot, InputTexture, InputTexture->getDesc().Levels_ - 1, 1, 0, 1 );
			BindingDesc.setUnorderedAccessView( OutputUAVSlot, OutputTexture, 0, 0, 1 );

			{
				BcU32 UniformSlot = LuminanceTransferProgram->findUniformBufferSlot( "ScnShaderToneMappingUniformBlockData" );
				if( UniformSlot != BcErrorCode )
				{
					BindingDesc.setUniformBuffer( UniformSlot, ToneMappingUniformBuffer_.get(), 0, sizeof( ScnShaderToneMappingUniformBlockData ) );
				}
			}

			auto ProgramBinding = RsCore::pImpl()->createProgramBinding( LuminanceTransferProgram, BindingDesc, (*getName()).c_str() );

			RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
				[ 
					ProgramBinding = ProgramBinding.get(),
					XGroups = 1,
					YGroups = 1
				]
				( RsContext* Context )
				{
					PSY_PROFILE_FUNCTION;
					PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent transfer lum" );
					Context->dispatchCompute( ProgramBinding, XGroups, YGroups, 1 );
				} );
		}
	}
	else
	{
		const ScnShaderPermutationFlags Permutation = 
			ScnShaderPermutationFlags::RENDER_POST_PROCESS |
			ScnShaderPermutationFlags::PASS_MAIN |
			ScnShaderPermutationFlags::MESH_STATIC_2D;

		MaVec2d UVSize( 1.0f, 1.0f );
		auto AlbedoTex = Textures_[ TEX_GBUFFER_ALBEDO ];
		auto Rect = AlbedoTex->getRect( 0 );			
		UVSize.x( Rect.W_ );
		UVSize.y( Rect.H_ );

		RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
			[ 
				UVSize = UVSize,
				GeometryBinding = GeometryBinding_.get()
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
				PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent setup quad" );

				setupQuad( Context, GeometryBinding->getDesc().VertexDeclaration_,
					GeometryBinding->getDesc().VertexBuffers_[ 0 ].Buffer_,
					MaVec2d( -1.0f, -1.0f ), MaVec2d( 1.0f, 1.0f ), UVSize );
			} );

		// Generate top level luminance mip.
		{
			auto* LuminanceProgram = LuminanceShader_->getProgram( Permutation );
			auto InputSRVSlot = LuminanceProgram->findShaderResourceSlot( "aHDRTexture" );
			RsProgramBindingDesc BindingDesc;

			auto* HDRTexture = Textures_[ TEX_HDR ]->getTexture();

			BindingDesc.setShaderResourceView( InputSRVSlot, HDRTexture, 0, 1, 0, 1 );
			
			auto ProgramBinding = RsCore::pImpl()->createProgramBinding( LuminanceProgram, BindingDesc, (*getName()).c_str() );

			RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
				[ 
					UniformBuffer = UniformBuffer_.get(),
					GeometryBinding = GeometryBinding_.get(),
					ProgramBinding = ProgramBinding.get(),
					RenderState = ResolveRenderState_.get(),
					FrameBuffer = LuminanceFrameBuffers_[ 0 ].get()
				]
				( RsContext* Context )
				{
					PSY_PROFILE_FUNCTION;
					PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent calc lum" );
			
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

		// Downsample.
		{
			auto* DownsampleProgram = DownsampleShader_->getProgram( Permutation );
			auto InputSRVSlot = DownsampleProgram->findShaderResourceSlot( "aInputTexture" );
			RsProgramBindingDesc BindingDesc;

			auto* LuminanceTexture = Textures_[ TEX_LUMINANCE ]->getTexture();
			
			for( BcU32 Level = 1; Level < LuminanceTexture->getDesc().Levels_; ++Level )
			{
				BindingDesc.setShaderResourceView( InputSRVSlot, LuminanceTexture, Level - 1, 1, 0, 1 );

				{
					BcU32 UniformSlot = DownsampleProgram->findUniformBufferSlot( "ScnShaderDownsampleUniformBlockData" );
					if( UniformSlot != BcErrorCode )
					{
						BindingDesc.setUniformBuffer( UniformSlot, DownsampleUniformBuffer_.get(), 0, sizeof( ScnShaderDownsampleUniformBlockData ) );
					}
				}

				auto ProgramBinding = RsCore::pImpl()->createProgramBinding( DownsampleProgram, BindingDesc, (*getName()).c_str() );

				DownsampleUniformBlock_.DownsampleSourceMipLevel_ = Level - 1;

				RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
					[ 
						Level = Level,
						GeometryBinding = GeometryBinding_.get(),
						ProgramBinding = ProgramBinding.get(),
						RenderState = ResolveRenderState_.get(),
						FrameBuffer = LuminanceFrameBuffers_[ Level ].get(),
						DownsampleUniformBuffer = DownsampleUniformBuffer_.get(),
						DownsampleUniformBlock = DownsampleUniformBlock_
					]
					( RsContext* Context )
					{
						PSY_PROFILE_FUNCTION;
						PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent downsample" );

						Context->updateBuffer( DownsampleUniformBuffer, 0, sizeof( ScnShaderDownsampleUniformBlockData ), 
							RsResourceUpdateFlags::ASYNC,
							[
								DownsampleUniformBlock = DownsampleUniformBlock
							]
							( RsBuffer* Buffer, const RsBufferLock& Lock )
							{
								memcpy( Lock.Buffer_, &DownsampleUniformBlock, sizeof( DownsampleUniformBlock ) );
							} );

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
		}

		// Transfer luminance to 2nd target.
		{
			auto* LuminanceTransferProgram = LuminanceTransferShader_->getProgram( Permutation );
			auto InputSRVSlot = LuminanceTransferProgram->findShaderResourceSlot( "aLuminanceTexture" );
			RsProgramBindingDesc BindingDesc;

			auto* LuminanceTexture = Textures_[ TEX_LUMINANCE ]->getTexture();

			BindingDesc.setShaderResourceView( InputSRVSlot, LuminanceTexture, LuminanceTexture->getDesc().Levels_ - 1, 1, 0, 1 );
			
			{
				BcU32 UniformSlot = LuminanceTransferProgram->findUniformBufferSlot( "ScnShaderToneMappingUniformBlockData" );
				if( UniformSlot != BcErrorCode )
				{
					BindingDesc.setUniformBuffer( UniformSlot, ToneMappingUniformBuffer_.get(), 0, sizeof( ScnShaderToneMappingUniformBlockData ) );
				}
			}

			auto ProgramBinding = RsCore::pImpl()->createProgramBinding( LuminanceTransferProgram, BindingDesc, (*getName()).c_str() );

			RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
				[
					GeometryBinding = GeometryBinding_.get(),
					ProgramBinding = ProgramBinding.get(),
					RenderState = BlendRenderState_.get(),
					FrameBuffer = LuminanceFrameBuffers_[ LuminanceFrameBuffers_.size() - 1 ].get()
				]
				( RsContext* Context )
				{
					PSY_PROFILE_FUNCTION;
					PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent transfer lum" );

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
	}
}

//////////////////////////////////////////////////////////////////////////
// renderResolve
void ScnDeferredRendererComponent::renderResolve( ScnRenderContext& RenderContext )
{
	// HACK: Create resolve.
	const ScnShaderPermutationFlags Permutation = 
		ScnShaderPermutationFlags::RENDER_POST_PROCESS |
		ScnShaderPermutationFlags::PASS_MAIN |
		ScnShaderPermutationFlags::MESH_STATIC_2D;
	auto ResolveProgram = ResolveShader_->getProgram( Permutation );
	if( ResolveProgramBinding_ == nullptr || ResolveProgramBinding_->getProgram() != ResolveProgram || DebugDrawMode_ != 0 )
	{
		// Create program binding for lighting view.
		RsProgramBindingDesc ProgramBindingDesc;
		auto Program = DebugDrawMode_== 0 ? 
			ResolveShader_->getProgram( Permutation ) : 
			DebugShader_->getProgram( Permutation );

		setTextures( Program, ProgramBindingDesc );
		OpaqueView_->setViewResources( Program, ProgramBindingDesc );

		BcU32 UniformSlot = Program->findUniformBufferSlot( "ScnShaderToneMappingUniformBlockData" );
		if( UniformSlot != BcErrorCode )
		{
			ProgramBindingDesc.setUniformBuffer( UniformSlot, ToneMappingUniformBuffer_.get(), 0, sizeof( ScnShaderToneMappingUniformBlockData ) );
		}

		if( DebugDrawMode_ != 0 )
		{
			BcU32 InputSlot = Program->findShaderResourceSlot( "aInputTex" );
			if( DebugDrawMode_ > 0 && DebugDrawMode_ <= TEX_MAX )
			{
				ProgramBindingDesc.setShaderResourceView( InputSlot, Textures_[ DebugDrawMode_ - 1 ]->getTexture() );
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
			PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent resolve" );

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
// calculateBloom
void ScnDeferredRendererComponent::calculateBloom( ScnRenderContext& RenderContext )
{
	// No bloom.
	if( !BloomBrightPassShader_ )
	{
		return;
	}

	// Update bloom params.
	RsCore::pImpl()->updateBuffer( BloomUniformBuffer_.get(), 0, sizeof( ScnShaderBloomUniformBlockData ), 
		RsResourceUpdateFlags::ASYNC,
		[
			BloomUniformBlock = BloomUniformBlock_
		]
		( RsBuffer* Buffer, const RsBufferLock& Lock )
		{
			memcpy( Lock.Buffer_, &BloomUniformBlock, sizeof( BloomUniformBlock ) );
		} );

	const ScnShaderPermutationFlags Permutation = 
		ScnShaderPermutationFlags::RENDER_POST_PROCESS |
		ScnShaderPermutationFlags::PASS_MAIN |
		ScnShaderPermutationFlags::MESH_STATIC_2D;

	MaVec2d UVSize( 1.0f, 1.0f );
	auto AlbedoTex = Textures_[ TEX_GBUFFER_ALBEDO ];
	auto Rect = AlbedoTex->getRect( 0 );			
	UVSize.x( Rect.W_ );
	UVSize.y( Rect.H_ );

	RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
		[ 
			UVSize = UVSize,
			GeometryBinding = GeometryBinding_.get()
		]
		( RsContext* Context )
		{
			PSY_PROFILE_FUNCTION;
			PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent setup quad" );

			setupQuad( Context, GeometryBinding->getDesc().VertexDeclaration_,
				GeometryBinding->getDesc().VertexBuffers_[ 0 ].Buffer_,
				MaVec2d( -1.0f, -1.0f ), MaVec2d( 1.0f, 1.0f ), UVSize );
		} );

	// Downsample.
	{
		auto* DownsampleProgram = DownsampleShader_->getProgram( Permutation );
		auto InputSRVSlot = DownsampleProgram->findShaderResourceSlot( "aInputTexture" );
		BcU32 UniformSlot = DownsampleProgram->findUniformBufferSlot( "ScnShaderDownsampleUniformBlockData" );
		RsProgramBindingDesc BindingDesc;
		
		DownsampleUniformBlock_.DownsampleSourceMipLevel_ = 0;
		RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
			[ 
				GeometryBinding = GeometryBinding_.get(),
				DownsampleUniformBuffer = DownsampleUniformBuffer_.get(),
				DownsampleUniformBlock = DownsampleUniformBlock_
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
				PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent downsample" );

				Context->updateBuffer( DownsampleUniformBuffer, 0, sizeof( ScnShaderDownsampleUniformBlockData ), 
					RsResourceUpdateFlags::ASYNC,
					[
						DownsampleUniformBlock = DownsampleUniformBlock
					]
					( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						memcpy( Lock.Buffer_, &DownsampleUniformBlock, sizeof( DownsampleUniformBlock ) );
					} );
			} );

		// Downsample 0. 
		{
			BindingDesc.setShaderResourceView( InputSRVSlot, Textures_[ TEX_HDR ]->getTexture(), 0, 1, 0, 1 );
			BindingDesc.setUniformBuffer( UniformSlot, DownsampleUniformBuffer_.get(), 0, sizeof( ScnShaderDownsampleUniformBlockData ) );

			auto ProgramBinding = RsCore::pImpl()->createProgramBinding( DownsampleProgram, BindingDesc, (*getName()).c_str() );

			RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
				[ 
					GeometryBinding = GeometryBinding_.get(),
					ProgramBinding = ProgramBinding.get(),
					RenderState = ResolveRenderState_.get(),
					FrameBuffer = FrameBuffers_[ FB_DOWNSAMPLE_BLOOM_0 ].get()
				]
				( RsContext* Context )
				{
					PSY_PROFILE_FUNCTION;
					PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent downsample 0" );

					Context->drawPrimitives( 
						GeometryBinding,
						ProgramBinding,
						RenderState,
						FrameBuffer,
						nullptr,
						nullptr,
						RsTopologyType::TRIANGLE_STRIP, 0, 4, 0, 1 );
				} );
		}

		// Downsample 1.
		{
			BindingDesc.setShaderResourceView( InputSRVSlot, Textures_[ TEX_DOWNSAMPLE_BLOOM_0 ]->getTexture(), 0, 1, 0, 1 );
			BindingDesc.setUniformBuffer( UniformSlot, DownsampleUniformBuffer_.get(), 0, sizeof( ScnShaderDownsampleUniformBlockData ) );

			auto ProgramBinding = RsCore::pImpl()->createProgramBinding( DownsampleProgram, BindingDesc, (*getName()).c_str() );

			RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
				[ 
					GeometryBinding = GeometryBinding_.get(),
					ProgramBinding = ProgramBinding.get(),
					RenderState = ResolveRenderState_.get(),
					FrameBuffer = FrameBuffers_[ FB_DOWNSAMPLE_BLOOM_1 ].get()
				]
				( RsContext* Context )
				{
					PSY_PROFILE_FUNCTION;
					PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent downsample 1" );

					Context->drawPrimitives( 
						GeometryBinding,
						ProgramBinding,
						RenderState,
						FrameBuffer,
						nullptr,
						nullptr,
						RsTopologyType::TRIANGLE_STRIP, 0, 4, 0, 1 );
				} );
		}
	}

	// Generate bloom target.
	{
		auto* BloomBrightPassProgram = BloomBrightPassShader_->getProgram( Permutation );
		RsProgramBindingDesc BindingDesc;

		setTextures( BloomBrightPassProgram, BindingDesc );
			
		BcU32 UniformSlot = BloomBrightPassProgram->findUniformBufferSlot( "ScnShaderBloomUniformBlockData" );
		if( UniformSlot != BcErrorCode )
		{
			BindingDesc.setUniformBuffer( UniformSlot, BloomUniformBuffer_.get(), 0, sizeof( ScnShaderBloomUniformBlockData ) );
		}

		auto InputSRVSlot = BloomBrightPassProgram->findShaderResourceSlot( "aInputTexture" );
		BindingDesc.setShaderResourceView( InputSRVSlot, Textures_[ TEX_DOWNSAMPLE_BLOOM_1 ]->getTexture(), 0, 1, 0, 1 );

		auto ProgramBinding = RsCore::pImpl()->createProgramBinding( BloomBrightPassProgram, BindingDesc, (*getName()).c_str() );

		RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
			[ 
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = ProgramBinding.get(),
				RenderState = ResolveRenderState_.get(),
				FrameBuffer = FrameBuffers_[ FB_BLOOM ].get()
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
				PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent bloom bright pass" );
			
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

	// Horizontal blur.
	{
		auto* BloomBlurProgram = BloomHBlurShader_->getProgram( Permutation );
		auto InputSRVSlot = BloomBlurProgram->findShaderResourceSlot( "aInputTexture" );
		RsProgramBindingDesc BindingDesc;

		auto* InputTexture = Textures_[ TEX_BLOOM ]->getTexture();
		BindingDesc.setShaderResourceView( InputSRVSlot, InputTexture, 0, 1, 0, 1 );
		setTextures( BloomBlurProgram, BindingDesc );

		BcU32 UniformSlot = BloomBlurProgram->findUniformBufferSlot( "ScnShaderBloomUniformBlockData" );
		if( UniformSlot != BcErrorCode )
		{
			BindingDesc.setUniformBuffer( UniformSlot, BloomUniformBuffer_.get(), 0, sizeof( ScnShaderBloomUniformBlockData ) );
		}

		auto ProgramBinding = RsCore::pImpl()->createProgramBinding( BloomBlurProgram, BindingDesc, (*getName()).c_str() );

		RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
			[ 
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = ProgramBinding.get(),
				RenderState = ResolveRenderState_.get(),
				FrameBuffer = FrameBuffers_[ FB_BLOOM_WORK ].get()
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
				PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent horizontal blur" );

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

	// Vertical blur.
	{
		auto* BloomBlurProgram = BloomVBlurShader_->getProgram( Permutation );
		auto InputSRVSlot = BloomBlurProgram->findShaderResourceSlot( "aInputTexture" );
		RsProgramBindingDesc BindingDesc;

		auto* InputTexture = Textures_[ TEX_BLOOM_WORK ]->getTexture();
		BindingDesc.setShaderResourceView( InputSRVSlot, InputTexture, 0, 1, 0, 1 );
		setTextures( BloomBlurProgram, BindingDesc );
			
		BcU32 UniformSlot = BloomBlurProgram->findUniformBufferSlot( "ScnShaderBloomUniformBlockData" );
		if( UniformSlot != BcErrorCode )
		{
			BindingDesc.setUniformBuffer( UniformSlot, BloomUniformBuffer_.get(), 0, sizeof( ScnShaderBloomUniformBlockData ) );
		}

		auto ProgramBinding = RsCore::pImpl()->createProgramBinding( BloomBlurProgram, BindingDesc, (*getName()).c_str() );

		RenderContext.pFrame_->queueRenderNode( RenderContext.Sort_,
			[ 
				GeometryBinding = GeometryBinding_.get(),
				ProgramBinding = ProgramBinding.get(),
				RenderState = ResolveRenderState_.get(),
				FrameBuffer = FrameBuffers_[ FB_BLOOM ].get()
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
				PSY_PROFILER_GPU_SECTION( UpdateRoot, "ScnDeferredRendererComponent vertical blur" );

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
}

//////////////////////////////////////////////////////////////////////////
// onViewDrawPreRender
void ScnDeferredRendererComponent::onViewDrawPreRender( ScnRenderContext& RenderContext )
{
	if( RenderContext.View_ == OpaqueView_ )
	{
		// begin opaque
	}
	else if( RenderContext.View_ == TransparentView_ )
	{
		// begin transparent
		renderLights( RenderContext );
		renderReflection( RenderContext );
	}
	else if( RenderContext.View_ == OverlayView_ )
	{
		// begin overlay
		renderResolve( RenderContext );
	}
}

//////////////////////////////////////////////////////////////////////////
// onViewDrawPostRender
void ScnDeferredRendererComponent::onViewDrawPostRender( ScnRenderContext& RenderContext )
{
	if( RenderContext.View_ == OpaqueView_ )
	{
		// end opaque

	}
	else if( RenderContext.View_ == TransparentView_ )
	{
		// end transparent
		downsampleHDR( RenderContext );
		calculateBloom( RenderContext );

		// Do debug rendering.
		Debug::Render(
			RenderContext.pFrame_,
			RenderContext.View_->getFrameBuffer(),
			RenderContext.View_->getViewport(),
			RenderContext.View_->getViewUniformBuffer(),
			RenderContext.Sort_ );
	}
	else if( RenderContext.View_ == OverlayView_ )
	{
#if !PSY_PRODUCTION
		if( Enabled_ && DsCore::pImpl() )
		{
			RsViewport Viewport;
			if( ResolveTarget_ )
			{
				auto FBDesc = ResolveTarget_->getDesc();
				auto Desc = FBDesc.RenderTargets_[ 0 ].Texture_->getDesc();
				Viewport = RsViewport( 
					BcU32( ResolveX_ * Desc.Width_ ),
					BcU32( ResolveY_ * Desc.Height_ ),
					BcU32( ResolveW_ * Desc.Width_ ),
					BcU32( ResolveH_ * Desc.Height_ ) );
			}
			else
			{
				auto Client = OsCore::pImpl()->getClient( 0 );
				Viewport = RsViewport( 
					BcU32( ResolveX_ * Client->getWidth() ),
					BcU32( ResolveY_ * Client->getHeight() ),
					BcU32( ResolveW_ * Client->getWidth() ),
					BcU32( ResolveH_ * Client->getHeight() ) );
			}

			DsCore::pImpl()->addViewOverlay( 
				OpaqueView_->getViewTransform(), 
				OpaqueView_->getProjectionTransform(), 
				Viewport );
		}
#endif
		// end overlay
	}
}
