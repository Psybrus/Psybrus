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

#include "BcTypes.h"

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

// TODO: Put this back in.
//# define EX_AL_EFX 
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
#ifdef EX_AL_EFX
extern LPALGENEFFECTS alGenEffects = EX_NULL;
extern LPALDELETEEFFECTS alDeleteEffects = EX_NULL;
extern LPALISEFFECT alIsEffect = EX_NULL;
extern LPALEFFECTI alEffecti = EX_NULL;
extern LPALEFFECTIV alEffectiv = EX_NULL;
extern LPALEFFECTF alEffectf = EX_NULL;
extern LPALEFFECTFV alEffectfv = EX_NULL;
extern LPALGETEFFECTI alGetEffecti = EX_NULL;
extern LPALGETEFFECTIV alGetEffectiv = EX_NULL;
extern LPALGETEFFECTF alGetEffectf = EX_NULL;
extern LPALGETEFFECTFV alGetEffectfv = EX_NULL;

//Filter objects
extern LPALGENFILTERS alGenFilters = EX_NULL;
extern LPALDELETEFILTERS alDeleteFilters = EX_NULL;
extern LPALISFILTER alIsFilter = EX_NULL;
extern LPALFILTERI alFilteri = EX_NULL;
extern LPALFILTERIV alFilteriv = EX_NULL;
extern LPALFILTERF alFilterf = EX_NULL;
extern LPALFILTERFV alFilterfv = EX_NULL;
extern LPALGETFILTERI alGetFilteri = EX_NULL;
extern LPALGETFILTERIV alGetFilteriv = EX_NULL;
extern LPALGETFILTERF alGetFilterf = EX_NULL;
extern LPALGETFILTERFV alGetFilterfv = EX_NULL;

// Auxiliary slot object
extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = EX_NULL;
extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = EX_NULL;
extern LPALISAUXILIARYEFFECTSLOT alIsAuxiliaryEffectSlot = EX_NULL;
extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = EX_NULL;
extern LPALAUXILIARYEFFECTSLOTIV alAuxiliaryEffectSlotiv = EX_NULL;
extern LPALAUXILIARYEFFECTSLOTF alAuxiliaryEffectSlotf = EX_NULL;
extern LPALAUXILIARYEFFECTSLOTFV alAuxiliaryEffectSlotfv = EX_NULL;
extern LPALGETAUXILIARYEFFECTSLOTI alGetAuxiliaryEffectSloti = EX_NULL;
extern LPALGETAUXILIARYEFFECTSLOTIV alGetAuxiliaryEffectSlotiv = EX_NULL;
extern LPALGETAUXILIARYEFFECTSLOTF alGetAuxiliaryEffectSlotf = EX_NULL;
extern LPALGETAUXILIARYEFFECTSLOTFV alGetAuxiliaryEffectSlotfv = EX_NULL;
#endif

#endif
