/**************************************************************************
*
* File:		Psybrus.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Engine.
*		
*
*
* 
**************************************************************************/

#ifndef __PSYBRUS_H__
#define __PSYBRUS_H__

#include "Base/BcTypes.h"

// Most of the game code will interact with the scene core at some point, so include this only.
#include "System/Scene/ScnCore.h"

#include <string>

//////////////////////////////////////////////////////////////////////////
// Setup and initialisation.
enum PsySetupFlags
{
	// Core.
	psySF_CONSOLE							= 0x00000001,		// Setup a console window.
	psySF_WINDOW							= 0x00000002,		// Setup a window.
	psySF_MANUAL							= 0x00000004,		// Manually start up engine internally.

	// Systems.
	psySF_REMOTE							= 0x00000010,		// Create remoting system.
	psySF_RENDER							= 0x00000020,		// Create render system.
	psySF_SOUND								= 0x00000040,		// Create sound system.

	// Content.
	psySF_CONTENT_CLIENT					= 0x00000100,		// Content client.
	psySF_CONTENT_SERVER					= 0x00000200,		// Content server.

	// Defaults.
	psySF_DEFAULT_SYSTEMS					= psySF_RENDER | psySF_SOUND,
	psySF_GAME_FINAL						= psySF_DEFAULT_SYSTEMS | psySF_WINDOW,
	psySF_GAME_DEV							= psySF_CONTENT_CLIENT | psySF_GAME_FINAL | psySF_REMOTE,
	psySF_SERVER							= psySF_CONTENT_SERVER | psySF_CONSOLE | psySF_REMOTE | psySF_DEFAULT_SYSTEMS,
};

struct PsySetupParams
{
	PsySetupParams( const std::string& Name = "Psybrus Game", 
		            BcU32 Flags = 0,
					BcF32 TickRate = 1.0f / 60.0f ):
		Name_( Name ),
		Flags_( Flags ),
		TickRate_( TickRate )
	{
	}

	BcU32			Flags_;
	std::string		Name_;
	BcF32			TickRate_;
};

/**
 * Setup by the user as a global.
 */
extern PsySetupParams GPsySetupParams;

/**
 * Called after engine is initialised, and game needs to be initialised.
 */
extern void PsyGameInit();


#endif
