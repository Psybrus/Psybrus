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

#include "GaMainGameState.h"

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
	// Wait for default material to be ready.
	if( ScnMaterial::Default->isReady() == BcTrue )
	{
		// Create default material instance.
		ScnMaterial::Default->createInstance( "DefaultMaterialInstance", ScnMaterialInstance::Default, BcErrorCode );

		// Spawn main game state.
		spawnSubState( 0, new GaMainGameState() );

		return sysSR_FINISHED;
	}

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
	// Allocate a frame to render.
	RsFrame* pFrame = RsCore::pImpl()->allocateFrame();

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

////////////////////////////////////////////////////////////////////////////////
// addState
void GaTopState::addState( GaBaseGameState* pState )
{
	StateList_.push_back( pState );
}

////////////////////////////////////////////////////////////////////////////////
// removeState
void GaTopState::removeState( GaBaseGameState* pState )
{
	for( TStateList::iterator Iter( StateList_.begin() ); Iter != StateList_.end(); ++Iter )
	{
		if( (*Iter) == pState )
		{
			StateList_.erase( Iter );
			break;
		}
	}
}
