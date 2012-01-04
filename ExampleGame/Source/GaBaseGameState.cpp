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
	// TODO: Support finding without ID.
	if( CsCore::pImpl()->findResource( "CameraEntity_0", CameraEntity_ ) )
	{
		for( BcU32 Idx = 0; Idx < CameraEntity_->getNoofComponents(); ++Idx )
		{
			if( CameraEntity_->getComponent( Idx )->isTypeOf< ScnCanvasComponent >() )
			{
				Canvas_ = CameraEntity_->getComponent( Idx );
				break;
			}
		}
	}
	BcAssert( Canvas_.isValid() );
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
void GaBaseGameState::render()
{
	
}
