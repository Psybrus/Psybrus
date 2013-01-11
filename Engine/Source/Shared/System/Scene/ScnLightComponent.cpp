/**************************************************************************
*
* File:		ScnLightComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/12/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsCore.h"

#include "System/Scene/ScnLightComponent.h"
#include "System/Scene/ScnLightManagerComponent.h"
#include "System/Scene/ScnMaterial.h"

#include "System/Scene/ScnEntity.h"


//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnLightComponent );

BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnLightComponent )
	BCREFLECTION_MEMBER( ScnLightType,						Type_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( RsColour,							AmbientColour_,					bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( RsColour,							DiffuseColour_,					bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,								AttnC_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,								AttnL_,							bcRFF_DEFAULT ),
	BCREFLECTION_MEMBER( BcF32,								AttnQ_,							bcRFF_DEFAULT ),
BCREFLECTION_DERIVED_END();

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnLightComponent::initialise()
{
	Super::initialise();

	Type_ = scnLT_POINT;
	AmbientColour_ = RsColour( 0.0f, 0.0f, 0.0f, 1.0f );
	DiffuseColour_ = RsColour::WHITE;
	
	createAttenuationValues( 32.0f, 128.0f, 256.0f );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnLightComponent::initialise( const Json::Value& Object )
{
	initialise();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnLightComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

	LightManager_ = getParentEntity()->getComponentAnyParentByType< ScnLightManagerComponent >();
	BcAssertMsg( LightManager_ != NULL, "Can't find an ScnLightManagerComponent in any of our entity's parents. Did you forget to add one to the root entity, or did you attach this entity to the wrong parent?" );

	LightManager_->registerLightComponent( this );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnLightComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );

	LightManager_->unregisterLightComponent( this );
	LightManager_ = NULL;
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
	const BcVec4d& Position( getParentEntity()->getMatrix().row3() );
	const BcVec4d& Direction( getParentEntity()->getMatrix().row2() );
	MaterialComponent->setLightParameters( LightIndex,
	                                       BcVec3d( Position.x(), Position.y(), Position.z() ),
	                                       BcVec3d( Direction.x(), Direction.y(), Direction.z() ),
	                                       AmbientColour_,
										   DiffuseColour_,
	                                       AttnC_,
	                                       AttnL_,
	                                       AttnQ_ );
}
