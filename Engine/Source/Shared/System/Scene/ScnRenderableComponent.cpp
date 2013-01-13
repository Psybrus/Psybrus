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

#include "System/Scene/ScnDebugRenderComponent.h"

#include "System/Scene/ScnLightingVisitor.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRenderableComponent );

BCREFLECTION_DERIVED_BEGIN( ScnSpatialComponent, ScnRenderableComponent )
	BCREFLECTION_MEMBER( BcU32,								RenderMask_,							bcRFF_DEFAULT ),
BCREFLECTION_DERIVED_END();

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnRenderableComponent::initialise()
{
	Super::initialise();

	setRenderMask( 1 );
	IsLit_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRenderableComponent::initialise( const Json::Value& Object )
{
	initialise();

	const Json::Value& RenderMaskValue = Object[ "rendermask" ];
	if( RenderMaskValue.type() != Json::nullValue )
	{
		setRenderMask( RenderMaskValue.asUInt() );
	}

	const Json::Value& IsLitValue = Object[ "islit" ];
	if( IsLitValue.type() == Json::booleanValue )
	{
		IsLit_ = IsLitValue.asBool();
	}
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnRenderableComponent::update( BcF32 Tick )
{
	//ScnDebugRenderComponent::pImpl()->drawAABB( getAABB(), RsColour::BLUE );

	Super::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// render
//virtual
void ScnRenderableComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	
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

//////////////////////////////////////////////////////////////////////////
// isLit
BcBool ScnRenderableComponent::isLit() const
{
	return IsLit_;
}