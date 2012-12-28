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

#include "GaEnemyComponent.h"
#include "GaPawnComponent.h"
#include "GaPlayerComponent.h"
#include "GaPlayerSoundComponent.h"
#include "GaWorldInfoComponent.h"
#include "GaWorldBSPComponent.h"
#include "GaWorldPressureComponent.h"


//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "7DFPS Game", psySF_GAME_DEV, 1.0f / 60.0f );	

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
	CsCore::pImpl()->registerResource< GaWorldInfoComponent >();
	CsCore::pImpl()->registerResource< GaWorldBSPComponent >();
	CsCore::pImpl()->registerResource< GaWorldPressureComponent >();
	CsCore::pImpl()->registerResource< GaEnemyComponent >();
	CsCore::pImpl()->registerResource< GaPlayerComponent >();
	CsCore::pImpl()->registerResource< GaPlayerSoundComponent >();
	CsCore::pImpl()->registerResource< GaPawnComponent >();
}

//////////////////////////////////////////////////////////////////////////
// PsyGameUnRegisterResources
void PsyGameUnRegisterResources()
{
	CsCore::pImpl()->unregisterResource< GaWorldInfoComponent >();
	CsCore::pImpl()->unregisterResource< GaWorldBSPComponent >();
	CsCore::pImpl()->unregisterResource< GaWorldPressureComponent >();
	CsCore::pImpl()->unregisterResource< GaEnemyComponent >();
	CsCore::pImpl()->unregisterResource< GaPlayerComponent >();
	CsCore::pImpl()->unregisterResource< GaPlayerSoundComponent >();
	CsCore::pImpl()->unregisterResource< GaPawnComponent >();
}
