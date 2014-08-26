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
DEFINE_RESOURCE( ScnLightComponent );

void ScnLightComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Type_",				&ScnLightComponent::Type_ ),
		new ReField( "AmbientColour_",		&ScnLightComponent::AmbientColour_ ),
		new ReField( "DiffuseColour_",		&ScnLightComponent::DiffuseColour_ ),
		new ReField( "AttnC_",				&ScnLightComponent::AttnC_ ),
		new ReField( "AttnL_",				&ScnLightComponent::AttnL_ ),
		new ReField( "AttnQ_",				&ScnLightComponent::AttnQ_ ),
	};
		
	ReRegisterClass< ScnLightComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2040 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnLightComponent::initialise()
{
	Super::initialise();

	Type_ = scnLT_POINT;
	AmbientColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
	DiffuseColour_ = RsColour::WHITE;
	
	createAttenuationValues( 64.0f, 256.0f, 1024.0f );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnLightComponent::initialise( const Json::Value& Object )
{
	initialise();

	const Json::Value& AmbientColourValue = Object[ "ambientcolour" ];
	if( AmbientColourValue != Json::nullValue )
	{
		AmbientColour_ = MaVec4d( AmbientColourValue.asCString() );
	}

	const Json::Value& DiffuseColourValue = Object[ "diffusecolour" ];
	if( DiffuseColourValue != Json::nullValue )
	{
		DiffuseColour_ = MaVec4d( DiffuseColourValue.asCString() );
	}

	const Json::Value& MinValue = Object[ "min" ];
	const Json::Value& MidValue = Object[ "mid" ];
	const Json::Value& MaxValue = Object[ "max" ];
	if( MinValue != Json::nullValue &&
		MidValue != Json::nullValue &&
		MaxValue != Json::nullValue )
	{
		createAttenuationValues( (BcF32)MinValue.asDouble(), (BcF32)MidValue.asDouble(), (BcF32)MaxValue.asDouble() );
	}
}

//////////////////////////////////////////////////////////////////////////
// onAttach
void ScnLightComponent::update( BcF32 Tick )
{
	//ScnDebugRenderComponent::pImpl()->drawEllipsoid( getParentEntity()->getWorldPosition(), MaVec3d( 1.0f, 1.0f, 1.0f ), DiffuseColour_ );
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
	const MaVec4d& Position( getParentEntity()->getWorldMatrix().row3() );
	const MaVec4d& Direction( getParentEntity()->getWorldMatrix().row2() );
	/*
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
