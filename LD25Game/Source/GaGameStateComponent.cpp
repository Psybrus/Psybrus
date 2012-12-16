/**************************************************************************
*
* File:		GaGameStateComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	15/12/12		
* Description:
*		Game state component. Logic.
*		
*
*
* 
**************************************************************************/

#include "GaGameStateComponent.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameStateComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameStateComponent::initialise( const Json::Value& Object )
{
	Super::initialise( Object );

	GameState_ = GS_SPLASH;
}

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
//virtual
void GaGameStateComponent::update( BcReal Tick )
{
	Super::update( Tick );

	Canvas_->clear();
	ScnMaterialComponentRef MaterialComponent( getParentEntity()->getComponentByType<ScnMaterialComponent>( 0 ) );
	Canvas_->setMaterialComponent( MaterialComponent );

	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
	BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
	BcReal AspectRatio = HW / HH;
		
	BcMat4d Ortho;
	Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );
	
	Canvas_->pushMatrix( Ortho );

	switch( GameState_ )
	{
		case GS_SPLASH:
		{
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, -192.0f ), "Crazy Dr. Banana's ", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, -160.0f ), "Pursuit of", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, -128.0f ), "The Goat of Good.", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, -96.0f ), "by NeiloGD", RsColour::WHITE, BcFalse );

			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 0.0f ), "Dr Banana is bent of destroying the universe,", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 32.0f ), "and has been hunting The Goat of Good for decades.", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 64.0f ), "You must whistle or play ocarina to control the ship,", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 96.0f ), "and weapons are fired by shouting \"PCHEW!\".", RsColour::WHITE, BcFalse );

			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 160.0f ), "Click to Continue", RsColour::WHITE, BcFalse );
		}
		break;

		case GS_GAME:
		{
		}
		break;

		case GS_WIN:
		{
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, -192.0f ), "You win!", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 0.0f ), "Evil triumphs!", RsColour::WHITE, BcFalse );

			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 160.0f ), "Click to Continue", RsColour::WHITE, BcFalse );
		}
		break;

		case GS_LOSE:
		{
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, -192.0f ), "You lose!", RsColour::WHITE, BcFalse );
			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 0.0f ), "Good triumphs!", RsColour::WHITE, BcFalse );

			Font_->drawCentered( Canvas_, BcVec2d( 0.0f, 160.0f ), "Click to Continue", RsColour::WHITE, BcFalse );
		}
		break;
	}


}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void GaGameStateComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Don't forget to attach!
	Super::onAttach( Parent );

	// Grab the canvas.
	Canvas_ = Parent->getComponentAnyParentByType< ScnCanvasComponent >( 0 );

	// Grab the font.
	Font_ = Parent->getComponentAnyParentByType< ScnFontComponent >( 0 );

	// Spawn level.
	ScnEntityRef LevelEntity = ScnCore::pImpl()->createEntity( "default", "LevelEntity" );
	Parent->attach( LevelEntity );

	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaGameStateComponent, &GaGameStateComponent::onMouseDown >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );

	EvtNullEvent::Delegate OnEnemyDie = EvtNullEvent::Delegate::bind< GaGameStateComponent, &GaGameStateComponent::onEnemyDie >( this );
	getParentEntity()->subscribe( gaEVT_ENEMY_DIE, OnEnemyDie );

	EvtNullEvent::Delegate OnPlayerDie = EvtNullEvent::Delegate::bind< GaGameStateComponent, &GaGameStateComponent::onPlayerDie >( this );
	getParentEntity()->subscribe( gaEVT_PLAYER_DIE, OnPlayerDie );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void GaGameStateComponent::onDetach( ScnEntityWeakRef Parent )
{
	Canvas_ = NULL;
	Font_ = NULL;

	if( PlayerEntity_.isValid() )
	{
		PlayerEntity_->detachFromParent();
		PlayerEntity_ = NULL;
	}

	if( EnemyEntity_.isValid() )
	{
		EnemyEntity_->detachFromParent();
		EnemyEntity_ = NULL;
	}

	OsCore::pImpl()->unsubscribeAll( this );

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaGameStateComponent::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	switch( GameState_ )
	{
		case GS_SPLASH:
		{
			GameState_ = GS_GAME;

			// Spawn player.
			PlayerEntity_ = ScnCore::pImpl()->createEntity( "default", "PlayerEntity" );
			getParentEntity()->attach( PlayerEntity_ );

			// Spawn enemy.
			EnemyEntity_ = ScnCore::pImpl()->createEntity( "default", "EnemyEntity" );
			getParentEntity()->attach( EnemyEntity_ );
		}
		break;

		case GS_GAME:
		{
		}
		break;

		case GS_LOSE:
		case GS_WIN:
		{
			GameState_ = GS_SPLASH;

			if( PlayerEntity_.isValid() )
			{
				PlayerEntity_->detachFromParent();
				PlayerEntity_ = NULL;
			}

			if( EnemyEntity_.isValid() )
			{
				EnemyEntity_->detachFromParent();
				EnemyEntity_ = NULL;
			}
		}
		break;
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onPlayerDie
eEvtReturn GaGameStateComponent::onPlayerDie( EvtID ID, const EvtNullEvent& Event )
{
	GameState_ = GS_LOSE;

	if( PlayerEntity_.isValid() )
	{
		PlayerEntity_->detachFromParent();
		PlayerEntity_ = NULL;
	}

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// onEnemyDie
eEvtReturn GaGameStateComponent::onEnemyDie( EvtID ID, const EvtNullEvent& Event )
{
	GameState_ = GS_WIN;

	if( EnemyEntity_.isValid() )
	{
		EnemyEntity_->detachFromParent();
		EnemyEntity_ = NULL;
	}

	return evtRET_PASS;
}
