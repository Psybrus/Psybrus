/**************************************************************************
*
* File:		GaTopState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Game top state.
*		
*
*
* 
**************************************************************************/

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaTopState::GaTopState()
{
	name( "GaTopState" );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaTopState::~GaTopState()
{
}

////////////////////////////////////////////////////////////////////////////////
// enterOnce
void GaTopState::enterOnce()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	/*
	BcBool Ready = BcTrue;

	for( BcU32 Idx = 0; Idx < MATERIAL_MAX; ++Idx )
	{
		Ready &= Materials_[ Idx ].isReady();
	}

	for( BcU32 Idx = 0; Idx < SOUND_MAX; ++Idx )
	{
		Ready &= Sounds_[ Idx ].isReady();
	}

	// Wait for default material to be ready.
	if( ScnMaterial::Default->isReady() == BcTrue &&
		Ready )
	{
		// Create default material instance.
		ScnMaterial::Default->createInstance( "DefaultMaterialInstance", ScnMaterialInstance::Default, BcErrorCode );

		// Spawn main game state.
		spawnSubState( 0, new GaMainGameState() );

		return sysSR_FINISHED;
	}
	*/

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaTopState::preMain()
{
}

////////////////////////////////////////////////////////////////////////////////
// main
eSysStateReturn GaTopState::main()
{
	/*
	// Render to all clients.
	for( BcU32 Idx = 0; Idx < OsCore::pImpl()->getNoofClients(); ++Idx )
	{
		// Grab client.
		OsClient* pClient = OsCore::pImpl()->getClient( Idx );

		// Get context.
		RsContext* pContext = RsCore::pImpl()->getContext( pClient );

		// Allocate a frame to render using default context.
		RsFrame* pFrame = RsCore::pImpl()->allocateFrame( pContext );

		// Render all registered states.
		for( TStateList::iterator Iter( StateList_.begin() ); Iter != StateList_.end(); ++Iter )
		{
			GaBaseGameState* pState = (*Iter);
	
			if( pState->internalStage() == sysBS_MAIN )
			{
				pState->render( pFrame );
			}
		}
		
		// Queue frame for render.
		RsCore::pImpl()->queueFrame( pFrame );
	}
	*/

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preLeave
void GaTopState::preLeave()
{
}

////////////////////////////////////////////////////////////////////////////////
// leave
eSysStateReturn GaTopState::leave()
{
	return sysSR_FINISHED;
}

////////////////////////////////////////////////////////////////////////////////
// leaveOnce
void GaTopState::leaveOnce()
{

}
