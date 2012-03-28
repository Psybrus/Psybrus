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

#include "GaMatchmakingState.h"

#include "GaTopState.h"

#include "GaGameComponent.h"

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
	ResourceList_.push_back( CsResourceRef<>( ScnMaterial::Default ) );

	CsCore::pImpl()->requestResource( "background", BackgroundMaterial_ );
	CsCore::pImpl()->requestResource( "spritesheet0", SpriteSheetMaterial0_ );
	CsCore::pImpl()->requestResource( "spritesheet1", SpriteSheetMaterial1_ );
	CsCore::pImpl()->requestResource( "hud", HUDMaterial_ );

	ScnSoundRef Sound;

	CsCore::pImpl()->requestResource( "ArrowHit", Sound );
	SoundMap_[ (*Sound->getName()).c_str() ] = Sound;
	CsCore::pImpl()->requestResource( "ArrowLaunch", Sound );
	SoundMap_[ (*Sound->getName()).c_str() ] = Sound;
	CsCore::pImpl()->requestResource( "RockHit", Sound );
	SoundMap_[ (*Sound->getName()).c_str() ] = Sound;
	CsCore::pImpl()->requestResource( "RockLaunch", Sound );
	SoundMap_[ (*Sound->getName()).c_str() ] = Sound;
	CsCore::pImpl()->requestResource( "Die", Sound );
	SoundMap_[ (*Sound->getName()).c_str() ] = Sound;
	CsCore::pImpl()->requestResource( "Walk", Sound );
	SoundMap_[ (*Sound->getName()).c_str() ] = Sound;

	spawnChildState( new GaMatchmakingState() );
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	BcBool Ready = BcTrue;

	Ready &= BackgroundMaterial_.isReady();
	Ready &= SpriteSheetMaterial0_.isReady();
	Ready &= SpriteSheetMaterial1_.isReady();
	Ready &= HUDMaterial_.isReady();

	for( BcU32 Idx = 0; Idx < ResourceList_.size(); ++Idx )
	{
		Ready &= ResourceList_[ Idx ].isReady();
	}

	for( TSoundMap::iterator It( SoundMap_.begin() ); It != SoundMap_.end(); ++It )
	{
		Ready &= (*It).second.isReady();
	}
	
	// Wait for default material to be ready.
	if( Ready == BcTrue )
	{
		//
		return sysSR_FINISHED;
	}

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaTopState::preMain()
{ 
	ScnEntityRef Entity;

	// Create model entity.
	/*
	if( CsCore::pImpl()->createResource( "ModelEntity_0", Entity ) )
	{
		GaGameComponentRef ExampleComponent;
		ScnModelComponentRef ModelComponent;
	
		// Create component resources.
		CsCore::pImpl()->createResource( BcName::INVALID, ExampleComponent );
		CsCore::pImpl()->createResource( BcName::INVALID, ModelComponent, ScnModel::Default );

		// Attach components.
		Entity->attach( ExampleComponent );
		Entity->attach( ModelComponent );

		EntityList_.push_back( Entity );
	}
	*/
	
	// Create view entity.
	if( CsCore::pImpl()->createResource( "ViewEntity_0", Entity ) )
	{
		ScnMaterialComponentRef MaterialComponent;
		ScnViewComponentRef ViewComponent;
		ScnCanvasComponentRef CanvasComponent;
		GaGameComponentRef GameComponent0;
		GaGameComponentRef GameComponent1;
		
		// Create component resources.
		CsCore::pImpl()->createResource( BcName::INVALID, ViewComponent, 0.0f, 0.0f, 1.0f, 1.0f, 0.1f, 1000.0f, BcPIDIV4, 0.0f );
		CsCore::pImpl()->createResource( BcName::INVALID, MaterialComponent, SpriteSheetMaterial0_, BcErrorCode );
		CsCore::pImpl()->createResource( BcName::INVALID, CanvasComponent, 1024 * 32, MaterialComponent );
		CsCore::pImpl()->createResource( BcName::INVALID, GameComponent0, GaMatchmakingState::getClientID() );
 
		// Attach components.
		Entity->attach( ViewComponent );
		Entity->attach( MaterialComponent );
		Entity->attach( CanvasComponent );
		Entity->attach( GameComponent0 );
		//Entity->attach( GameComponent1 );

		// Setup entity position to render from.
		BcMat4d LookAt;
		LookAt.lookAt( BcVec3d( -10.0f, 10.0f, -10.0f ), BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, 1.0f, 0.0f ) );
		LookAt.inverse();
		Entity->setMatrix( LookAt );		

		EntityList_.push_back( Entity );
	}

	// Add entities to scene.
	for( BcU32 Idx = 0; Idx < EntityList_.size(); ++Idx )
	{
		ScnCore::pImpl()->addEntity( EntityList_[ Idx ] );
	}
}

////////////////////////////////////////////////////////////////////////////////
// main
eSysStateReturn GaTopState::main()
{

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preLeave
void GaTopState::preLeave()
{
	// Remove entities from the scene.
	for( BcU32 Idx = 0; Idx < EntityList_.size(); ++Idx )
	{
		ScnCore::pImpl()->removeEntity( EntityList_[ Idx ] );
	}

	ResourceList_.clear();
	EntityList_.clear();
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
// playSound
void GaTopState::playSound( const BcChar* pSoundName, const BcFixedVec2d& Position )
{
	TSoundMap::iterator It( SoundMap_.find( pSoundName ) );

	if( It != SoundMap_.end() )
	{
		ScnSoundRef Sound( (*It).second );

		if( SsCore::pImpl() )
		{
			SsChannel* pChannel = SsCore::pImpl()->play( Sound->getSample(), NULL );
			if( pChannel != NULL )
			{
				pChannel->position( BcVec3d( Position.x(), -Position.y(), 2.0f ) / 800.0f );
				pChannel->gain( 0.2f );
				pChannel->pitch( BcAbs( BcRandom::Global.randReal() * 0.1f ) + 0.95f );
			}
		}
	}
}
