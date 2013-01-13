/**************************************************************************
*
* File:		ScnSpriteComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	23/04/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/ScnSpriteComponent.h"

#include "System/Scene/ScnEntity.h"

#include "System/Content/CsCore.h"
#include "System/Sound/SsCore.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSpriteComponent );

BCREFLECTION_EMPTY_REGISTER( ScnSpriteComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnSpriteComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSpriteComponent::initialise()
{
	Position_ = BcVec2d( 0.0f, 0.0f );
	Size_ = BcVec2d( 0.0f, 0.0f );
	Colour_ = RsColour::WHITE;
	Index_ = 0;
	Layer_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSpriteComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	ScnSpriteComponent::initialise();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnSpriteComponent::postUpdate( BcF32 Tick )
{
	Super::postUpdate( Tick );

	ScnEntityWeakRef Entity = getParentEntity();
	const BcMat4d& Matrix = Entity->getWorldMatrix();

	// Push matrix onto canvas.
	Canvas_->pushMatrix( Matrix );

	// Draw sprite at the correct transform position.
	Canvas_->setMaterialComponent( Material_ );
	Canvas_->drawSprite( Position_, Size_, Index_, Colour_, Layer_ );

	// Pop.
	Canvas_->popMatrix();
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSpriteComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	// Find a canvas to use for rendering (someone in ours, or our parent's hierarchy).
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );
	BcAssertMsg( Canvas_.isValid(), "Sprite component needs to be attached to an entity with a canvas component in any parent!" );
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSpriteComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Canvas_ = NULL;
}
