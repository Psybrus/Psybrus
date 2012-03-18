/**************************************************************************
 *
 * File:		SsAL.h
 * Author:		Neil Richardson
 * Ver/Date:	
 * Description:
 *		
 *
 *
 *
 * 
 **************************************************************************/

#ifndef __SSAL_H__
#define __SSAL_H__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// Linux Includes
#if PLATFORM_LINUX
# include <AL/al.h>
# include <AL/alc.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Windows Includes
#if PLATFORM_WINDOWS
# include <al.h>
# include <alc.h>
# include <efx.h>
# include <EFX-Util.h>
# include <efx-creative.h>

# define SS_AL_EFX_SUPPORTED 1
#endif

//////////////////////////////////////////////////////////////////////////
// OSX Includes
#if PLATFORM_OSX
# include <CoreAudio/CoreAudioTypes.h>
# include <OpenAL/al.h>
# include <OpenAL/alc.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Utility
extern void alBreakOnError();

//////////////////////////////////////////////////////////////////////////
// EFX Functions
#if SS_AL_EFX_SUPPORTED
extern LPALGENEFFECTS alGenEffects;
extern LPALDELETEEFFECTS alDeleteEffects;
extern LPALISEFFECT alIsEffect;
extern LPALEFFECTI alEffecti;
extern LPALEFFECTIV alEffectiv;
extern LPALEFFECTF alEffectf;
extern LPALEFFECTFV alEffectfv;
extern LPALGETEFFECTI alGetEffecti;
extern LPALGETEFFECTIV alGetEffectiv;
extern LPALGETEFFECTF alGetEffectf;
extern LPALGETEFFECTFV alGetEffectfv;

//Filter objects
extern LPALGENFILTERS alGenFilters;
extern LPALDELETEFILTERS alDeleteFilters;
extern LPALISFILTER alIsFilter;
extern LPALFILTERI alFilteri;
extern LPALFILTERIV alFilteriv;
extern LPALFILTERF alFilterf;
extern LPALFILTERFV alFilterfv;
extern LPALGETFILTERI alGetFilteri;
extern LPALGETFILTERIV alGetFilteriv;
extern LPALGETFILTERF alGetFilterf;
extern LPALGETFILTERFV alGetFilterfv;

// Auxiliary slot object
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
extern LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
extern LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf;
extern LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv;
extern LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti;
extern LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv;
extern LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf;
extern LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv;
#endif

#endif
