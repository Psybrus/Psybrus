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

	spawnEntity( new GaPlayerEntity( Projection_ ) );
	spawnEntity( new GaSwarmEntity( Projection_ ) );
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
	BcReal Tick = 1.0f / 60.0f;

	// Do spawn/kill entities.
	for( BcU32 Idx = 0; Idx < SpawnEntities_.size(); ++Idx )
	{
		Entities_.push_back( SpawnEntities_[ Idx ] );
	}
	SpawnEntities_.clear();

	for( BcU32 Idx = 0; Idx < KillEntities_.size(); ++Idx )
	{
		for( TEntityListIterator It( Entities_.begin() ); It != Entities_.end(); ++It )
		{
			if( (*It) == KillEntities_[ Idx ] )
			{
				Entities_.erase( It );
				break;
			}
		}
	}
	SpawnEntities_.clear();

	// Update entities.
	for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
	{
		GaEntity* pEntity = Entities_[ Idx ];
		pEntity->update( Tick );
	}

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

	// Render entities.
	for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
	{
		GaEntity* pEntity = Entities_[ Idx ];
		pEntity->render( Canvas_ );
	}

	// Base render.
	GaBaseGameState::render( pFrame );
}

////////////////////////////////////////////////////////////////////////////////
// spawnEntity
void GaMainGameState::spawnEntity( GaEntity* pEntity )
{
	SpawnEntities_.push_back( pEntity );
	pEntity->setParent( this );
}

////////////////////////////////////////////////////////////////////////////////
// killEntity
void GaMainGameState::killEntity( GaEntity* pEntity )
{
	KillEntities_.push_back( pEntity );
	pEntity->setParent( this );
}