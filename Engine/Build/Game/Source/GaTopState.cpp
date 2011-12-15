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
	ScnMaterialRef MaterialRef;

	if( CsCore::pImpl()->importResource( "EngineContent/default.material", MaterialRef ) )
	{
		//
		int a = 0; ++a;
	}
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	return sysSR_FINISHED;
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

		pState->render( pFrame );
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
