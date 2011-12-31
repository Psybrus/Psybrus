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

#include "GaGameComponent.h"
#include "GaPlayerComponent.h"
#include "GaSwarmComponent.h"
#include "GaFoodComponent.h"

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Leave My Lettuce Alone :(", psySF_GAME_DEV, 1.0f / 60.0f );	

//////////////////////////////////////////////////////////////////////////
// OnUpdate
eEvtReturn OnUpdate( EvtID ID, const SysSystemEvent& Event )
{
	// If processing has completed stop the kernel.
	if( GaTopState::pImpl()->process() )
	{
		delete GaTopState::pImpl();
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
	CsCore::pImpl()->registerResource< GaGameComponent >( BcFalse );
	CsCore::pImpl()->registerResource< GaPlayerComponent >( BcFalse );
	CsCore::pImpl()->registerResource< GaSwarmComponent >( BcFalse );
	CsCore::pImpl()->registerResource< GaFoodComponent >( BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// PsyGameUnRegisterResources
void PsyGameUnRegisterResources()
{
	CsCore::pImpl()->unregisterResource< GaGameComponent >();
	CsCore::pImpl()->unregisterResource< GaPlayerComponent >();
	CsCore::pImpl()->unregisterResource< GaSwarmComponent >();
	CsCore::pImpl()->unregisterResource< GaFoodComponent >();
}
