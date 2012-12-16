/**************************************************************************
*
* File:		MainGame.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main Game Entrypoint.
*		
*
*
* 
**************************************************************************/

#include "Psybrus.h"

#include "GaTopState.h"

#include "GaCameraComponent.h"
#include "GaEnemyComponent.h"
#include "GaGameInfoComponent.h"
#include "GaGameStateComponent.h"
#include "GaLevelComponent.h"
#include "GaPlayerComponent.h"
#include "GaPortaudioComponent.h"
#include "GaProjectileComponent.h"

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Crazy Dr. Banana's pursuit of The Goat of Good", psySF_GAME_DEV, 1.0f / 60.0f );	

//////////////////////////////////////////////////////////////////////////
// OnUpdate
eEvtReturn OnUpdate( EvtID ID, const SysSystemEvent& Event )
{
	// If processing has completed stop the kernel.
	if( GaTopState::pImpl()->process() )
	{
		// Free topstate.
		delete GaTopState::pImpl();

		// Stop kernel.
		SysKernel::pImpl()->stop();

		return evtRET_REMOVE;
	}
	
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// OnQuit
eEvtReturn OnQuit( EvtID ID, const OsEventCore& Event )
{
	GaTopState::pImpl()->leaveState();
	
	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// PsyGameInit
void PsyGameInit()
{
	// Create a new game top state.
	new GaTopState();

	// Subscribe to update.
	SysSystemEvent::Delegate OnUpdateDelegate = SysSystemEvent::Delegate::bind< OnUpdate >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_UPDATE, OnUpdateDelegate );

	// Subscribe to quit.
	OsEventCore::Delegate OnQuitDelegate = OsEventCore::Delegate::bind< OnQuit >();
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT, OnQuitDelegate );
}

//////////////////////////////////////////////////////////////////////////
// PsyGameRegisterResources
void PsyGameRegisterResources()
{
	CsCore::pImpl()->registerResource< GaCameraComponent >();
	CsCore::pImpl()->registerResource< GaEnemyComponent >();
	CsCore::pImpl()->registerResource< GaGameInfoComponent >();
	CsCore::pImpl()->registerResource< GaGameStateComponent >();
	CsCore::pImpl()->registerResource< GaLevelComponent >();
	CsCore::pImpl()->registerResource< GaPlayerComponent >();
	CsCore::pImpl()->registerResource< GaPortaudioComponent >();
	CsCore::pImpl()->registerResource< GaProjectileComponent >();
}

//////////////////////////////////////////////////////////////////////////
// PsyGameUnRegisterResources
void PsyGameUnRegisterResources()
{
	CsCore::pImpl()->unregisterResource< GaCameraComponent >();
	CsCore::pImpl()->unregisterResource< GaEnemyComponent >();
	CsCore::pImpl()->unregisterResource< GaGameInfoComponent >();
	CsCore::pImpl()->unregisterResource< GaGameStateComponent >();
	CsCore::pImpl()->unregisterResource< GaLevelComponent >();
	CsCore::pImpl()->unregisterResource< GaPlayerComponent >();
	CsCore::pImpl()->unregisterResource< GaPortaudioComponent >();
	CsCore::pImpl()->unregisterResource< GaProjectileComponent >();
}
