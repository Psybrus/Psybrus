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

#include "System/Scene/Rendering/ScnSpriteComponent.h"

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

void ScnSpriteComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &ScnSpriteComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "Material_", &ScnSpriteComponent::Material_, bcRFF_TRANSIENT ),
		new ReField( "Material_", &ScnSpriteComponent::MaterialName_ ),
		new ReField( "Position_", &ScnSpriteComponent::Position_ ),
		new ReField( "Size_", &ScnSpriteComponent::Size_ ),
		new ReField( "Colour_", &ScnSpriteComponent::Colour_ ),
		new ReField( "Index_", &ScnSpriteComponent::Index_ ),
		new ReField( "Layer_", &ScnSpriteComponent::Layer_ ),
		new ReField( "Center_", &ScnSpriteComponent::Center_ ),
		new ReField( "IsScreenSpace_", &ScnSpriteComponent::IsScreenSpace_ ),
	};
	
	ReRegisterClass< ScnSpriteComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2100 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSpriteComponent::initialise()
{
	Position_ = MaVec2d( 0.0f, 0.0f );
	Size_ = MaVec2d( 0.0f, 0.0f );
	Colour_ = RsColour::WHITE;
	Index_ = 0;
	Layer_ = 0;
	Center_ = BcFalse;
	IsScreenSpace_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSpriteComponent::initialise( const Json::Value& Object )
{
	ScnSpriteComponent::initialise();
	MaterialName_ = Object[ "materialcomponent" ].asCString();

	if( Object[ "size" ].type() != Json::nullValue )
	{
		Size_ = Object[ "size" ].asCString();
	}

	if( Object[ "colour" ].type() != Json::nullValue )
	{
		Colour_ = MaVec4d( Object[ "colour" ].asCString() );
	}

	if( Object[ "position" ].type() != Json::nullValue )
	{
		Position_ = MaVec2d( Object[ "position" ].asCString() );
	}

	if( Object[ "center" ].type() != Json::nullValue )
	{
		Center_ = Object[ "center" ].asBool();
	}

	if( Object[ "layer" ].type() != Json::nullValue )
	{
		Layer_ = Object[ "layer" ].asUInt();
	}

	if( Object[ "isscreenspace" ].type() != Json::nullValue )
	{
		IsScreenSpace_ = Object[ "isscreenspace" ].asBool();
	}
	
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnSpriteComponent::postUpdate( BcF32 Tick )
{
	Super::postUpdate( Tick );

	ScnEntityWeakRef Entity = getParentEntity();
	MaMat4d Matrix = Entity->getWorldMatrix();

	// Push matrix onto canvas.
	if( !IsScreenSpace_ )
	{
		Canvas_->pushMatrix( Matrix );
	}
	else
	{
		Matrix = Canvas_->popMatrix();
	}

	// Draw sprite at the correct transform position.
	Canvas_->setMaterialComponent( Material_ );
	if( Center_ )
	{
		Canvas_->drawSpriteCentered( Position_, Size_, Index_, Colour_, Layer_ );
	}
	else
	{
		Canvas_->drawSprite( Position_, Size_, Index_, Colour_, Layer_ );
	}

	// Pop.
	if( !IsScreenSpace_ )
	{
		Canvas_->popMatrix();
	}
	else
	{
		Canvas_->pushMatrix( Matrix );
		Canvas_->setMatrix( Matrix );
	}
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

	// Find a canvas to use for rendering (someone in ours, or our parent's hierarchy).
	Material_ = Parent->getComponentAnyParentByType< ScnMaterialComponent >( MaterialName_ );
	BcAssertMsg( Material_.isValid(), "Sprite component needs to be attached to an entity with a material component in any parent!" );
}

//////////////////////////////////////////////////////////////////////////
// setPitch
//virtual
void ScnSpriteComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Canvas_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getPosition
const MaVec2d& ScnSpriteComponent::getPosition() const
{
	return Position_;
}

//////////////////////////////////////////////////////////////////////////
// getPosition
void ScnSpriteComponent::setPosition( const MaVec2d& Position )
{
	Position_ = Position;
}

//////////////////////////////////////////////////////////////////////////
// getSize
const MaVec2d& ScnSpriteComponent::getSize() const
{
	return Size_;
}

//////////////////////////////////////////////////////////////////////////
// setSize
void ScnSpriteComponent::setSize( const MaVec2d& Size )
{
	Size_ = Size;
}

//////////////////////////////////////////////////////////////////////////
// getColour
const RsColour& ScnSpriteComponent::getColour() const
{
	return Colour_;
}

//////////////////////////////////////////////////////////////////////////
// setColour
void ScnSpriteComponent::setColour( const RsColour& Colour )
{
	Colour_ = Colour;
}
