/**************************************************************************
*
* File:		GaMainGameState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main game state.
*		
*
*
* 
**************************************************************************/

#include "GaMainGameState.h"

#include "GaPlayerEntity.h"
#include "GaSwarmEntity.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaMainGameState::GaMainGameState()
{

}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaMainGameState::~GaMainGameState()
{

}

////////////////////////////////////////////////////////////////////////////////
// enterOnce
//virtual
void GaMainGameState::enterOnce()
{
	GaBaseGameState::enterOnce();

	Projection_.orthoProjection( -320.0f, 320.0f, -240.0f, 240.0f, -1.0f, 1.0f );

	pPlayer_ = new GaPlayerEntity( Projection_ );
	pSwarm_ = new GaSwarmEntity( Projection_ );
}

////////////////////////////////////////////////////////////////////////////////
// enter
//virtual
eSysStateReturn GaMainGameState::enter()
{
	return GaBaseGameState::enter();
}

////////////////////////////////////////////////////////////////////////////////
// preMain
//virtual
void GaMainGameState::preMain()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// main
//virtual
eSysStateReturn GaMainGameState::main()
{
	pPlayer_->update( 1.0f / 60.0f );


	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// leave
//virtual
eSysStateReturn GaMainGameState::leave()
{
	return sysSR_FINISHED;
}

////////////////////////////////////////////////////////////////////////////////
// leaveOnce
//virtual
void GaMainGameState::leaveOnce()
{

}

////////////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaMainGameState::render( RsFrame* pFrame )
{
	// Setup frame.
	pFrame->setRenderTarget( NULL );
	pFrame->setViewport( RsViewport( 0, 0, GResolutionWidth, GResolutionHeight ) );

	// Setup canvas.
	Canvas_->clear();
	Canvas_->pushMatrix( Projection_ );

	// Render player.
	pPlayer_->render( Canvas_ );

	// Base render.
	GaBaseGameState::render( pFrame );
}

