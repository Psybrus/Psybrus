/**************************************************************************
 *
 * File:		SsChannel.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "System/Sound/SsChannel.h"

//////////////////////////////////////////////////////////////////////////
// SsChannelParams
SsChannelParams::SsChannelParams( 
		BcF32 Gain,
		BcF32 Pitch,
		BcF32 Min,
		BcF32 Max,
		SsAttenuationModel AttenuationModel,
		BcF32 RolloffFactor,
		MaVec3d Position, 
		MaVec3d Velocity ):
	Gain_( Gain ),
	Pitch_( Pitch ),
	Min_( Min ),
	Max_( Max ),
	AttenuationModel_( AttenuationModel ),
	RolloffFactor_( RolloffFactor ),
	Position_( Position ),
	Velocity_( Velocity )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
SsChannel::SsChannel( const SsChannelParams& Params ):
	Params_( Params )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
SsChannel::~SsChannel()
{

}

//////////////////////////////////////////////////////////////////////////
// getParams
const SsChannelParams& SsChannel::getParams() const
{
	return Params_;
}

//////////////////////////////////////////////////////////////////////////
// setParams
void SsChannel::setParams( const SsChannelParams& Params )
{
	Params_ = Params;
}

