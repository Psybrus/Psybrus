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
// Processor

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderableProcessor::ScnRenderableProcessor()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnRenderableProcessor::~ScnRenderableProcessor()
{
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnRenderableProcessor::initialise()
{
	ScnViewProcessor::pImpl()->registerRenderInterface( ScnRenderableComponent::StaticGetClass(), this );
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void ScnRenderableProcessor::shutdown()
{
	ScnViewProcessor::pImpl()->deregisterRenderInterface( ScnRenderableComponent::StaticGetClass(), this );
}

//////////////////////////////////////////////////////////////////////////
// createViewRenderData
class ScnViewRenderData* ScnRenderableProcessor::createViewRenderData( class ScnComponent* Component, class ScnViewComponent* View )
{
	BcAssert( Component->isTypeOf< ScnRenderableComponent >() );
	return static_cast< ScnRenderableComponent* >( Component )->createViewRenderData( View );
}

//////////////////////////////////////////////////////////////////////////
// destroyViewRenderData
void ScnRenderableProcessor::destroyViewRenderData( class ScnComponent* Component, class ScnViewComponent* View, ScnViewRenderData* ViewRenderData )
{
	BcAssert( Component->isTypeOf< ScnRenderableComponent >() );
	static_cast< ScnRenderableComponent* >( Component )->destroyViewRenderData( View, ViewRenderData );
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnRenderableProcessor::render( const ScnViewComponentRenderData* ComponentRenderDatas, BcU32 NoofComponents, class ScnRenderContext& RenderContext )
{
	for( BcU32 Idx = 0; Idx< NoofComponents; ++Idx )
	{
		const ScnViewComponentRenderData& ComponentRenderData( ComponentRenderDatas[ Idx ] );
		ScnComponent* Component = ComponentRenderData.Component_;
		BcAssert( Component->isTypeOf< ScnRenderableComponent >() );
		auto RenderableComponent = static_cast< ScnRenderableComponent* >( Component );
		auto* ViewRenderData = ComponentRenderData.ViewRenderData_;
		if( ViewRenderData )
		{
			RenderContext.ViewRenderData_ = ViewRenderData;
			RenderContext.Sort_.Pass_ = BcU64( ViewRenderData->getSortPassType() );
			RenderableComponent->render( RenderContext );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getAABB
void ScnRenderableProcessor::getAABB( MaAABB* OutAABBs, class ScnComponent** Components, BcU32 NoofComponents )
{
	for( BcU32 Idx = 0; Idx < NoofComponents; ++Idx )
	{
		ScnComponent* Component = Components[ Idx ];
		BcAssert( Component->isTypeOf< ScnRenderableComponent >() );
		OutAABBs[ Idx ] = static_cast< ScnRenderableComponent* >( Component )->getAABB();
	}
}

//////////////////////////////////////////////////////////////////////////
// getRenderMask
void ScnRenderableProcessor::getRenderMask( BcU32* OutRenderMasks, class ScnComponent** Components, BcU32 NoofComponents )
{
	for( BcU32 Idx = 0; Idx < NoofComponents; ++Idx )
	{
		ScnComponent* Component = Components[ Idx ];
		BcAssert( Component->isTypeOf< ScnRenderableComponent >() );
		OutRenderMasks[ Idx ] = static_cast< ScnRenderableComponent* >( Component )->getRenderMask();
	}
}

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
		.addAttribute( new ScnRenderableProcessor() );
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
// createViewRenderData
//virtual
ScnViewRenderData* ScnRenderableComponent::createViewRenderData( class ScnViewComponent* View )
{
	auto SortPassType = View->getSortPassType( Passes_, RenderPermutations_ );
	if( SortPassType != RsRenderSortPassType::INVALID )
	{
		auto RetVal = new ScnViewRenderData();
		RetVal->setSortPassType( SortPassType );
		return RetVal;
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// destroyViewRenderData
//virtual
void ScnRenderableComponent::destroyViewRenderData( class ScnViewComponent* View, ScnViewRenderData* ViewRenderData )
{
	delete ViewRenderData;
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void ScnRenderableComponent::render( ScnRenderContext& RenderContext )
{
	
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB ScnRenderableComponent::getAABB() const
{
	return MaAABB();
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
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnRenderableComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

