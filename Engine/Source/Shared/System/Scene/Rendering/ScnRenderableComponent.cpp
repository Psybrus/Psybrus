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

#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"

#include "System/Os/OsCore.h"
#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnRenderableComponent );

void ScnRenderableComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "RenderMask_", &ScnRenderableComponent::RenderMask_, bcRFF_IMPORTER ),
		new ReField( "IsLit_", &ScnRenderableComponent::IsLit_, bcRFF_IMPORTER ),
		new ReField( "RenderPermutations_", &ScnRenderableComponent::RenderPermutations_, bcRFF_IMPORTER | bcRFF_FLAGS ),
		new ReField( "Passes_", &ScnRenderableComponent::Passes_, bcRFF_IMPORTER | bcRFF_FLAGS ),
	};
	
	ReRegisterClass< ScnRenderableComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderableComponent::ScnRenderableComponent():
	RenderMask_( 1 ),
	IsLit_( BcFalse ),
	RenderPermutations_( ScnShaderPermutationFlags::RENDER_FORWARD | ScnShaderPermutationFlags::RENDER_DEFERRED | ScnShaderPermutationFlags::RENDER_FORWARD_PLUS ),
	Passes_( RsRenderSortPassFlags::DEPTH | RsRenderSortPassFlags::OPAQUE | RsRenderSortPassFlags::SHADOW )
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
	return new ScnViewRenderData();
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
		ScnViewRenderData* ViewRenderData = nullptr;
		auto SortPassType = getSortPassType( ViewComponent );
		if( SortPassType != RsRenderSortPassType::INVALID )
		{
			ViewRenderData = createViewRenderData( ViewComponent );
			if( ViewRenderData )
			{
				ViewRenderData->setSortPassType( SortPassType );
			}
		}	
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

	// HACK: Subscribe for resize event to reset bindings incase anything needs to be rebound.
	OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
		[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
		{
			resetViewRenderData( nullptr );
			return evtRET_PASS;
		} );

	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnRenderableComponent::onDetach( ScnEntityWeakRef Parent )
{
	OsCore::pImpl()->unsubscribeAll( this );
	ScnCore::pImpl()->removeCallback( this );

	// Reset all view render data.
	resetViewRenderData( nullptr );

	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getSortPassType
RsRenderSortPassType ScnRenderableComponent::getSortPassType( class ScnViewComponent* View ) const
{
	RsRenderSortPassType RetVal = RsRenderSortPassType::INVALID;
	if( BcContainsAnyFlags( View->getRenderPermutation(), RenderPermutations_ ) )
	{
		const auto ViewPasses = View->getPasses();
		const auto CombinedFlags = ViewPasses & Passes_;
		const auto LeadingZeros = BcCountLeadingZeros( static_cast< BcU32 >( CombinedFlags ) );
		if( LeadingZeros < 32 )
		{
			RetVal = static_cast< RsRenderSortPassType >( 31 - LeadingZeros );
		}
	}
	return RetVal;
}
