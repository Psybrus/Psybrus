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

#include "GaTopState.h"

#include "GaPlayerEntity.h"
#include "GaSwarmEntity.h"
#include "GaFoodEntity.h"

#include "GaBunnyRenderer.h"

#include "GaOverlayState.h"

static BcU32 Level = 0;

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaMainGameState::GaMainGameState()
{
	pContext_ = NULL;	
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

	WorldHalfSize_.set( 320.0f, 240.0f );

	//Level = 5;

	spawnEntity( new GaPlayerEntity() );
	spawnEntity( new GaSwarmEntity( Level ) );

	switch( Level )
	{
	default:
	case 6:
		spawnEntity( new GaFoodEntity( BcVec2d( -256.0f,   0.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d( -128.0f,   0.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(    0.0f,   0.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(  128.0f,   0.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(  256.0f,   0.0f ) ) );

	case 5:
	case 4:
		spawnEntity( new GaFoodEntity( BcVec2d( -256.0f,   128.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d( -128.0f,   128.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(    0.0f,   128.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(  128.0f,   128.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(  256.0f,   128.0f ) ) );

	case 3:
	case 2:
		spawnEntity( new GaFoodEntity( BcVec2d( -256.0f,  -128.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(  256.0f,  -128.0f ) ) );

	case 1:
	case 0:
		spawnEntity( new GaFoodEntity( BcVec2d( -128.0f,  -128.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(    0.0f,  -128.0f ) ) );
		spawnEntity( new GaFoodEntity( BcVec2d(  128.0f,  -128.0f ) ) );
	}

	ScnMaterialRef Material;
	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_BACKGROUND, Material );
	CsCore::pImpl()->createResource( BcName::INVALID, BackgroundMaterialComponent_, Material, scnSPF_DEFAULT );

	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_FOREGROUND, Material );
	CsCore::pImpl()->createResource( BcName::INVALID, ForegroundMaterialComponent_, Material, scnSPF_DEFAULT );

	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_BAR, Material );
	CsCore::pImpl()->createResource( BcName::INVALID, BarMaterialComponent_, Material, scnSPF_DEFAULT );

	// Get default render context.
	pContext_ = RsCore::pImpl()->getContext( NULL );
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
	if( Level == 0 )
	{
		ScnMaterialRef Material;
		GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_SPLASH, Material );
		spawnChildState( new GaOverlayState( Material, "", NULL ) );	
	}
}

////////////////////////////////////////////////////////////////////////////////
// main
//virtual
eSysStateReturn GaMainGameState::main()
{
	BcReal Tick = SysKernel::pImpl()->getFrameTime();

	spawnKill();

	// Update entities.
	for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
	{
		GaEntity* pEntity = Entities_[ Idx ];
		pEntity->update( Tick );
	}

	// Check for win/lose condition.
	BcReal MaxFoodHealth = 0.0f;
	BcReal TotalFoodHealth = 0.0f;
	for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
	{
		GaFoodEntity* pFood = dynamic_cast< GaFoodEntity* >( Entities_[ Idx ] );

		if( pFood != NULL )
		{
			MaxFoodHealth += 1.0f;			
			TotalFoodHealth += pFood->getHealthFraction();
		}
	}

	// Cache for rendering.
	FoodHealth_ = TotalFoodHealth / MaxFoodHealth;

	// If more than half of the food is deaded, we lose.
	if( FoodHealth_ < 0.5f )
	{
		doLose();
	}

	GaSwarmEntity* pSwarm = getEntity< GaSwarmEntity >( 0 );

	// If the swarm is dead, you win.
	if( pSwarm->isAlive() == BcFalse )
	{
		doWin();
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
	// Free all entities.
	for( BcU32 Idx = 0; Idx < SpawnEntities_.size(); ++Idx )
	{
		delete SpawnEntities_[ Idx ];
	}

	for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
	{
		delete Entities_[ Idx ];
	}

	for( BcU32 Idx = 0; Idx < KillEntities_.size(); ++Idx )
	{
		delete KillEntities_[ Idx ];
	}
}

////////////////////////////////////////////////////////////////////////////////
// render
//virtual
void GaMainGameState::render( RsFrame* pFrame )
{
	
	RsViewport Viewport( 0, 0, pFrame->getContext()->getWidth(), pFrame->getContext()->getHeight() );

	// Setup frame.
	pFrame->setRenderTarget( NULL );
	pFrame->setViewport( Viewport );

	// Setup canvas.
	Canvas_->clear();

	// Render background.
	Projection_.perspProjection( BcPIDIV4, (BcReal)pFrame->getContext()->getWidth() / (BcReal)pFrame->getContext()->getHeight(), 1.0f, 1024.0f );
	WorldView_.lookAt( BcVec3d( 0.0f, 350.0f, 270.0f ), BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, 0.0f, 1.0f ) );

	if( SsCore::pImpl() != NULL )
	{
		SsCore::pImpl()->setListener( BcVec3d( 0.0f, 350.0f, 270.0f ), BcVec3d( 0.0f, -4.0f, -2.0f ).normal(), BcVec3d( 0.0f, 0.0f, 1.0f  ) );
	}

	setMaterialComponentParams( BackgroundMaterialComponent_, BcMat4d() );
	Canvas_->setMaterialComponent( BackgroundMaterialComponent_ );
	Canvas_->drawSpriteCentered3D( BcVec3d( 0.0f, 0.0f, 0.0f ), WorldHalfSize_ * 4.0f, 0, RsColour::WHITE, 0 );

	// Render entities.
	for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
	{
		GaEntity* pEntity = Entities_[ Idx ];
		pEntity->render( Canvas_ );
	}

	// Draw foreground.
	BcMat4d Ortho;
	Ortho.orthoProjection( -WorldHalfSize_.x(), WorldHalfSize_.x(), -WorldHalfSize_.y(), WorldHalfSize_.y(), -1.0f, 0.0f );

	Canvas_->pushMatrix( Ortho );
	Canvas_->setMaterialComponent( ForegroundMaterialComponent_ );
	Canvas_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( WorldHalfSize_.x() * 2.2f, WorldHalfSize_.y() * -2.0f ), 0, RsColour::WHITE, 20 );

	BcReal Width = BcMax( 0.0f, FoodHealth_ - 0.5f ) * 2.0f;

	RsColour Colour;
	Colour.lerp( RsColour::RED, RsColour::GREEN, Width );

	Canvas_->setMaterialComponent( BarMaterialComponent_ );
	Canvas_->drawSpriteCentered( 
		BcVec2d( 0.0f, WorldHalfSize_.y() - ( WorldHalfSize_.y() * 0.05f ) ), 
		BcVec2d( WorldHalfSize_.x() * 1.5f * Width, WorldHalfSize_.y() * 0.05f ), 
		0, Colour, 20 );

	Canvas_->popMatrix();

	// Base render.
	GaBaseGameState::render( pFrame );
}

////////////////////////////////////////////////////////////////////////////////
// setMaterialComponentParams
void GaMainGameState::setMaterialComponentParams( ScnMaterialComponentRef MaterialComponentRef, const BcMat4d& Transform )
{
	// TODO: Optimise this.
	BcU32 ClipTransform = MaterialComponentRef->findParameter( "uClipTransform" );
	BcU32 WorldTransform = MaterialComponentRef->findParameter( "uWorldTransform" );
	MaterialComponentRef->setParameter( ClipTransform, WorldView_ * Projection_ );
	MaterialComponentRef->setParameter( WorldTransform, Transform );
}

////////////////////////////////////////////////////////////////////////////////
// getWorldPosition
void GaMainGameState::getWorldPosition( const BcVec2d& ScreenPosition, BcVec3d& Near, BcVec3d& Far )
{
	if( pContext_ != NULL )
	{
		RsViewport Viewport( 0, 0, pContext_->getWidth(), pContext_->getHeight() );
		Viewport.projection( Projection_ );
		Viewport.view( WorldView_ );
		Viewport.unProject( ScreenPosition, Near, Far );
	}
}

////////////////////////////////////////////////////////////////////////////////
// spawnEntity
void GaMainGameState::spawnEntity( GaEntity* pEntity )
{
	SpawnEntities_.push_back( pEntity );
	pEntity->setParent( this );
	pEntity->setProjection( Projection_ );
}

////////////////////////////////////////////////////////////////////////////////
// killEntity
void GaMainGameState::killEntity( GaEntity* pEntity )
{
	KillEntities_.push_back( pEntity );
}

////////////////////////////////////////////////////////////////////////////////
// spawnKill
void GaMainGameState::spawnKill()
{
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
}

////////////////////////////////////////////////////////////////////////////////
// doWin
void GaMainGameState::doWin()
{
	ScnMaterialRef Material;
	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_WIN, Material );
	spawnChildState( new GaOverlayState( Material, "", new GaMainGameState() ) );
	++Level;
}

////////////////////////////////////////////////////////////////////////////////
// doLose
void GaMainGameState::doLose()
{
	ScnMaterialRef Material;
	GaTopState::pImpl()->getMaterial( GaTopState::MATERIAL_LOSE, Material );
	spawnChildState( new GaOverlayState( Material, "", new GaMainGameState() ) );
}



