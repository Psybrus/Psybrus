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

// Include this first.
#include "Base/BcReflection.h"
#include "Base/BcReflectionSerialise.h"

#include "Base/BcAABB.h"
#include "Base/BcAtomicMutex.h"
#include "Base/BcBSPTree.h"
#include "Base/BcDebug.h"
#include "Base/BcDelegate.h"
#include "Base/BcEndian.h"
#include "Base/BcFile.h"
#include "Base/BcFixed.h"
#include "Base/BcFixedVectors.h"
#include "Base/BcGlobal.h"
#include "Base/BcLog.h"
#include "Base/BcLogFile.h"
#include "Base/BcHash.h"
#include "Base/BcMat3d.h"
#include "Base/BcMat4d.h"
#include "Base/BcMath.h"
#include "Base/BcMemory.h"
#include "Base/BcName.h"
#include "Base/BcPlane.h"
#include "Base/BcPortability.h"
#include "Base/BcRegex.h"
#include "Base/BcQuat.h"
#include "Base/BcRandom.h"
#include "Base/BcScopedLock.h"
#include "Base/BcStream.h"
#include "Base/BcString.h"
#include "Base/BcTypes.h"
#include "Base/BcUTF8.h"
#include "Base/BcVectors.h"

#include "System/SysEvents.h"
#include "System/SysKernel.h"
#include "System/SysState.h"

#include "Events/EvtBinding.h"
#include "Events/EvtEvent.h"
#include "Events/EvtPublisher.h"
#include "Events/EvtProxyBuffered.h"

#include "System/Content/CsCore.h"
#include "System/File/FsCore.h"
#include "System/Os/OsCore.h"
#include "System/Renderer/RsCore.h"
#include "System/Sound/SsCore.h"
#include "System/Scene/ScnCore.h"

#include "System/Scene/Animation/ScnAnimation.h"
#include "System/Scene/Animation/ScnAnimationComponent.h"
#include "System/Scene/Animation/ScnAnimationTreeNode.h"
#include "System/Scene/Animation/ScnAnimationTreeBlendNode.h"
#include "System/Scene/Animation/ScnAnimationTreeTrackNode.h"

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/ScnDebugRenderComponent.h"
#include "System/Scene/ScnTexture.h"
#include "System/Scene/ScnTextureAtlas.h"
#include "System/Scene/ScnRenderTarget.h"
#include "System/Scene/ScnLightComponent.h"
#include "System/Scene/ScnMaterial.h"
#include "System/Scene/ScnModel.h"
#include "System/Scene/ScnParticleSystemComponent.h"
#include "System/Scene/ScnCanvasComponent.h"
#include "System/Scene/ScnShader.h"
#include "System/Scene/ScnFont.h"
#include "System/Scene/ScnSound.h"
#include "System/Scene/ScnSoundEmitter.h"
#include "System/Scene/ScnSoundListenerComponent.h"
#include "System/Scene/ScnViewComponent.h"

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

/**
 * Called when user resources need to be registered.
 */
extern void PsyGameRegisterResources();

/**
 * Called when user resources need to be unregistered.
 */
extern void PsyGameUnRegisterResources();

#endif
