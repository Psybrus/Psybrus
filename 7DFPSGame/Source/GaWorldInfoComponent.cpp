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

//////////////////////////////////////////////////////////////////////////
// initialise
void GaWorldInfoComponent::initialise( const Json::Value& Object )
{
	Width_ = Object["width"].asUInt();
	Height_ = Object["height"].asUInt();
	Depth_ = Object["depth"].asUInt();
	Scale_ = static_cast< BcReal >( Object["scale"].asDouble() );
	Damping_ = static_cast< BcReal >( Object["damping"].asDouble() );
	AccumulatorMultiplier_ = static_cast< BcReal >( Object["accumulatormultiplier"].asDouble() );
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
const BcReal GaWorldInfoComponent::getScale() const
{
	return Scale_;
}

//////////////////////////////////////////////////////////////////////////
// getDamping
const BcReal GaWorldInfoComponent::getDamping() const
{
	return Damping_;
}

//////////////////////////////////////////////////////////////////////////
// getAccumulatorMultiplier
const BcReal GaWorldInfoComponent::getAccumulatorMultiplier() const
{
	return AccumulatorMultiplier_;
}
