/**************************************************************************
*
* File:		ScnLightComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	11/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsCore.h"

#include "System/Scene/Rendering/ScnLightComponent.h"
#include "System/Scene/Rendering/ScnMaterial.h"

#include "System/Scene/ScnEntity.h"

#include "System/Scene/Rendering/ScnDebugRenderComponent.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnLightComponent );

void ScnLightComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Type_", &ScnLightComponent::Type_, bcRFF_IMPORTER ),
		new ReField( "AmbientColour_", &ScnLightComponent::AmbientColour_, bcRFF_IMPORTER ),
		new ReField( "DiffuseColour_", &ScnLightComponent::DiffuseColour_, bcRFF_IMPORTER ),
		new ReField( "Min_", &ScnLightComponent::Min_, bcRFF_IMPORTER ),
		new ReField( "Mid_", &ScnLightComponent::Mid_, bcRFF_IMPORTER ),
		new ReField( "Max_", &ScnLightComponent::Max_, bcRFF_IMPORTER ),

		new ReField( "AttnC_", &ScnLightComponent::AttnC_ ),
		new ReField( "AttnL_", &ScnLightComponent::AttnL_ ),
		new ReField( "AttnQ_", &ScnLightComponent::AttnQ_ ),
	};
		
	ReRegisterClass< ScnLightComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( -2040 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnLightComponent::ScnLightComponent()
{
	Type_ = scnLT_POINT;
	AmbientColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
	DiffuseColour_ = RsColour::WHITE;	
	Min_ = 64.0f;
	Mid_ = 256.0f;
	Max_ = 1024.0f;
	AttnC_ = 0.0f;
	AttnL_ = 0.0f;
	AttnQ_ = 0.0f;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ScnLightComponent::~ScnLightComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnLightComponent::update( BcF32 Tick )
{
	createAttenuationValues( Min_, Mid_, Max_ );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnLightComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnLightComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// getAmbientColour
const RsColour& ScnLightComponent::getAmbientColour() const
{
	return AmbientColour_;
}

//////////////////////////////////////////////////////////////////////////
// getDiffuseColour
const RsColour& ScnLightComponent::getDiffuseColour() const
{
	return DiffuseColour_;
}

//////////////////////////////////////////////////////////////////////////
// findAttenuationByDistance
BcF32 ScnLightComponent::findAttenuationByDistance( BcF32 Distance ) const
{
	BcF32 InvA = ( ( AttnC_ + ( Distance * AttnL_ ) + ( Distance * Distance * AttnQ_ ) ) );
	return 1.0f / InvA;
}

//////////////////////////////////////////////////////////////////////////
// findDistanceByAttenuation
BcF32 ScnLightComponent::findDistanceByAttenuation( BcF32 Attenuation ) const
{
	// If we want it at a low value, return max distance.
	if( Attenuation < 0.05f )
	{
		Attenuation = 0.05f;
	}

	BcF32 A = AttnQ_;
	BcF32 B = AttnL_;
	BcF32 C = AttnC_ - ( 1.0f / Attenuation );

	// 0 = Ax2 + Bx + C
	if( A > 0.0f )
	{
		BcF32 Discriminant = ( ( B * B ) - ( 4.0f * A * C ) );
		if( Discriminant < 0.0f )
		{
			return 1e24f;
		}
		return BcAbs( ( -B + BcSqrt( Discriminant ) ) / ( 2.0f * A ) );
	}
	// 0 = Bx + C
	else if( B > 0.0f )
	{
		return BcAbs( -C / B );
	}
	// 0 = C
	else
	{
		return 1e24f;
	}
}

//////////////////////////////////////////////////////////////////////////
// createAttenuationValues
void ScnLightComponent::createAttenuationValues( BcF32 MinDistance, BcF32 MidDistance, BcF32 MaxDistance )
{
	// x = min - 1 ( 1 )
	// y = mid - 12 ( 144 )
	// z = max - 16 ( 256 )
	//
	// Ax^2 + Bx + C = 1.0f / 1.0f
	// Ay^2 + By + C = 1.0f / 0.5f
	// Az^2 + Bz + C = 1.0f / 0.1f
	//
	// Solve for A, B and C. x, y & z are all known positive values.

	// TODO: Implement MaxDistance. Only linear is produced from this.
	AttnQ_ = 0.0f;
	AttnL_ = 1.0f / ( MidDistance - MinDistance );
	AttnC_ = ( 1.0f - MinDistance * AttnL_ );

	// Post-conditions:
	// findAttenuationByDistance( MinDistance ) == 1.0f +/- Epsilon
	// findAttenuationByDistance( MidDistance ) == 0.5f +/- Epsilon
	// findAttenuationByDistance( MaxDistance ) == 0.1f +/- Epsilon
}

//////////////////////////////////////////////////////////////////////////
// setMaterialParameters
void ScnLightComponent::setMaterialParameters( BcU32 LightIndex, ScnMaterialComponent* MaterialComponent )
{
	/*
	const MaVec4d& Position( getParentEntity()->getWorldMatrix().row3() );
	const MaVec4d& Direction( getParentEntity()->getWorldMatrix().row2() );
	MaterialComponent->setLightParameters( LightIndex,
	                                       MaVec3d( Position.x(), Position.y(), Position.z() ),
	                                       MaVec3d( Direction.x(), Direction.y(), Direction.z() ),
	                                       AmbientColour_,
	                                       DiffuseColour_,
	                                       AttnC_,
	                                       AttnL_,
	                                       AttnQ_ );
	                                       */
}

//////////////////////////////////////////////////////////////////////////
// getAABB
MaAABB ScnLightComponent::getAABB() const
{
	const BcF32 MinAttn = 0.01f;
	const BcF32 MaxDistance = findDistanceByAttenuation( MinAttn );
	const MaVec3d MaxDistanceVec3( MaxDistance, MaxDistance, MaxDistance );
	return MaAABB( -MaxDistanceVec3, MaxDistanceVec3 );
}
