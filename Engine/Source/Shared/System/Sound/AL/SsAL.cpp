/**************************************************************************
 *
 * File:		SsAL.cpp
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#include "SsAL.h"
#include "BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// Utility
void alBreakOnError()
{
	ALenum Error = alGetError();
	if( Error != 0 )
	{
		switch( Error )
		{
		case AL_INVALID_NAME:
			BcPrintf( "alBreakOnError: AL_INVALID_NAME\n" );
			break;
		case AL_INVALID_ENUM:
			BcPrintf( "alBreakOnError: AL_INVALID_ENUM\n" );
			break;
		case AL_INVALID_VALUE:    
			BcPrintf( "alBreakOnError: AL_INVALID_VALUE\n" );
			break;
		case AL_ILLEGAL_COMMAND:  
			BcPrintf( "alBreakOnError: AL_ILLEGAL_COMMAND\n" );
			break;
		default:
			BcPrintf( "alBreakOnError: Unknown error\n" );
			break;
		}

		//BcBreakpoint;
	}
}

//////////////////////////////////////////////////////////////////////////
// EFX Functions
#if SS_AL_EFX_SUPPORTED
// Effect objects
LPALGENEFFECTS alGenEffects = NULL;
LPALDELETEEFFECTS alDeleteEffects = NULL;
LPALISEFFECT alIsEffect = NULL;
LPALEFFECTI alEffecti = NULL;
LPALEFFECTIV alEffectiv = NULL;
LPALEFFECTF alEffectf = NULL;
LPALEFFECTFV alEffectfv = NULL;
LPALGETEFFECTI alGetEffecti = NULL;
LPALGETEFFECTIV alGetEffectiv = NULL;
LPALGETEFFECTF alGetEffectf = NULL;
LPALGETEFFECTFV alGetEffectfv = NULL;

//Filter objects
LPALGENFILTERS alGenFilters = NULL;
LPALDELETEFILTERS alDeleteFilters = NULL;
LPALISFILTER alIsFilter = NULL;
LPALFILTERI alFilteri = NULL;
LPALFILTERIV alFilteriv = NULL;
LPALFILTERF alFilterf = NULL;
LPALFILTERFV alFilterfv = NULL;
LPALGETFILTERI alGetFilteri = NULL;
LPALGETFILTERIV alGetFilteriv = NULL;
LPALGETFILTERF alGetFilterf = NULL;
LPALGETFILTERFV alGetFilterfv = NULL;

// Auxiliary slot object
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = NULL;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = NULL;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = NULL;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = NULL;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = NULL;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = NULL;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = NULL;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = NULL;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = NULL;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = NULL;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = NULL;
#endif
