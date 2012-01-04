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
	RmCore::pImpl()->connect( "127.0.0.1" );

	// NEILO HACKY CODE :(
	BcBool Imported;
#if 0 && PSY_SERVER
	Imported = CsCore::pImpl()->importResource( "GameContent/bunny.ScnMaterial", Materials_[ MATERIAL_BUNNY ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/kitty.ScnMaterial", Materials_[ MATERIAL_KITTY ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/lettuce.ScnMaterial", Materials_[ MATERIAL_LETTUCE ] );
	BcAssert( Imported ); 

	Imported = CsCore::pImpl()->importResource( "GameContent/background.ScnMaterial", Materials_[ MATERIAL_BACKGROUND ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/foreground.ScnMaterial", Materials_[ MATERIAL_FOREGROUND ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/bar.ScnMaterial", Materials_[ MATERIAL_BAR ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/emote.ScnMaterial", Materials_[ MATERIAL_EMOTE ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/splash.ScnMaterial", Materials_[ MATERIAL_SPLASH ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/win.ScnMaterial", Materials_[ MATERIAL_WIN ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->importResource( "GameContent/lose.ScnMaterial", Materials_[ MATERIAL_LOSE ] );
	BcAssert( Imported );


	ScnMaterialRef Font;
	Imported = CsCore::pImpl()->importResource( "EngineContent/font.ScnMaterial", Font );
	BcAssert( Imported );

	ScnSoundRef Sound;
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbitchew0.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbitchew1.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbitchew2.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbithop0.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbithop1.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbithop2.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbithop3.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbithop4.ScnSound", Sound );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->importResource( "GameContent/rabbitscared.ScnSound", Sound );
	BcAssert( Imported );
#endif

	Imported = CsCore::pImpl()->requestResource( "bunny", Materials_[ MATERIAL_BUNNY ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "kitty", Materials_[ MATERIAL_KITTY ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "lettuce", Materials_[ MATERIAL_LETTUCE ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "background", Materials_[ MATERIAL_BACKGROUND ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "foreground", Materials_[ MATERIAL_FOREGROUND ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "bar", Materials_[ MATERIAL_BAR ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "emote", Materials_[ MATERIAL_EMOTE ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "splash", Materials_[ MATERIAL_SPLASH ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "win", Materials_[ MATERIAL_WIN ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "lose", Materials_[ MATERIAL_LOSE ] );
	BcAssert( Imported );

	Imported = CsCore::pImpl()->requestResource( "rabbitchew0", Sounds_[ SOUND_CHEW0 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbitchew1", Sounds_[ SOUND_CHEW1 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbitchew2", Sounds_[ SOUND_CHEW2 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbithop0", Sounds_[ SOUND_HOP0 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbithop1", Sounds_[ SOUND_HOP1 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbithop2", Sounds_[ SOUND_HOP2 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbithop3", Sounds_[ SOUND_HOP3 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbithop4", Sounds_[ SOUND_HOP4 ] );
	BcAssert( Imported );
	Imported = CsCore::pImpl()->requestResource( "rabbitscared", Sounds_[ SOUND_SCARED ] );
	BcAssert( Imported );

}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
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
		if( CsCore::pImpl()->createResource( "CameraEntity", CameraEntity_ ) )
		{
			// Create a view component.
			CsCore::pImpl()->createResource( BcName::INVALID, ViewComponent_, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1024.0f, BcPIDIV4, 0.0f );
			
			// Create default material component.
			CsCore::pImpl()->createResource( BcName::INVALID, ScnMaterialComponent::Default, ScnMaterial::Default, scnSPF_DEFAULT );
			
			// Create a canvas component.
			CsCore::pImpl()->createResource( BcName::INVALID, CanvasComponent_, 1024 * 16, ScnMaterialComponent::Default );
			
			// Attach components.
			CameraEntity_->attach( ViewComponent_ );
			CameraEntity_->attach( ScnMaterialComponent::Default );
			CameraEntity_->attach( CanvasComponent_ );
			
			// Add entity to the scene.
			ScnCore::pImpl()->addEntity( CameraEntity_ );
		}


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
	// Clear canvas ready for use.
	CanvasComponent_->clear();

	// Render all registered states.
	for( TStateList::iterator Iter( StateList_.begin() ); Iter != StateList_.end(); ++Iter )
	{
		GaBaseGameState* pState = (*Iter);

		if( pState->internalStage() == sysBS_MAIN )
		{
			pState->render();
		}
	}

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preLeave
void GaTopState::preLeave()
{
	ScnCore::pImpl()->removeEntity( CameraEntity_ );
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

////////////////////////////////////////////////////////////////////////////////
// getMaterial
void GaTopState::getMaterial( BcU32 Idx, ScnMaterialRef& Material )
{
	Material = Materials_[ Idx ];
}

////////////////////////////////////////////////////////////////////////////////
// getSound
ScnSoundRef GaTopState::getSound( BcU32 Idx )
{
	return Sounds_[ Idx ];
}
