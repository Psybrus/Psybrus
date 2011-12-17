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
	name( "GaBaseGameState" );

	GaTopState::pImpl()->addState( this );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaBaseGameState::~GaBaseGameState()
{
	GaTopState::pImpl()->removeState( this );
}

////////////////////////////////////////////////////////////////////////////////
// enterOnce
//virtual
void GaBaseGameState::enterOnce()
{
	if( CsCore::pImpl()->createResource( "Canvas", Canvas_, 4096, ScnMaterialInstance::Default ) )
	{
		int a = 0; ++a;
	}
}

////////////////////////////////////////////////////////////////////////////////
// enter
//virtual
eSysStateReturn GaBaseGameState::enter()
{
	if( Canvas_.isReady() == BcFalse )
	{
		return sysSR_CONTINUE;
	}

	return sysSR_FINISHED;
}

////////////////////////////////////////////////////////////////////////////////
// render
void GaBaseGameState::render( RsFrame* pFrame )
{
	RsViewport Viewport( 0, 0, 640, 480 );

	pFrame->setRenderTarget( NULL );
	pFrame->setViewport( Viewport );

	Canvas_->clear();
	Canvas_->drawLine( BcVec2d( -1.0f, -1.0f ), BcVec2d( 1.0f, 1.0f ), RsColour::WHITE, 0 );

	Canvas_->render( pFrame, RsRenderSort( 0 ) );
}
