#include "System/Scene/Rendering/ScnViewProcessor.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/ScnCore.h"

#include "Base/BcProfiler.h"

#include "System/Debug/DsImGui.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnViewProcessor );

void ScnViewProcessor::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "RenderableComponents_", &ScnViewProcessor::RenderableComponents_, bcRFF_TRANSIENT ),
		new ReField( "ViewComponents_", &ScnViewProcessor::ViewComponents_, bcRFF_TRANSIENT ),
		new ReField( "ProcessFuncEntries_", &ScnViewProcessor::ProcessFuncEntries_, bcRFF_TRANSIENT ),
		new ReField( "GatheredComponents_", &ScnViewProcessor::GatheredComponents_, bcRFF_TRANSIENT )
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

	// HACK: Register all renderable stuff:
	registerProcessFunc(
		ScnRenderableComponent::StaticGetClass(), 
		ScnViewProcessFuncEntry::Render< ScnRenderableComponent >( "Renderable" ) );
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void ScnViewProcessor::shutdown()
{
	ScnCore::pImpl()->removeCallback( this );
}

//////////////////////////////////////////////////////////////////////////
// registerProcessFunc
void ScnViewProcessor::registerProcessFunc( const ReClass* Class, ScnViewProcessFuncEntry ProcessFunc )
{
	BcAssert( std::find_if( ProcessFuncEntries_.begin(), ProcessFuncEntries_.end(),
		[ Class ]( const ScnViewProcessFuncEntry& Entry )
		{
			return Entry.Class_ == Class;

		} ) == ProcessFuncEntries_.end() );

	ProcessFunc.Class_ = Class;
	ProcessFuncEntries_.push_back( ProcessFunc );
}

//////////////////////////////////////////////////////////////////////////
// deregisterProcessFunc
void ScnViewProcessor::deregisterProcessFunc( const ReClass* Class )
{
	ProcessFuncEntries_.erase( std::find_if( ProcessFuncEntries_.begin(), ProcessFuncEntries_.end(),
		[ Class ]( const ScnViewProcessFuncEntry& Entry )
		{
			return Entry.Class_ == Class;

		} ) );
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

	// Iterate over all view components.
	for( auto InComponent : InComponents )
	{
		BcAssert( InComponent->isTypeOf< ScnViewComponent >() );
		auto* ViewComponent = static_cast< ScnViewComponent* >( InComponent.get() );

		// Bind view.
		ViewComponent->preDraw( Frame, Sort );

		// Gather renderable components.
		GatheredComponents_.clear();
		ScnCore::pImpl()->gather( ViewComponent->getFrustum(), GatheredComponents_ );
		
#if 0
		// Render.
		for( auto Component : GatheredComponents_ )
		{
			ScnRenderableComponentRef RenderableComponent( Component );
			if( RenderableComponent )
			{
				if( RenderableComponent->getRenderMask() & ViewComponent->getRenderMask() )
				{
					RenderableComponent->render( ViewComponent, Frame, Sort );
				}
			}
		}
#else
		// Render using batch processor.
		ScnComponentList FilteredComponents;
		for( auto& ViewProcessFuncEntry : ProcessFuncEntries_ )
		{
			// Filter.
			// TODO: Gather into seperate lists.
			// TODO: Render mask should be done in processor?
			for( auto Component : GatheredComponents_ )
			{
				if( Component->isTypeOf( ViewProcessFuncEntry.Class_ ) )
				{
					ScnRenderableComponentRef RenderableComponent( Component );
					if( RenderableComponent )
					{
						if( RenderableComponent->getRenderMask() & ViewComponent->getRenderMask() )
						{
							FilteredComponents.push_back( Component );
						}
					}
				}
			}

			// Draw.
			ViewProcessFuncEntry.Func_( FilteredComponents, ViewComponent, Frame, Sort );
		}

#endif
		// Increment viewport.
		Sort.Viewport_++;
	}

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
	else if( Component->isTypeOf< ScnViewComponent >() )
	{
		ViewComponents_.insert( static_cast< ScnViewComponent* >( Component ) );
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
		RenderableComponents_.erase( 
			std::find( RenderableComponents_.begin(), RenderableComponents_.end(), Component ) );
	}
	else if( Component->isTypeOf< ScnViewComponent >() )
	{
		ViewComponents_.erase( 
			std::find( ViewComponents_.begin(), ViewComponents_.end(), Component ) );
	}
}
