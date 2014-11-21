/**************************************************************************
*
* File:		RsSamplerState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Render state creation and management.
*		
*
*
* 
**************************************************************************/

#ifndef __RSSAMPLERSTATE_H__
#define __RSSAMPLERSTATE_H__

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsSamplerStateDesc
struct RsSamplerStateDesc
{
	REFLECTION_DECLARE_BASIC_MANUAL_NOINIT( RsSamplerStateDesc );

	RsSamplerStateDesc( ReNoInit );
	RsSamplerStateDesc();

	RsTextureSamplingMode AddressU_;
	RsTextureSamplingMode AddressV_;
	RsTextureSamplingMode AddressW_;
	RsTextureFilteringMode MinFilter_;
	RsTextureFilteringMode MagFilter_;
	BcF32 MipLODBias_;
	BcU32 MaxAnisotropy_;
	RsColour BorderColour_;
	BcF32 MinLOD_;
	BcF32 MaxLOD_;
};

//////////////////////////////////////////////////////////////////////////
// RsSamplerState
class RsSamplerState:
	public RsResource
{
public:
	RsSamplerState( class RsContext* pContext, const RsSamplerStateDesc& Desc );
	virtual ~RsSamplerState();

	/**
	 * Get desc.
	 */
	const RsSamplerStateDesc& getDesc() const;

private:
	RsSamplerStateDesc Desc_;
};


#endif // __RSSAMPLERSTATE_H__
