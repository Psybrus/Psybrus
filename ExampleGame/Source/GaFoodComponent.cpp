/**************************************************************************
*
* File:		GaFoodComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Food.
*		
*
*
* 
**************************************************************************/

#include "GaFoodComponent.h"

#include "GaMainGameState.h"

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( GaFoodComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaFoodComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaFoodComponent" )
	.endCatagory();
}

////////////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void GaFoodComponent::initialise( const BcVec2d& Position )
{
	Super::initialise();

	Position_ = Position;
	StartingHealth_ = 10.0f;
	Health_ = StartingHealth_;

	ScnMaterialRef Material;
	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_LETTUCE, Material );

	CsCore::pImpl()->createResource( BcName::INVALID, MaterialComponent_, Material, scnSPF_DEFAULT );
	CsCore::pImpl()->createResource( BcName::INVALID, ShadowMaterialComponent_, ScnMaterial::Default, scnSPF_DEFAULT );
}

////////////////////////////////////////////////////////////////////////////////
// eat
void GaFoodComponent::eat( BcReal Tick )
{
	Health_ -= Tick;
}

////////////////////////////////////////////////////////////////////////////////
// isAlive
BcBool GaFoodComponent::isAlive()
{
	return Health_ > 0.0f;
}

////////////////////////////////////////////////////////////////////////////////
// getHealthFraction
BcReal GaFoodComponent::getHealthFraction()
{
	return Health_ / StartingHealth_;
}

////////////////////////////////////////////////////////////////////////////////
// update
//virtual
void GaFoodComponent::update( BcReal Tick )
{

}

////////////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaFoodComponent::render( ScnCanvasComponentRef Canvas )
{
	BcVec2d Size( 32.0f, -32.0f );

	BcMat4d PositionMatrix;

	PositionMatrix.translation( BcVec3d( Position_.x(), Position_.y(), 0.0f ) );
	
	BcRandom RandomAdjustment = BcRandom( BcHash( (BcU8*)&Position_, sizeof( Position_ ) ) );

	Canvas->setMaterialComponent( ShadowMaterialComponent_ );
	pParent()->setMaterialComponentParams( ShadowMaterialComponent_, PositionMatrix );

	for( BcReal X = -48.0f; X <= 48.0f; X += 24.0f )
	{
		for( BcReal Y = -48.0f; Y <= 48.0f; Y += 24.0f )
		{
			BcReal NormalisedHealth = Health_ / StartingHealth_;
			BcReal ScaleValue = BcClamp( BcAbs( RandomAdjustment.randReal() ), 0.0f, 1.0f );
			BcReal RenderHealth = BcClamp( ( ( NormalisedHealth - ScaleValue ) ) / ( 1.0f - ScaleValue ), 0.0f, 1.0f );

			Canvas->drawSpriteCentered3D( BcVec3d( X, Y, 0.1f ), BcVec2d( 48.0f, 48.0f ) * RenderHealth, 0, RsColour::WHITE, 6 );
		}
	}

	RandomAdjustment = BcRandom( BcHash( (BcU8*)&Position_, sizeof( Position_ ) ) );

	Canvas->setMaterialComponent( MaterialComponent_ );
	pParent()->setMaterialComponentParams( MaterialComponent_, PositionMatrix );

	for( BcReal X = -48.0f; X <= 48.0f; X += 24.0f )
	{
		for( BcReal Y = -48.0f; Y <= 48.0f; Y += 24.0f )
		{
			BcReal NormalisedHealth = Health_ / StartingHealth_;
			BcReal ScaleValue = BcClamp( BcAbs( RandomAdjustment.randReal() ), 0.0f, 1.0f );
			BcReal RenderHealth = ( ( NormalisedHealth - ScaleValue ) ) / ( 1.0f - ScaleValue );
			
			BcU32 SpriteIdx = BcClamp( (BcU32)( 24.0f - ( RenderHealth * 24.0f ) ), 0, 23 );
			Canvas->drawSpriteCenteredUp3D( BcVec3d( X, Y, -Size.y() * 0.3f ), Size, SpriteIdx, RsColour::WHITE, 7 );
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaFoodComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	Parent->attach( MaterialComponent_ );
	Parent->attach( ShadowMaterialComponent_ );
}

////////////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaFoodComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	Parent->detach( MaterialComponent_ );
	Parent->detach( ShadowMaterialComponent_ );
}
