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
#include "Base/BcMath.h"
#ifdef PSY_IMPORT_PIPELINE
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnSpriteComponent );
REFLECTION_DEFINE_BASIC( ScnSpriteComponent::Animation );

void ScnSpriteComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "MaterialName_", &ScnSpriteComponent::MaterialName_, bcRFF_IMPORTER ),
		new ReField( "Position_", &ScnSpriteComponent::Position_ , bcRFF_IMPORTER | bcRFF_DEBUG_EDIT ),
		new ReField( "Size_", &ScnSpriteComponent::Size_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT  ),
		new ReField( "Colour_", &ScnSpriteComponent::Colour_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT ),
		new ReField( "Index_", &ScnSpriteComponent::Index_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT ),
		new ReField( "Layer_", &ScnSpriteComponent::Layer_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT  ),
		new ReField( "Center_", &ScnSpriteComponent::Center_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT  ),
		new ReField( "IsScreenSpace_", &ScnSpriteComponent::IsScreenSpace_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT  ),
		new ReField( "AnimationTimer_", &ScnSpriteComponent::AnimationTimer_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT ),
		new ReField( "AnimationRate_", &ScnSpriteComponent::AnimationRate_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT ),
		new ReField( "Animation_", &ScnSpriteComponent::Animation_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT ),
		new ReField( "Animations_", &ScnSpriteComponent::Animations_, bcRFF_IMPORTER | bcRFF_DEBUG_EDIT ),

		new ReField( "Canvas_", &ScnSpriteComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "Material_", &ScnSpriteComponent::Material_, bcRFF_TRANSIENT ),
		new ReField( "CurrKey_", &ScnSpriteComponent::CurrKey_, bcRFF_DEBUG_EDIT ),
	};
	
	ReRegisterClass< ScnSpriteComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( -2100 ) );
}

void ScnSpriteComponent::Animation::StaticRegisterClass()
{
	ReField* Fields[] =
	{
		new ReField( "Next_", &ScnSpriteComponent::Animation::Next_ ),
		new ReField( "Keys_", &ScnSpriteComponent::Animation::Keys_ ),
	};

	ReRegisterClass< ScnSpriteComponent::Animation >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSpriteComponent::ScnSpriteComponent()
{
	Position_ = MaVec2d( 0.0f, 0.0f );
	Size_ = MaVec2d( 0.0f, 0.0f );
	Colour_ = RsColour::WHITE;
	Index_ = 0;
	Layer_ = 0;
	Center_ = BcFalse;
	IsScreenSpace_ = BcFalse;
	Rotation_ = 0.0f;

	CurrKey_ = 0;
	AnimationTimer_ = 0.0f;
	AnimationRate_ = 12.0f;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSpriteComponent::~ScnSpriteComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnSpriteComponent::postUpdate( BcF32 Tick )
{
	Super::postUpdate( Tick );

	if( !Canvas_.isValid() )
	{
		return;
	}

	ScnEntityWeakRef Entity = getParentEntity();
	MaMat4d Matrix = Entity->getWorldMatrix();

	if( !Animation_.empty() && Animations_.find( Animation_ ) != Animations_.end() )
	{
		const auto& Animation = Animations_[ Animation_ ];

		if( CurrKey_ < Animation.Keys_.size() )
		{
			Index_ = Animation.Keys_[ CurrKey_ ];
		}
	
		AnimationTimer_ += Tick;
		if( AnimationTimer_ > ( 1.0f / AnimationRate_ ) )
		{
			AnimationTimer_ -= ( 1.0f / AnimationRate_ );
			++CurrKey_;

			if( CurrKey_ >= (BcU32)Animation.Keys_.size() )
			{
				setAnimation( Animation.Next_ );
			}
		}
	}

	if( Index_ == BcErrorCode )
	{
		return;
	}

	// Push matrix onto canvas.
	if( !IsScreenSpace_ )
	{
		Canvas_->pushMatrix( Matrix );

		if( BcAbs( Rotation_ ) > 0.01f )
		{
			MaMat4d Rotation;
			Rotation.rotation( MaVec3d( 0.0f, 0.0f, Rotation_ ) );
			Canvas_->pushMatrix( Rotation );
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

		if( BcAbs( Rotation_ ) > 0.01f )
		{
			Canvas_->popMatrix();
		}

		Canvas_->popMatrix();
	}
	else
	{
		Matrix = Canvas_->popMatrix();

		MaMat4d Rotation;
		Rotation.rotation( MaVec3d( 0.0f, 0.0f, Rotation_ ) );
		MaMat4d Translation;
		Translation.translation( MaVec3d( Position_.x(), Position_.y(), 0.0f ) );
		Canvas_->pushMatrix( Translation );
		Canvas_->pushMatrix( Rotation );

		// Draw sprite at the correct transform position.
		Canvas_->setMaterialComponent( Material_ );
		if( Center_ )
		{
			Canvas_->drawSpriteCentered( MaVec2d(), Size_, Index_, Colour_, Layer_ );
		}
		else
		{
			Canvas_->drawSprite( MaVec2d(), Size_, Index_, Colour_, Layer_ );
		}

		Canvas_->popMatrix();
		Canvas_->popMatrix();

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

//////////////////////////////////////////////////////////////////////////
// getRotation
BcF32 ScnSpriteComponent::getRotation() const
{
	return Rotation_;
}

//////////////////////////////////////////////////////////////////////////
// setRotation
void ScnSpriteComponent::setRotation( BcF32 Rotation )
{
	Rotation_ = Rotation;
}

//////////////////////////////////////////////////////////////////////////
// setAnimation
void ScnSpriteComponent::setAnimation( std::string Animation )
{
	Animation_ = Animation;
	CurrKey_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// setMaterial
void ScnSpriteComponent::setMaterial( ScnMaterialComponentRef Material )

{
	Material_ = Material;
}

//////////////////////////////////////////////////////////////////////////
// setSpriteIndex
void ScnSpriteComponent::setSpriteIndex( BcU32 Index )
{
	Index_ = Index;
}
