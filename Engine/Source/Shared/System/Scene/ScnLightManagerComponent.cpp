/**************************************************************************
*
* File:		ScnLightManagerComponent.cpp
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

#include "System/Scene/ScnLightManagerComponent.h"

#include "System/Scene/ScnLightComponent.h"
#include "System/Scene/ScnMaterial.h"

#include "System/Scene/ScnEntity.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnLightManagerComponent );

BCREFLECTION_DERIVED_BEGIN( ScnComponent, ScnLightManagerComponent )
	BCREFLECTION_MEMBER( BcBool,				DummyMemberRemove_,				bcRFF_DEFAULT ),
BCREFLECTION_DERIVED_END();

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnLightManagerComponent::initialise()
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnLightManagerComponent::initialise( const Json::Value& Object )
{
	initialise();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnLightManagerComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnLightManagerComponent::onDetach( ScnEntityWeakRef Parent )
{
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// registerLightComponent
//virtual
void ScnLightManagerComponent::registerLightComponent( ScnLightComponent* LightComponent )
{
	LightComponents_.push_back( LightComponent );
}

//////////////////////////////////////////////////////////////////////////
// unregisterLightComponent
//virtual
void ScnLightManagerComponent::unregisterLightComponent( ScnLightComponent* LightComponent )
{
	TLightComponentList::iterator It( std::find( LightComponents_.begin(), LightComponents_.end(), LightComponent ) );
	BcAssert( It != LightComponents_.end() );
	LightComponents_.erase( It );
}

//////////////////////////////////////////////////////////////////////////
// setMaterialParameters
void ScnLightManagerComponent::setMaterialParameters( ScnMaterialComponent* MaterialComponent ) const
{
	// TODO: Determine closest lights!
	BcU32 Count = BcMin( (BcU32)4, (BcU32)LightComponents_.size() );
	for( BcU32 Idx = 0; Idx < Count; ++Idx )
	{
		LightComponents_[ Idx ]->setMaterialParameters( Idx, MaterialComponent );
	}	

	for( BcU32 Idx = Count; Idx < 4; ++Idx )
	{
		MaterialComponent->setLightParameters( Idx,
	                                           BcVec3d( 0.0f, 0.0f, 0.0f ),
	                                           BcVec3d( 0.0f, 0.0f, 0.0f ),
	                                           RsColour::BLACK,
	                                           RsColour::BLACK,
	                                           0.0f,
	                                           0.0f,
	                                           0.0f );
	}

}
