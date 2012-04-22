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

#include "GaElementComponent.h"
#include "GaStrongForceComponent.h"
#include "GaGameComponent.h"
#include "GaSunComponent.h"
#include "GaCameraComponent.h"


//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Ludum Dare 23: Tiny World: \"Fusion Time!\"", psySF_GAME_DEV, 1.0f / 60.0f );	

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
	CsCore::pImpl()->registerResource< GaElementComponent >();
	CsCore::pImpl()->registerResource< GaStrongForceComponent >();
	CsCore::pImpl()->registerResource< GaGameComponent >();
	CsCore::pImpl()->registerResource< GaSunComponent >();
	CsCore::pImpl()->registerResource< GaCameraComponent >();
}

//////////////////////////////////////////////////////////////////////////
// PsyGameUnRegisterResources
void PsyGameUnRegisterResources()
{
	CsCore::pImpl()->unregisterResource< GaElementComponent >();
	CsCore::pImpl()->unregisterResource< GaStrongForceComponent >();
	CsCore::pImpl()->unregisterResource< GaGameComponent >();
	CsCore::pImpl()->unregisterResource< GaSunComponent >();
	CsCore::pImpl()->unregisterResource< GaCameraComponent >();
}
