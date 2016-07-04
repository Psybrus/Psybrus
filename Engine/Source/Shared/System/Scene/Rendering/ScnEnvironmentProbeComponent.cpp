#include "System/Scene/Rendering/ScnEnvironmentProbeComponent.h"
#include "System/Scene/Rendering/ScnDeferredRendererComponent.h"
#include "System/Scene/Rendering/ScnTexture.h"

#include "System/Scene/ScnCore.h"

#include "System/Content/CsCore.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// ScnEnvironmentFilterUniformBlock
REFLECTION_DEFINE_BASIC( ScnEnvironmentFilterUniformBlock );

void ScnEnvironmentFilterUniformBlock::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "EnvironmentFilterCubeFace_", &ScnEnvironmentFilterUniformBlock::EnvironmentFilterCubeFace_ ),
		new ReField( "EnvironmentFilterRoughness_", &ScnEnvironmentFilterUniformBlock::EnvironmentFilterRoughness_ ),
	};
		
	auto& Class = ReRegisterClass< ScnEnvironmentFilterUniformBlock >( Fields );
	Class.addAttribute( new ScnShaderDataAttribute( "EnvironmentFilter", BcFalse ) );
	Class.setFlags( bcRFF_POD );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEnvironmentProbeProcessor::ScnEnvironmentProbeProcessor():
	ScnComponentProcessor( {
		ScnComponentProcessFuncEntry(
			"Update Probes",
			ScnComponentPriority::VIEW_RENDER - 1,
			std::bind( &ScnEnvironmentProbeProcessor::updateProbes, this, std::placeholders::_1 ) ) } )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ScnEnvironmentProbeProcessor::~ScnEnvironmentProbeProcessor()
{
}

//////////////////////////////////////////////////////////////////////////
// getProbeEnvironmentMap
ScnTexture* ScnEnvironmentProbeProcessor::getProbeEnvironmentMap( const MaVec3d& Position ) const
{
	// TODO: Distance check.
	if( EnvironmentProbes_.size() > 0 )
	{
		return EnvironmentProbes_[ 0 ]->Texture_;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnEnvironmentProbeProcessor::initialise()
{
	ScnCore::pImpl()->addCallback( this );

	UseCompute_ = RsCore::pImpl()->getContext( nullptr )->getFeatures().ComputeShaders_;

	ShaderPackage_ = CsCore::pImpl()->requestPackage( "pp_shaders" );
	ShaderPackage_->acquire();
	CsCore::pImpl()->requestPackageReadyCallback( ShaderPackage_->getName(), 
		[ this ]( CsPackage* Package, BcU32 )
		{
			BcAssert( Package == ShaderPackage_ );
			if( UseCompute_ )
			{
				ScnShaderRef Shader;
				CsCore::pImpl()->requestResource( 
					ShaderPackage_->getName(), "pp_compute_filter_r8g8b8a8", Shader );
				BcAssert( Shader );
				FilterProgram_ = Shader->getProgram( ScnShaderPermutationFlags::NONE );
				BcAssert( FilterProgram_ );
			}
		}, 0 );

	RsSamplerStateDesc SamplerDesc;
	SamplerDesc.MinFilter_ = RsTextureFilteringMode::LINEAR;
	SamplerDesc.MagFilter_ = RsTextureFilteringMode::LINEAR;
	Sampler_ = RsCore::pImpl()->createSamplerState( SamplerDesc, "pp_compute_filter_r8g8b8a8" );

	FilterUniform_ = RsCore::pImpl()->createBuffer( RsBufferDesc( 
		RsResourceBindFlags::UNIFORM_BUFFER, RsResourceCreationFlags::STREAM, 
		sizeof( ScnEnvironmentFilterUniformBlock ) ), "pp_compute_filter_r8g8b8a8" );
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void ScnEnvironmentProbeProcessor::shutdown()
{
	ScnCore::pImpl()->removeCallback( this );
	ShaderPackage_->release();
	Sampler_.reset();
	FilterUniform_.reset();
}

//////////////////////////////////////////////////////////////////////////
// updateProbes
void ScnEnvironmentProbeProcessor::updateProbes( const ScnComponentList& Components )
{
	if( !ShaderPackage_->isReady() )
	{
		return;
	}

	// Get context.
	RsContext* Context = RsCore::pImpl()->getContext( nullptr );

	if( Components.size() > 0 )
	{
		for( size_t Idx = 0; Idx < Components.size(); ++Idx )
		{
			auto Component = Components[ Idx ];
			auto* ProbeComponent = static_cast< ScnEnvironmentProbeComponent* >( Component.get() );
			MaVec3d Position = ProbeComponent->getParentEntity()->getWorldPosition();
			if( ( Position - ProbeComponent->GeneratedWorldPosition_ ).magnitudeSquared() > 1e6f )
			{
				ProbeUpdateQueue_.push_back( ProbeComponent );
			}
		}

		// Allocate a frame, but without present.
		RsFrame* Frame = RsCore::pImpl()->allocateFrame( Context, false );

		// Add a single probe to generate.
		if( ProbeUpdateQueue_.size() > 0 )
		{
			auto* ProbeComponent = ProbeUpdateQueue_.front();
			ProbeUpdateQueue_.pop_front();
				
			// TODO: Renderer interface component?
			auto Renderer = ProbeComponent->Renderer_->getComponentByType< ScnDeferredRendererComponent >();

			RsRenderSort Sort( 0 );

			// Setup projection for the capture.
			Renderer->setProjectionParams( 0.1f, 5000.0f, BcPIDIV4, 0.0f );
			
			// Get renderer position.
			ProbeComponent->GeneratedWorldPosition_ = ProbeComponent->getParentEntity()->getWorldPosition();
			MaMat4d Transform;

			// POSITIVE_X
			Transform.rotation( MaVec3d( 0.0f, -BcPIDIV2, 0.0f ) );
			Transform.translation( ProbeComponent->GeneratedWorldPosition_ );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 0 ].get(), Sort );
			Sort.RenderTarget_++;

			// NEGATIVE_X
			Transform.rotation( MaVec3d( 0.0f, BcPIDIV2, 0.0f ) );
			Transform.translation( ProbeComponent->GeneratedWorldPosition_ );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 1 ].get(), Sort );
			Sort.RenderTarget_++;

			// POSITIVE_Y
			Transform.rotation( MaVec3d( BcPIDIV2, 0.0f, 0.0f ) );
			Transform.translation( ProbeComponent->GeneratedWorldPosition_ );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 2 ].get(), Sort );
			Sort.RenderTarget_++;

			// NEGATIVE_Y
			Transform.rotation( MaVec3d( -BcPIDIV2, 0.0f, 0.0f ) );
			Transform.translation( ProbeComponent->GeneratedWorldPosition_ );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 3 ].get(), Sort );
			Sort.RenderTarget_++;

			// POSITIVE_Z
			Transform.rotation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
			Transform.translation( ProbeComponent->GeneratedWorldPosition_ );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 4 ].get(), Sort );
			Sort.RenderTarget_++;

			// NEGATIVE_Z
			Transform.rotation( MaVec3d( 0.0f, BcPI, 0.0f ) );
			Transform.translation( ProbeComponent->GeneratedWorldPosition_ );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 5 ].get(), Sort );
			Sort.RenderTarget_++;

			// Generate mip map chain.
			auto Texture = ProbeComponent->Texture_->getTexture();
			for( size_t Idx = 1; Idx < Texture->getDesc().Levels_; ++Idx )
			{
				generateMipLevel( Frame, Sort, Texture, Idx, RsTextureFace::POSITIVE_X );
				generateMipLevel( Frame, Sort, Texture, Idx, RsTextureFace::NEGATIVE_X );
				generateMipLevel( Frame, Sort, Texture, Idx, RsTextureFace::POSITIVE_Y );
				generateMipLevel( Frame, Sort, Texture, Idx, RsTextureFace::NEGATIVE_Y );
				generateMipLevel( Frame, Sort, Texture, Idx, RsTextureFace::POSITIVE_Z );
				generateMipLevel( Frame, Sort, Texture, Idx, RsTextureFace::NEGATIVE_Z );
			}
		}

		// Queue frame for render.
		RsCore::pImpl()->queueFrame( Frame );
	}
}

