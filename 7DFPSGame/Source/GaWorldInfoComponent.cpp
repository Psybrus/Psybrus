/**************************************************************************
*
* File:		GaWorldInfoComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		World info component.
*		
*
*
* 
**************************************************************************/

#include "GaWorldInfoComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaWorldInfoComponent );

BCREFLECTION_EMPTY_REGISTER( GaWorldInfoComponent );
/*
BCREFLECTION_DERIVED_BEGIN( ScnComponent, GaWorldInfoComponent )
	BCREFLECTION_MEMBER( BcName,							Name_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								Index_,							bcRFF_DEFAULT | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( CsPackage,							pPackage_,						bcRFF_POINTER | bcRFF_TRANSIENT ),
	BCREFLECTION_MEMBER( BcU32,								RefCount_,						bcRFF_DEFAULT | bcRFF_TRANSIENT ),
BCREFLECTION_DERIVED_END();
*/

//////////////////////////////////////////////////////////////////////////
// initialise
void GaWorldInfoComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	Width_ = Object["width"].asUInt();
	Height_ = Object["height"].asUInt();
	Depth_ = Object["depth"].asUInt();
	Scale_ = static_cast< BcF32 >( Object["scale"].asDouble() );
	Damping_ = static_cast< BcF32 >( Object["damping"].asDouble() );
	AccumulatorMultiplier_ = static_cast< BcF32 >( Object["accumulatormultiplier"].asDouble() );
}

//////////////////////////////////////////////////////////////////////////
// getWidth
const BcU32 GaWorldInfoComponent::getWidth() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
const BcU32 GaWorldInfoComponent::getHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// getDepth
const BcU32 GaWorldInfoComponent::getDepth() const
{
	return Depth_;
}

//////////////////////////////////////////////////////////////////////////
// getScale
const BcF32 GaWorldInfoComponent::getScale() const
{
	return Scale_;
}

//////////////////////////////////////////////////////////////////////////
// getDamping
const BcF32 GaWorldInfoComponent::getDamping() const
{
	return Damping_;
}

//////////////////////////////////////////////////////////////////////////
// getAccumulatorMultiplier
const BcF32 GaWorldInfoComponent::getAccumulatorMultiplier() const
{
	return AccumulatorMultiplier_;
}
