/**************************************************************************
*
* File:		RsLight.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Light object.
*		
*
*
* 
**************************************************************************/

#ifndef __RSLIGHT_H__
#define __RSLIGHT_H__

#include "Base/BcTypes.h"
#include "Base/BcVec4d.h"

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Enums
enum eRsLightType
{
	rsLT_NONE = 0,
	rsLT_OMNI,
	rsLT_DIR,
};

//////////////////////////////////////////////////////////////////////////
// RsLight
class RsLight
{
public:
	// Type.
	eRsLightType	Type_;

	// Position/Direction.
	BcVec4d			PosDir_;

	// Colour.
	RsColour		Colour_;

	// Decay.
	BcF32			AttnC_;
	BcF32			AttnL_;
	BcF32			AttnQ_;

public:
	/**
	*	Find attenuation of light by a distance.
	*/
	BcF32 findAttenuationByDistance( BcF32 Distance ) const;

	/**
	*	Find distance from light by an attenuation.
	*/
	BcF32 findDistanceByAttenuation( BcF32 Attenuation ) const;
	
	/**
	*	Create attenuation values using min, mid and max distances.
	*/
	void createAttenuationValues( BcF32 MinDistance, BcF32 MidDistance, BcF32 MaxDistance );
};

#endif

