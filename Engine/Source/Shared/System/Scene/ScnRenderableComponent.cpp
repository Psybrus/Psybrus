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

#include "System/Scene/ScnLightManagerComponent.h"

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

	setRenderMask( 1 );
	IsLit_ = BcFalse;
	pSpatialTreeNode_ = NULL;
	LightManager_ = NULL;
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
void ScnRenderableComponent::postUpdate( BcF32 Tick )
{
	Super::update( Tick );

	// Reinsert node if we need to.
	pSpatialTreeNode_->reinsertComponent( this );
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

	// Grab the light manager.
	LightManager_ = getParentEntity()->getComponentAnyParentByType< ScnLightManagerComponent >();
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnRenderableComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	LightManager_ = NULL;
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
// setLightingMaterialParams
void ScnRenderableComponent::setLightingMaterialParams( class ScnMaterialComponent* MaterialComponent )
{
	if( IsLit_ && LightManager_ != NULL )
	{
		LightManager_->setMaterialParameters( MaterialComponent );
	}
}

//////////////////////////////////////////////////////////////////////////
// setSpatialTreeNode
void ScnRenderableComponent::setSpatialTreeNode( ScnSpatialTreeNode* pNode )
{
	pSpatialTreeNode_ = pNode;
}

//////////////////////////////////////////////////////////////////////////
// getSpatialTreeNode
ScnSpatialTreeNode* ScnRenderableComponent::getSpatialTreeNode()
{
	return pSpatialTreeNode_;
}


//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
BcAABB ScnRenderableComponent::getAABB() const
{
	BcAssertMsg( BcFalse, "ScnRenderableComponent: Not implemented a getAABB!" );
	return BcAABB();
}

//////////////////////////////////////////////////////////////////////////
// isLit
BcBool ScnRenderableComponent::isLit() const
{
	return IsLit_;
}