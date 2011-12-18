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

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Leave My Lettuce Alone :(", psySF_GAME_DEV, 1.0f / 60.0f );	


//////////////////////////////////////////////////////////////////////////
// OnUpdate
eEvtReturn OnUpdate( EvtID ID, const SysSystemEvent& Event )
{
	GaTopState::pImpl()->process();
	
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// OnRender
eEvtReturn OnRender( EvtID ID, const SysSystemEvent& Event )
{
	//GaTopState::pImpl()->process();
	return evtRET_PASS;
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
}

//////////////////////////////////////////////////////////////////////////
// DEMO CODE.


