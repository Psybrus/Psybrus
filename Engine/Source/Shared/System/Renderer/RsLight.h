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
#include "Base/BcVectors.h"

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
	BcReal			AttnC_;
	BcReal			AttnL_;
	BcReal			AttnQ_;

public:
	/**
	*	Find attenuation of light by a distance.
	*/
	BcReal findAttenuationByDistance( BcReal Distance ) const;

	/**
	*	Find distance from light by an attenuation.
	*/
	BcReal findDistanceByAttenuation( BcReal Attenuation ) const;
	
	/**
	*	Create attenuation values using min, mid and max distances.
	*/
	void createAttenuationValues( BcReal MinDistance, BcReal MidDistance, BcReal MaxDistance );
};

#endif

