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

#include "System/Scene/ScnRenderableComponent.h"
#include "System/Scene/ScnEntity.h"
#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderableComponent );

BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnRenderableComponent )
	BCREFLECTION_MEMBER( BcU32,								RenderMask_,							bcRFF_DEFAULT ),
BCREFLECTION_DERIVED_END();

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnRenderableComponent::initialise()
{
	Super::initialise();

	setRenderMask( BcErrorCode );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRenderableComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnRenderableComponent::update( BcF32 Tick )
{
	Super::update( Tick );

}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void ScnRenderableComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	// Do nothing.
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
