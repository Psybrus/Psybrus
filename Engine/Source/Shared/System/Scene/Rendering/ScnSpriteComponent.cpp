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
#include "System/Debug/DsCore.h"
#include "Base/BcMath.h"
#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSpriteComponent );
REFLECTION_DEFINE_BASIC( ScnSpriteComponent::Animation );

void ScnSpriteComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Canvas_", &ScnSpriteComponent::Canvas_, bcRFF_TRANSIENT ),
		new ReField( "Material_", &ScnSpriteComponent::Material_, bcRFF_TRANSIENT ),
		new ReField( "Material_", &ScnSpriteComponent::MaterialName_ ),
		new ReField( "Position_", &ScnSpriteComponent::Position_ , DsCore::DsCoreSerialised ),
		new ReField( "Size_", &ScnSpriteComponent::Size_, DsCore::DsCoreSerialised  ),
		new ReField( "Colour_", &ScnSpriteComponent::Colour_, DsCore::DsCoreSerialised ),
		new ReField( "Index_", &ScnSpriteComponent::Index_ ),
		new ReField( "Layer_", &ScnSpriteComponent::Layer_, DsCore::DsCoreSerialised  ),
		new ReField( "Center_", &ScnSpriteComponent::Center_, DsCore::DsCoreSerialised  ),
		new ReField( "IsScreenSpace_", &ScnSpriteComponent::IsScreenSpace_, DsCore::DsCoreSerialised  ),
		new ReField( "CurrKey_", &ScnSpriteComponent::CurrKey_, DsCore::DsCoreSerialised ),
		new ReField( "AnimationTimer_", &ScnSpriteComponent::AnimationTimer_, DsCore::DsCoreSerialised ),
		new ReField( "AnimationRate_", &ScnSpriteComponent::AnimationRate_, DsCore::DsCoreSerialised ),
		new ReField( "Animation_", &ScnSpriteComponent::Animation_, DsCore::DsCoreSerialised ),
		new ReField( "Animations_", &ScnSpriteComponent::Animations_, DsCore::DsCoreSerialised ),
	};
	
	ReRegisterClass< ScnSpriteComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2100 ) );
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
	Rotation_ = 0.0f;

	CurrKey_ = 0;
	AnimationTimer_ = 0.0f;
	AnimationRate_ = 24.0f;
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

	if( Object[ "index" ].type() != Json::nullValue ) 
	{
		Index_ = Object[ "index" ].asUInt();
	}

	if( Object[ "isscreenspace" ].type() != Json::nullValue )
	{
		IsScreenSpace_ = Object[ "isscreenspace" ].asBool();
	}

	if( Object[ "animations" ].type() != Json::nullValue )
	{
		auto AnimMap = Object[ "animations" ];
		auto MemberNames = AnimMap.getMemberNames();
		for( BcU32 Idx = 0; Idx < MemberNames.size(); ++Idx )
		{
			auto MemberName = MemberNames[ Idx ].c_str();
			auto Anim = AnimMap[ MemberName ];
			Animation NewAnimation;
			NewAnimation.Next_ = Anim[ "next" ].asCString();


			auto AnimKeys = Anim[ "keys" ];
			for( BcU32 KeyIdx = 0; KeyIdx < AnimKeys.size(); ++KeyIdx )
			{
				NewAnimation.Keys_.push_back( AnimKeys[ KeyIdx ].asUInt() );
			}

			Animations_[ MemberName ] = NewAnimation;
		}
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

	if( !Animation_.empty() )
	{
		const auto& Animation = Animations_[ Animation_ ];

		if( CurrKey_ < Animation.Keys_.size() )
		{
			Index_ = Animation.Keys_[ CurrKey_ ];
		}
	
		AnimationTimer_ += AnimationRate_;
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
}
