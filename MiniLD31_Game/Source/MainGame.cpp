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

#include "GaWorldComponent.h"
#include "GaPlayerComponent.h"
#include "GaMonsterComponent.h"
#include "GaTitleComponent.h"

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Into the Abyss", psySF_GAME_DEV, 1.0f / 60.0f );	

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
	ScnTransform Transform;
	BcQuat Rotation;
	Rotation.fromEular( 0.0f, BcPI, 0.0f );
	Transform.setTranslation( BcVec3d( 10.0f, 10.0f, 10.0f ) );
	Transform.setRotation( Rotation );
	
	BcMat4d A, B;
	
	Transform.getMatrix( A );
	Transform.getInvertedMatrix( B );
	
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
	CsCore::pImpl()->registerResource< GaWorldComponent >( BcFalse );
	CsCore::pImpl()->registerResource< GaPlayerComponent >( BcFalse );
	CsCore::pImpl()->registerResource< GaMonsterComponent >( BcFalse );
	CsCore::pImpl()->registerResource< GaTitleComponent >( BcFalse );
}

//////////////////////////////////////////////////////////////////////////
// PsyGameUnRegisterResources
void PsyGameUnRegisterResources()
{
	CsCore::pImpl()->unregisterResource< GaWorldComponent >();
	CsCore::pImpl()->unregisterResource< GaPlayerComponent >();
	CsCore::pImpl()->unregisterResource< GaMonsterComponent >();
	CsCore::pImpl()->unregisterResource< GaTitleComponent >();
}
