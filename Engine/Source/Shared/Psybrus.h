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
#include "System/Scene/ScnComponentProcessor.h"

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
	psySF_SCENE								= 0x00000080,		// Create scene system.

	// Run mode.
	psySF_GAME								= 0x00000100,		// Game.
	psySF_TOOL								= 0x00000200,		// Tool.

	// Defaults.
	psySF_DEFAULT_SYSTEMS					= psySF_RENDER | psySF_SOUND | psySF_SCENE,
	psySF_GAME_FINAL						= psySF_DEFAULT_SYSTEMS | psySF_WINDOW,
	psySF_GAME_DEV							= psySF_GAME | psySF_GAME_FINAL | psySF_REMOTE,
	psySF_SERVER							= psySF_GAME | psySF_CONSOLE | psySF_REMOTE | psySF_RENDER,
	psySF_IMPORTER							= psySF_TOOL | psySF_CONSOLE,
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

	std::string		Name_;
	BcU32			Flags_;
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

/**
 * Called to launch game once scene has initialised.
 */
extern void PsyLaunchGame();


#endif
