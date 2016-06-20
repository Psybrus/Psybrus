#include "System/Scene/Rendering/ScnEnvironmentProbeComponent.h"
#include "System/Scene/Rendering/ScnDeferredRendererComponent.h"
#include "System/Scene/Rendering/ScnTexture.h"

#include "System/Scene/ScnCore.h"

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
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void ScnEnvironmentProbeProcessor::shutdown()
{
	ScnCore::pImpl()->removeCallback( this );
}

//////////////////////////////////////////////////////////////////////////
// updateProbes
void ScnEnvironmentProbeProcessor::updateProbes( const ScnComponentList& Components )
{
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
			MaVec3d Position = ProbeComponent->getParentEntity()->getWorldPosition();
			MaMat4d Transform;

			// POSITIVE_X
			Transform.rotation( MaVec3d( 0.0f, -BcPIDIV2, 0.0f ) );
			Transform.translation( Position );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 0 ].get(), Sort );
			Sort.RenderTarget_++;

			// NEGATIVE_X
			Transform.rotation( MaVec3d( 0.0f, BcPIDIV2, 0.0f ) );
			Transform.translation( Position );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 1 ].get(), Sort );
			Sort.RenderTarget_++;

			// POSITIVE_Y
			Transform.rotation( MaVec3d( BcPIDIV2, 0.0f, 0.0f ) );
			Transform.translation( Position );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 2 ].get(), Sort );
			Sort.RenderTarget_++;

			// NEGATIVE_Y
			Transform.rotation( MaVec3d( -BcPIDIV2, 0.0f, 0.0f ) );
			Transform.translation( Position );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 3 ].get(), Sort );
			Sort.RenderTarget_++;

			// POSITIVE_Z
			Transform.rotation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
			Transform.translation( Position );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 4 ].get(), Sort );
			Sort.RenderTarget_++;

			// NEGATIVE_Z
			Transform.rotation( MaVec3d( 0.0f, BcPI, 0.0f ) );
			Transform.translation( Position );
			Renderer->getParentEntity()->setWorldMatrix( Transform );
			Renderer->render( Frame, ProbeComponent->CubemapFaceTargets_[ 5 ].get(), Sort );
			Sort.RenderTarget_++;

			ProbeComponent->GeneratedWorldPosition_ = Position;
		}

		// Queue frame for render.
		RsCore::pImpl()->queueFrame( Frame );
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
	Texture_ = ScnTexture::NewCube( Renderer->getWidth(), Renderer->getHeight(), 1, RsTextureFormat::R8G8B8A8, true, false, (*getName()).c_str() );

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

