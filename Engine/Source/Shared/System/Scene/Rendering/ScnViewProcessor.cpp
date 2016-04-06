#include "System/Scene/Rendering/ScnViewProcessor.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/Rendering/ScnRenderingVisitor.h"

#include "System/Debug/DsImGui.h"

#include "System/Os/OsCore.h"
#include "System/Os/OsClient.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnViewProcessor::ScnViewProcessor():
	ScnComponentProcessor( {
		ScnComponentProcessFuncEntry(
			"Render Views",
			ScnComponentPriority::VIEW_RENDER,
			std::bind( &ScnViewProcessor::renderViews, this, std::placeholders::_1 ) ) } )
{
}

//////////////////////////////////////////////////////////////////////////
// renderViews
void ScnViewProcessor::renderViews( const ScnComponentList& Components )
{
	PSY_PROFILER_SECTION( RenderRoot, std::string( "ScnViewProcessor::renderViews" ) );

	// TODO: Have things register with RsCore to be rendered maybe?

	// Get context.
	RsContext* pContext = RsCore::pImpl()->getContext( nullptr );

	// Check for native window handle, and a size greater than 0.
	// Don't want to render at all if none of those conditions are met.
	auto Client = pContext->getClient();
	if( Client->getWindowHandle() != 0 &&
		Client->getWidth() > 0 &&
		Client->getHeight() > 0 )
	{
		// Allocate a frame to render using default context.
		RsFrame* pFrame = RsCore::pImpl()->allocateFrame( pContext );

		RsRenderSort Sort( 0 );
		
		// Check viewport count.
		if( Components.size() > RS_SORT_VIEWPORT_MAX )
		{
			PSY_LOG( "WARNING: More ScnViewComponents than there are availible slots. Reduce number of ScnViewComponents in scene or expect strange results." );
		}

		// If we have no views, clear to black.
		if( Components.size() == 0 )
		{
			pFrame->queueRenderNode( Sort,
				[]( RsContext* Context )
				{
					Context->clear( nullptr, RsColour::BLACK, BcTrue, BcTrue, BcTrue );
				} );
		}

		// Iterate over all view components.
		for( auto Component : Components )
		{
			BcAssert( Component->isTypeOf< ScnViewComponent >() );
			auto* ViewComponent = static_cast< ScnViewComponent* >( Component.get() );
	
			ScnRenderContext RenderContext( ViewComponent, pFrame, Sort );

			ViewComponent->bind( pFrame, Sort );

			ScnRenderingVisitor Visitor( RenderContext );

			Visitor.render();

			// Increment viewport.
			Sort.Viewport_++;
		}

		// TODO: Move completely to DsCore.
		//       Probably depends on registration with RsCore.
		// Render ImGui.
		ImGui::Psybrus::Render( pContext, pFrame );

		// Queue frame for render.
		RsCore::pImpl()->queueFrame( pFrame );
	}
}
