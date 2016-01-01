/**************************************************************************
*
* File:		ScnRenderableComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/12/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnRenderableComponent.h"
#include "System/Scene/ScnCore.h"
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"
#include "System/Renderer/RsCore.h"

#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnRenderableComponent );

void ScnRenderableComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "RenderMask_", &ScnRenderableComponent::RenderMask_, bcRFF_IMPORTER ),
		new ReField( "IsLit_", &ScnRenderableComponent::IsLit_, bcRFF_IMPORTER ),
		new ReField( "IsTransparent_", &ScnRenderableComponent::IsTransparent_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnRenderableComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderableComponent::ScnRenderableComponent():
	RenderMask_( 1 ),
	IsLit_( BcFalse ),
	IsTransparent_( BcFalse )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnRenderableComponent::~ScnRenderableComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttachComponent
void ScnRenderableComponent::onAttachComponent( class ScnComponent* Component )
{

}

//////////////////////////////////////////////////////////////////////////
// onDetachComponent
void ScnRenderableComponent::onDetachComponent( class ScnComponent* Component )
{
	if( Component->isTypeOf< ScnViewComponent >() )
	{
		auto* ViewComponent = static_cast< ScnViewComponent* >( Component );
		resetViewRenderData( ViewComponent );
	}
}

//////////////////////////////////////////////////////////////////////////
// createViewRenderData
//virtual
ScnViewRenderData* ScnRenderableComponent::createViewRenderData( class ScnViewComponent* View )
{
	return shouldRenderInView( View ) ? new ScnViewRenderData() : nullptr;
}

//////////////////////////////////////////////////////////////////////////
// destroyViewRenderData
//virtual
void ScnRenderableComponent::destroyViewRenderData( ScnViewRenderData* ViewRenderData )
{
	delete ViewRenderData;
}

//////////////////////////////////////////////////////////////////////////
// getViewRenderData
class ScnViewRenderData* ScnRenderableComponent::getViewRenderData( class ScnViewComponent* ViewComponent )
{
	auto FoundIt = ViewRenderData_.find( ViewComponent );
	if( FoundIt == ViewRenderData_.end() )
	{
		auto ViewRenderData = createViewRenderData( ViewComponent );
		ViewRenderData_.insert( std::make_pair( ViewComponent, ViewRenderData ) );
		return ViewRenderData;
	}
	return FoundIt->second;
}

//////////////////////////////////////////////////////////////////////////
// resetViewRenderData
void ScnRenderableComponent::resetViewRenderData( ScnViewComponent* ViewComponent )
{
	for( auto ViewRenderDataIt = ViewRenderData_.begin(); ViewRenderDataIt != ViewRenderData_.end(); )
	{
		if( ViewComponent == nullptr || ViewRenderDataIt->first == ViewComponent )
		{
			destroyViewRenderData( ViewRenderDataIt->second );
			ViewRenderDataIt = ViewRenderData_.erase( ViewRenderDataIt );
		}
		else
		{
			++ViewRenderDataIt;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void ScnRenderableComponent::render( ScnRenderContext & RenderContext )
{
	
}

//////////////////////////////////////////////////////////////////////////
// setRenderMask
void ScnRenderableComponent::setRenderMask( BcU32 RenderMask )
{
	RenderMask_ = RenderMask;
}

//////////////////////////////////////////////////////////////////////////
// getRenderMask
const BcU32 ScnRenderableComponent::getRenderMask() const
{
	return RenderMask_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnRenderableComponent::onAttach( ScnEntityWeakRef Parent )
{
	// TODO: Add type of component to callback.
	ScnCore::pImpl()->addCallback( this );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnRenderableComponent::onDetach( ScnEntityWeakRef Parent )
{
	ScnCore::pImpl()->removeCallback( this );

	// Reset all view render data.
	resetViewRenderData( nullptr );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// shouldRenderInView
bool ScnRenderableComponent::shouldRenderInView( class ScnViewComponent* View ) const
{
	bool ShouldRender = false;

	// If opaque view and not transparent - render.
	if( ( View->getPassPermutations() & ScnShaderPermutationFlags::PASS_OPAQUE ) != ScnShaderPermutationFlags::NONE )
	{
		if( !IsTransparent_ )
		{
			ShouldRender = true;
		}
	}

	// If transparent view and transparent - render.
	if( ( View->getPassPermutations() & ScnShaderPermutationFlags::PASS_TRANSPARENT ) != ScnShaderPermutationFlags::NONE )
	{
		if( IsTransparent_ )
		{
			ShouldRender = true;
		}
	}

	return ShouldRender;
}
