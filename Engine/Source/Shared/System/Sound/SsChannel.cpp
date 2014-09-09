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
SsChannelParams::SsChannelParams():
	Position_( 0.0f, 0.0f, 0.0f ),
	Gain_( 1.0f ),
	Pitch_( 1.0f )
{

}

//////////////////////////////////////////////////////////////////////////
// SsChannelParams
SsChannelParams::SsChannelParams( 
		MaVec3d Position, 
		BcF32 Gain,
		BcF32 Pitch ):
	Position_( Position ),
	Gain_( Gain ),
	Pitch_( Pitch )
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

