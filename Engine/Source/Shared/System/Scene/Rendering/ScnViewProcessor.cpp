#include "System/Scene/Rendering/ScnViewProcessor.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/ScnCore.h"

#include "Base/BcProfiler.h"

#include "System/Debug/DsImGui.h"

#include "System/Scene/Rendering/ScnRenderingVisitor.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnViewProcessor );

void ScnViewProcessor::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "RenderableComponents_", &ScnViewProcessor::RenderableComponents_, bcRFF_TRANSIENT ),
		new ReField( "GatheredRenderableComponents_", &ScnViewProcessor::GatheredRenderableComponents_, bcRFF_TRANSIENT )
	};

	ReRegisterClass< ScnViewProcessor, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnViewProcessor::ScnViewProcessor():
	ScnComponentProcessor( 
		{
			ScnComponentProcessFuncEntry(
				"Render Views",
				ScnComponentPriority::VIEW_RENDER,
				std::bind( &ScnViewProcessor::renderViews, this, std::placeholders::_1 ) ),
		} )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnViewProcessor::~ScnViewProcessor()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnViewProcessor::initialise()
{
	ScnCore::pImpl()->addCallback( this );
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void ScnViewProcessor::shutdown()
{
	ScnCore::pImpl()->removeCallback( this );
}

//////////////////////////////////////////////////////////////////////////
// renderViews
void ScnViewProcessor::renderViews( const ScnComponentList& InComponents )
{
	PSY_PROFILER_SECTION( RenderRoot, std::string( "ScnViewProcessor::renderViews" ) );

	// TODO: Have things register with RsCore to be rendered maybe?

	// Get context.
	RsContext* pContext = RsCore::pImpl()->getContext( nullptr );

	// Early out if size is 0 on any axis.
	if( pContext->getWidth() == 0 || pContext->getHeight() == 0 )
	{
		return;
	}

	// Allocate a frame to render using default context.
	RsFrame* Frame = RsCore::pImpl()->allocateFrame( pContext );

	RsRenderSort Sort( 0 );

	// Old path with frustum culling.
#if 0
	// Iterate over all view components.
	for( auto InComponent : InComponents )
	{
		BcAssert( InComponent->isTypeOf< ScnViewComponent >() );
		auto* Component = static_cast< ScnViewComponent* >( InComponent.get() );

		Component->bind( Frame, Sort );

		ScnRenderingVisitor Visitor( Component, Frame, Sort );

		// Increment viewport.
		Sort.Viewport_++;
	}
#else
	// New path, naive no culling (yet).

	// Iterate over all view components.
	for( auto InComponent : InComponents )
	{
		BcAssert( InComponent->isTypeOf< ScnViewComponent >() );
		auto* ViewComponent = static_cast< ScnViewComponent* >( InComponent.get() );

		// Bind view.
		ViewComponent->bind( Frame, Sort );

		// Gather renderable components.
		GatheredRenderableComponents_.clear();
		for( auto RenderableComponent : RenderableComponents_ )
		{
			if( ViewComponent->getRenderMask() & RenderableComponent->getRenderMask() )
			{
				if( RenderableComponent->isReady() )
				{
					GatheredRenderableComponents_.push_back( RenderableComponent );
				}
			}
		}
		
		// Render.
		for( auto RenderableComponent : GatheredRenderableComponents_ )
		{
			RenderableComponent->render( ViewComponent, Frame, Sort );
		}

		// Increment viewport.
		Sort.Viewport_++;
	}


#endif

	// TODO: Move completely to DsCore.
	//       Probably depends on registration with RsCore.
	// Render ImGui.
	ImGui::Psybrus::Render( pContext, Frame );

	// Queue frame for render.
	RsCore::pImpl()->queueFrame( Frame );
}

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
//virtual
void ScnViewProcessor::onAttachComponent( class ScnComponent* Component )
{
	BcAssert( Component->isReady() );
	if( Component->isTypeOf< ScnRenderableComponent >() )
	{
		RenderableComponents_.insert( static_cast< ScnRenderableComponent* >( Component ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
//virtual
void ScnViewProcessor::onDetachComponent( class ScnComponent* Component )
{
	BcAssert( Component->isReady() );
	if( Component->isTypeOf< ScnRenderableComponent >() )
	{
		auto OldCount = RenderableComponents_.size();
		RenderableComponents_.erase( 
			std::find( RenderableComponents_.begin(), RenderableComponents_.end(), Component ) );
		BcAssert( OldCount != RenderableComponents_.size() );
	}
}
