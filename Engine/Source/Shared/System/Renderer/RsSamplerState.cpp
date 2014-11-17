/**************************************************************************
*
* File:		RsSamplerState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Render state creation and management.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsSamplerState.h"

#include <limits>

//////////////////////////////////////////////////////////////////////////
// RsSamplerStateDesc
void RsSamplerStateDesc::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "AddressU_", &RsSamplerStateDesc::AddressU_ ),
		new ReField( "AddressV_", &RsSamplerStateDesc::AddressV_ ),
		new ReField( "AddressW_", &RsSamplerStateDesc::AddressW_ ),
		new ReField( "MipLODBias_", &RsSamplerStateDesc::MipLODBias_ ),
		new ReField( "MaxAnisotropy_", &RsSamplerStateDesc::MaxAnisotropy_ ),
		new ReField( "BorderColour_", &RsSamplerStateDesc::BorderColour_ ),
		new ReField( "MinLOD_", &RsSamplerStateDesc::MinLOD_ ),
		new ReField( "MaxLOD_", &RsSamplerStateDesc::MaxLOD_ ),
	};
		
	ReRegisterClass< RsSamplerStateDesc >( Fields );
}

RsSamplerStateDesc::RsSamplerStateDesc( ReNoInit ):
	AddressU_( RsTextureSamplingMode::WRAP ),
	AddressV_( RsTextureSamplingMode::WRAP ),
	AddressW_( RsTextureSamplingMode::WRAP ),
	MipLODBias_( 0.0f ),
	MaxAnisotropy_( 1 ),
	BorderColour_( RsColour::BLACK ),
	MinLOD_( -std::numeric_limits< BcF32 >::max() ),
	MaxLOD_( std::numeric_limits< BcF32 >::max() )
{

}

RsSamplerStateDesc::RsSamplerStateDesc():
	AddressU_( RsTextureSamplingMode::WRAP ),
	AddressV_( RsTextureSamplingMode::WRAP ),
	AddressW_( RsTextureSamplingMode::WRAP ),
	MipLODBias_( 0.0f ),
	MaxAnisotropy_( 1 ),
	BorderColour_( RsColour::BLACK ),
	MinLOD_( -std::numeric_limits< BcF32 >::max() ),
	MaxLOD_( std::numeric_limits< BcF32 >::max() )
{

}


//////////////////////////////////////////////////////////////////////////
// Ctor
RsSamplerState::RsSamplerState( class RsContext* pContext, const RsSamplerStateDesc& Desc ):
	RsResource( pContext ),
	Desc_( Desc )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsSamplerState::~RsSamplerState()
{

}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsSamplerStateDesc& RsSamplerState::getDesc() const
{
	return Desc_;
}