//////////////////////////////////////////////////////////////////////////
// generateMipLevel
void ScnEnvironmentProbeProcessor::generateMipLevel( RsFrame* Frame, RsRenderSort Sort, RsTexture* Texture, size_t Level, RsTextureFace Face )
{
	BcAssert( Texture );
	BcAssert( Level > 0 );
	BcAssert( Level < Texture->getDesc().Levels_ );

	BcF32 Roughness = BcF32( Level ) / BcF32( Texture->getDesc().Levels_ - 1 );

	if( UseCompute_ )
	{
		auto InputSamplerSlot = FilterProgram_->findSamplerSlot( "aInputTexture" );
		auto InputSRVSlot = FilterProgram_->findShaderResourceSlot( "aInputTexture" );
		auto OutputUAVSlot = FilterProgram_->findUnorderedAccessSlot( "aOutputTexture" );
		auto UBSlot = FilterProgram_->findUniformBufferSlot( "ScnEnvironmentFilterUniformBlock" );
		RsProgramBindingDesc BindingDesc;
		if( InputSamplerSlot != BcErrorCode )
		{
			BindingDesc.setSamplerState( InputSamplerSlot, Sampler_.get() );
		}
		BindingDesc.setShaderResourceView( InputSRVSlot, Texture, 0, 1, BcU32( Face ), 1 );
		BindingDesc.setUnorderedAccessView( OutputUAVSlot, Texture, Level, BcU32( Face ), 1 );
		BindingDesc.setUniformBuffer( UBSlot, FilterUniform_.get(), 0, sizeof( ScnEnvironmentFilterUniformBlock ) );

		auto ProgramBinding = RsCore::pImpl()->createProgramBinding( FilterProgram_, BindingDesc, (*getName()).c_str() );

		Frame->queueRenderNode( Sort,
			[ 
				ProgramBinding = ProgramBinding.get(),
				Face = Face,
				FilterUniform = FilterUniform_.get(),
				XGroups = Texture->getDesc().Width_ >> Level,
				YGroups = Texture->getDesc().Height_ >> Level,
				Roughness
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;

				Context->updateBuffer( FilterUniform, 0, sizeof( ScnEnvironmentFilterUniformBlock ), RsResourceUpdateFlags::NONE,
					[
						Face,
						FilterUniform,
						Roughness
					]
					( RsBuffer* Buffer, const RsBufferLock& Lock )
					{
						auto* OutBuffer = reinterpret_cast< ScnEnvironmentFilterUniformBlock* >( Lock.Buffer_ );
						OutBuffer->EnvironmentFilterCubeFace_ = BcU32( Face );
						OutBuffer->EnvironmentFilterRoughness_ = Roughness;
					} );

				Context->dispatchCompute( ProgramBinding, XGroups, YGroups, 1 );
			} );
	}
	else
	{
		BcAssertMsg( false, "Non-compute path unimplemented." );
	}

}

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
void ScnEnvironmentProbeProcessor::onAttachComponent( ScnComponent* Component )
{
	if( Component->isTypeOf< ScnEnvironmentProbeComponent >() )
	{
		auto* ProbeComponent = static_cast< ScnEnvironmentProbeComponent* >( Component );
		EnvironmentProbes_.push_back( ProbeComponent );
	}

	// TODO: More intelligent determination of if a probe needs to be regenerated.
	// Regenerate if entity has been attached.
	else if( Component->isTypeOf< ScnEntity >() )
	{
		ProbeUpdateQueue_.clear();
		for( auto ProbeComponent : EnvironmentProbes_ )
		{
			ProbeUpdateQueue_.push_back( ProbeComponent );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
void ScnEnvironmentProbeProcessor::onDetachComponent( ScnComponent* Component )
{
	if( Component->isTypeOf< ScnEnvironmentProbeComponent >() )
	{
		{
			auto It = std::find( ProbeUpdateQueue_.begin(), ProbeUpdateQueue_.end(), Component );
			if( It != ProbeUpdateQueue_.end() )
			{
				ProbeUpdateQueue_.erase( It );
			}
		}

		{
			auto It = std::find( EnvironmentProbes_.begin(), EnvironmentProbes_.end(), Component );
			if( It != EnvironmentProbes_.end() )
			{
				EnvironmentProbes_.erase( It );
			}
		}
	}

	// TODO: More intelligent determination of if a probe needs to be regenerated.
	// Regenerate if entity has been attached.
	else if( Component->isTypeOf< ScnEntity >() )
	{
		for( auto ProbeComponent : EnvironmentProbes_ )
		{
			ProbeUpdateQueue_.push_back( ProbeComponent );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// ScnEnvironmentProbeComponent
REFLECTION_DEFINE_DERIVED( ScnEnvironmentProbeComponent );

void ScnEnvironmentProbeComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "TemplateRenderer_", &ScnEnvironmentProbeComponent::TemplateRenderer_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnEnvironmentProbeComponent, Super >( Fields )
		.addAttribute( new ScnEnvironmentProbeProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnEnvironmentProbeComponent::ScnEnvironmentProbeComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ScnEnvironmentProbeComponent::~ScnEnvironmentProbeComponent()
{
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnEnvironmentProbeComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Renderer_ = ScnCore::pImpl()->spawnEntity(
		ScnEntitySpawnParams( "Renderer", TemplateRenderer_, MaMat4d(), Parent ) );
	
	// TODO: Renderer interface component?
	auto Renderer = Renderer_->getComponentByType< ScnDeferredRendererComponent >();
	BcAssert( Renderer->getWidth() > 0 && Renderer->getHeight() > 0 );

	// Create texture to render into.
	auto Levels = BcS32( 32 - BcCountLeadingZeros( BcU32( Renderer->getWidth() ) ) );

	// TODO: Non-compute path for mipmap generation.
	if( !RsCore::pImpl()->getContext( nullptr )->getFeatures().ComputeShaders_ )
	{
		Levels = 1;
	}

	// Don't want tail mips 1x1 & 2x2.
	Levels = std::max( 1, Levels - 2 );
	Texture_ = ScnTexture::NewCube( Renderer->getWidth(), Renderer->getHeight(), Levels, RsTextureFormat::R8G8B8A8, 
		RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET | RsResourceBindFlags::UNORDERED_ACCESS, (*getName()).c_str() );

	// Generate cubemaps.
	RsFrameBufferDesc Desc( 1 );
	for( size_t Idx = 0; Idx < CubemapFaceTargets_.size(); ++Idx )
	{
		Desc.setRenderTarget( 0, Texture_->getTexture(), 0, RsTextureFace( Idx ) );
		CubemapFaceTargets_[ Idx ] = RsCore::pImpl()->createFrameBuffer( Desc, (*getName()).c_str() );
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetach
void ScnEnvironmentProbeComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Texture_->markDestroy();
	Texture_ = nullptr;
}

