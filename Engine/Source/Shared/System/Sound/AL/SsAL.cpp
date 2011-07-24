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
#ifdef EX_AL_EFX
// Effect objects
LPALGENEFFECTS alGenEffects = EX_NULL;
LPALDELETEEFFECTS alDeleteEffects = EX_NULL;
LPALISEFFECT alIsEffect = EX_NULL;
LPALEFFECTI alEffecti = EX_NULL;
LPALEFFECTIV alEffectiv = EX_NULL;
LPALEFFECTF alEffectf = EX_NULL;
LPALEFFECTFV alEffectfv = EX_NULL;
LPALGETEFFECTI alGetEffecti = EX_NULL;
LPALGETEFFECTIV alGetEffectiv = EX_NULL;
LPALGETEFFECTF alGetEffectf = EX_NULL;
LPALGETEFFECTFV alGetEffectfv = EX_NULL;

//Filter objects
LPALGENFILTERS alGenFilters = EX_NULL;
LPALDELETEFILTERS alDeleteFilters = EX_NULL;
LPALISFILTER alIsFilter = EX_NULL;
LPALFILTERI alFilteri = EX_NULL;
LPALFILTERIV alFilteriv = EX_NULL;
LPALFILTERF alFilterf = EX_NULL;
LPALFILTERFV alFilterfv = EX_NULL;
LPALGETFILTERI alGetFilteri = EX_NULL;
LPALGETFILTERIV alGetFilteriv = EX_NULL;
LPALGETFILTERF alGetFilterf = EX_NULL;
LPALGETFILTERFV alGetFilterfv = EX_NULL;

// Auxiliary slot object
LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = EX_NULL;
LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = EX_NULL;
LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = EX_NULL;
LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = EX_NULL;
LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = EX_NULL;
LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = EX_NULL;
LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = EX_NULL;
LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = EX_NULL;
LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = EX_NULL;
LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = EX_NULL;
LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = EX_NULL;
#endif
