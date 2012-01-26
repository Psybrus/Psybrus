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

#include "BcAABB.h"
#include "BcAtomicMutex.h"
#include "BcDebug.h"
#include "BcDelegate.h"
#include "BcEndian.h"
#include "BcFile.h"
#include "BcFixed.h"
#include "BcGlobal.h"
#include "BcHash.h"
#include "BcMat3d.h"
#include "BcMat4d.h"
#include "BcMath.h"
#include "BcMemory.h"
#include "BcName.h"
#include "BcPlane.h"
#include "BcPortability.h"
#include "BcQuat.h"
#include "BcRandom.h"
#include "BcScopedLock.h"
#include "BcStream.h"
#include "BcString.h"
#include "BcTypes.h"
#include "BcUTF8.h"
#include "BcVectors.h"

#include "SysKernel.h"

#include "EvtBinding.h"
#include "EvtEvent.h"
#include "EvtPublisher.h"

#include "CsCore.h"
#include "FsCore.h"
#include "OsCore.h"
#include "RmCore.h"
#include "RsCore.h"
#include "SsCore.h"
#include "ScnCore.h"

#include "ScnComponent.h"
#include "ScnEntity.h"
#include "ScnTexture.h"
#include "ScnTextureAtlas.h"
#include "ScnRenderTarget.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnCanvasComponent.h"
#include "ScnShader.h"
#include "ScnFont.h"
#include "ScnSound.h"
#include "ScnSoundEmitter.h"
#include "ScnViewComponent.h"
#include "ScnRigidBodyWorld.h"
#include "ScnRigidBody.h"

#include "SysEvents.h"
#include "SysKernel.h"
#include "SysState.h"

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
					BcReal TickRate = 1.0f / 60.0f ):
		Name_( Name ),
		Flags_( Flags ),
		TickRate_( TickRate )
	{
	}

	BcU32			Flags_;
	std::string		Name_;
	BcReal			TickRate_;
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
 * Called when user resources need to be registered.
 */
extern void PsyGameRegisterResources();

/**
 * Called when user resources need to be unregistered.
 */
extern void PsyGameUnRegisterResources();

#endif
