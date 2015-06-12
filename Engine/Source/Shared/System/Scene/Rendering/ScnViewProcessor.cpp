#include "System/Scene/Rendering/ScnViewProcessor.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "Base/BcProfiler.h"

#include "System/Debug/DsImGui.h"

#include "System/Scene/Rendering/ScnRenderingVisitor.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnViewProcessor );

void ScnViewProcessor::StaticRegisterClass()
{
#if 0
	ReField* Fields[] = 
	{
	};
#endif

	ReRegisterClass< ScnViewProcessor, Super >();
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
// renderViews
void ScnViewProcessor::renderViews( const ScnComponentList& InComponents )
{
	PSY_PROFILER_SECTION( RenderRoot, std::string( "ScnViewProcessor::renderViews" ) );

	// TODO: Have things register with RsCore to be rendered maybe?

	// Get context.
	RsContext* pContext = RsCore::pImpl()->getContext( nullptr );

	// Allocate a frame to render using default context.
	RsFrame* pFrame = RsCore::pImpl()->allocateFrame( pContext );

	RsRenderSort Sort( 0 );

	// Iterate over all view components.
	for( auto InComponent : InComponents )
	{
		BcAssert( InComponent->isTypeOf< ScnViewComponent >() );
		auto* Component = static_cast< ScnViewComponent* >( InComponent.get() );

		Component->bind( pFrame, Sort );

		ScnRenderingVisitor Visitor( Component, pFrame, Sort );

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

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
//virtual
void ScnViewProcessor::onAttachComponent( class ScnComponent* Component )
{

}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
//virtual
void ScnViewProcessor::onDetachComponent( class ScnComponent* Component )
{

}
