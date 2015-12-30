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
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"
#include "System/Renderer/RsCore.h"

#include "System/Scene/Rendering/ScnDebugRenderComponent.h"
#include "System/Scene/Rendering/ScnLightingVisitor.h"
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
	};
		
	ReRegisterClass< ScnRenderableComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnRenderableComponent::ScnRenderableComponent():
	RenderMask_( 1 ),
	IsLit_( BcFalse ),
	ViewRenderDataVersion_( 0 )
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
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// destroyViewRenderData
//virtual
void ScnRenderableComponent::destroyViewRenderData( class ScnViewComponent* View, class ScnViewRenderData* ViewRenderData )
{
}

//////////////////////////////////////////////////////////////////////////
// setViewRenderDataDirty
void ScnRenderableComponent::setViewRenderDataDirty()
{
	ViewRenderDataVersion_++;
}

//////////////////////////////////////////////////////////////////////////
// getViewRenderDataVersion
BcU32 ScnRenderableComponent::getViewRenderDataVersion() const
{
	return ViewRenderDataVersion_;
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
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnRenderableComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// isLit
bool ScnRenderableComponent::isLit() const
{
	return IsLit_;
}