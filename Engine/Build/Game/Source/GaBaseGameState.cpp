/**************************************************************************
*
* File:		GaBaseGameState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Base game state.
*		
*
*
* 
**************************************************************************/

#include "GaBaseGameState.h"

#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaBaseGameState::GaBaseGameState()
{
	GaTopState::pImpl()->addState( this );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaBaseGameState::~GaBaseGameState()
{
	GaTopState::pImpl()->removeState( this );
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaBaseGameState::render( RsFrame* pFrame )
{
}
