/**************************************************************************
*
* File:		AkIncludes.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		All Audiokinetic Wwise includes.
*		
*
*
* 
**************************************************************************/

#ifndef __AKINCLUDES_H__
#define __AKINCLUDES_H__

#include <AK/SoundEngine/Common/AkTypes.h>

#include <AK/SoundEngine/Common/AkMemoryMgr.h>		// Memory Manager
#include <AK/SoundEngine/Common/AkModule.h>			// Default memory and stream managers
#include <AK/SoundEngine/Common/IAkStreamMgr.h>		// Streaming Manager
#include <AK/SoundEngine/Common/AkSoundEngine.h>    // Sound engine
#include <AK/MusicEngine/Common/AkMusicEngine.h>	// Music Engine
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>	// AkStreamMgrModule

#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>	// Communication between Wwise and the game (excluded in release build)
#endif
#endif // __AKINCLUDES_H__
