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

#include "EvtBridgeIRC.h"

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

	// dummy.
	//pBridgeIRC_ = new EvtBridgeIRC( OsCore::pImpl() );
	pBridgeIRC_ = NULL;
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
		BcU32 TeamID = 0;
		if( SysArgs_.find( "-client ") != std::string::npos )
		{
			TeamID = 1;
		}

		ScnMaterialComponentRef MaterialComponent;
		ScnViewComponentRef ViewComponent;
		ScnCanvasComponentRef CanvasComponent;
		GaGameComponentRef GameComponent0;
		GaGameComponentRef GameComponent1;
		
		// Create component resources.
		CsCore::pImpl()->createResource( BcName::INVALID, ViewComponent, 0.0f, 0.0f, 1.0f, 1.0f, 0.1f, 1000.0f, BcPIDIV4, 0.0f );
		CsCore::pImpl()->createResource( BcName::INVALID, MaterialComponent, SpriteSheetMaterial0_, BcErrorCode );
		CsCore::pImpl()->createResource( BcName::INVALID, CanvasComponent, 1024 * 32, MaterialComponent );
		CsCore::pImpl()->createResource( BcName::INVALID, GameComponent0, TeamID );
		//CsCore::pImpl()->createResource( BcName::INVALID, GameComponent1, 1 );

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

	delete pBridgeIRC_;
	pBridgeIRC_ = NULL;
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
