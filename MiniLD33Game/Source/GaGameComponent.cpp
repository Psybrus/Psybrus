/**************************************************************************
*
* File:		GaGameComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#include "GaGameComponent.h"

//////////////////////////////////////////////////////////////////////////
// Units
static GaGameUnitDescriptor GGameProjectile_Soldier = 
{
	5,
	BcFixedVec2d( 0.5f, 0.5f ),		// Unit size.
	BcFixed( 32.0f ),				// Move speed.
	BcFixed( 2.0f ),				// Rate of attack.
	BcFixed( 0.1f ),				// Range.
	BcFixed( 0.1f ),				// Range.
	BcFixed( 7.0f ),				// Health.
	BcFalse,						// Armoured.
	NULL,
};

static GaGameUnitDescriptor GGameProjectile_Archer = 
{
	6,
	BcFixedVec2d( 0.5f, 0.5f ),		// Unit size.
	BcFixed( 32.0f ),				// Move speed.
	BcFixed( 0.5f ),				// Rate of attack.
	BcFixed( 0.25f ),				// Range.
	BcFixed( 0.25f ),				// Range.
	BcFixed( 10.0f ),				// Health.
	BcFalse,						// Armoured.
	NULL,
};

static GaGameUnitDescriptor GGameProjectile_Trebuchet = 
{
	7,
	BcFixedVec2d( 0.5f, 0.5f ),		// Unit size.
	BcFixed( 16.0f ),				// Move speed.
	BcFixed( 0.5f ),				// Rate of attack.
	BcFixed( 1.25f ),				// Range.
	BcFixed( 1.25f ),				// Range.
	BcFixed( 30.0f ),				// Health.
	BcFalse,						// Armoured.
	NULL,
};

static GaGameUnitDescriptor GGameUnit_Soldier = 
{
	2,
	BcFixedVec2d( 1.0f, 1.0f ),		// Unit size.
	BcFixed( 2.5f ),				// Move speed.
	BcFixed( 1.2f ),				// Rate of attack.
	BcFixed( 1.2f ),				// Range.
	BcFixed( 1.0f ),				// Range.
	BcFixed( 20.0f ),				// Health.
	BcTrue,							// Armoured.
	&GGameProjectile_Soldier,
};

static GaGameUnitDescriptor GGameUnit_Archer = 
{
	3,
	BcFixedVec2d( 1.0f, 1.0f ),		// Unit size.
	BcFixed( 2.5f ),				// Move speed.
	BcFixed( 0.5f ),				// Rate of attack.
	BcFixed( 14.0f ),				// Range.
	BcFixed( 2.0f ),				// Range.
	BcFixed( 40.0f ),				// Health.
	BcFalse,						// Armoured.
	&GGameProjectile_Archer,
};

static GaGameUnitDescriptor GGameUnit_Trebuchet = 
{
	4,
	BcFixedVec2d( 2.0f, 2.0f ),		// Unit size.
	BcFixed( 0.3f ),				// Move speed.
	BcFixed( 0.25f ),				// Rate of attack.
	BcFixed( 25.0f ),				// Range.
	BcFixed( 10.0f ),				// Range.
	BcFixed( 100.0f ),				// Health.
	BcFalse,						// Armoured.
	&GGameProjectile_Trebuchet,
};

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaGameComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void GaGameComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "GaGameComponent" )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
void GaGameComponent::initialise( BcU32 TeamID )
{
	Super::initialise();

	MouseDown_ = BcFalse;
	BoxSelection_ = BcFalse;
	CtrlDown_ = BcFalse; 
	AttackMove_ = BcFalse;
	TeamID_ = TeamID;

	// Setup control groups.
	for( BcU32 Idx = 0; Idx < 10; ++Idx )
	{
		ControlGroups_.push_back( GaGameUnitIDList() );
	}

	pSimulator_ = new GaGameSimulator( 1.0f / 15.0f, 1.0f, TeamID, "localhost", 6000 );

	pSimulator_->addUnit( GGameUnit_Trebuchet, 0, BcFixedVec2d( -19.0f,  0.0f ) );

	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 0, BcFixedVec2d( -17.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -15.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 0, BcFixedVec2d( -14.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Trebuchet, 1, BcFixedVec2d(  19.0f,  0.0f ) );
	
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Archer, 1, BcFixedVec2d(  17.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  15.0f,  2.0f  ) );

	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f, -2.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f, -1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f,  0.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f,  1.0f  ) );
	pSimulator_->addUnit( GGameUnit_Soldier, 1, BcFixedVec2d(  14.0f,  2.0f  ) );
}

//////////////////////////////////////////////////////////////////////////
// destroy
void GaGameComponent::destroy()
{
	delete pSimulator_;
	pSimulator_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::update( BcReal Tick )
{
	pSimulator_->tick( Tick );

	//if( TeamID_ == 0 )
	{
		if( CanvasComponent_.isValid() )
		{
			OsClient* pClient = OsCore::pImpl()->getClient( 0 );
			BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
			BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
			BcReal AspectRatio = HW / HH;

			BcMat4d Ortho;
			Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );

			// Clear canvas and push projection matrix.
			CanvasComponent_->clear();   

			CanvasComponent_->pushMatrix( Ortho );

			CanvasComponent_->setMaterialComponent( BackgroundMaterial_ );
			CanvasComponent_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( 1280.0f, 720.0f ), 0, RsColour( 1.0f, 1.0f, 1.0f, 1.0f ), 0 );

			CanvasComponent_->setMaterialComponent( SpriteSheetMaterials_[ 0 ] );
			pSimulator_->render( CanvasComponent_, 0 );
			CanvasComponent_->setMaterialComponent( SpriteSheetMaterials_[ 1 ] );
			pSimulator_->render( CanvasComponent_, 1 );

			// Find unit over mouse.
			GaGameUnitIDList SelectionList = UnitSelection_;
			if( MouseDown_ )
			{
				if( BoxSelection_ )
				{
					pSimulator_->findUnits( SelectionList, StartGameCursorPosition_, EndGameCursorPosition_, BcErrorCode, BcErrorCode );
				}
			}
			else
			{
				pSimulator_->findUnits( SelectionList, GameCursorPosition_, 0.5f, BcErrorCode, BcErrorCode );
			}

			CanvasComponent_->setMaterialComponent( HUDMaterial_ );
			pSimulator_->renderHUD( CanvasComponent_, SelectionList );
		}

		// Draw cursor.
		CanvasComponent_->setMaterialComponent( HUDMaterial_ );
		CanvasComponent_->drawSpriteCentered( BcVec2d( CursorPosition_.x(), CursorPosition_.y() ), BcVec2d( 64.0f, 64.0f ), 1, RsColour::WHITE, 10 );

		// Draw selection box.
		if( MouseDown_ && BoxSelection_ ) 
		{
			BcVec2d Min = BcVec2d( StartGameCursorPosition_.x(), StartGameCursorPosition_.y() ) * 32.0f;
			BcVec2d Max = BcVec2d( GameCursorPosition_.x(), GameCursorPosition_.y() ) * 32.0f;
			CanvasComponent_->drawSprite( Min, Max - Min, 0, RsColour::GREEN * RsColour( 1.0f, 1.0f, 1.0f, 0.1f ), 11 );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find canvas component on parent. TODO: Make a utility function for this.
	for( BcU32 Idx = 0; Idx < Parent->getNoofComponents(); ++Idx )
	{
		CanvasComponent_ = Parent->getComponent( Idx );

		if( CanvasComponent_.isValid() )
		{
			break;
		}
	}

	// Materials.
	ScnMaterialRef Material;
	if( CsCore::pImpl()->requestResource( "background", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, BackgroundMaterial_, Material, BcErrorCode ) )
		{
			Parent->attach( BackgroundMaterial_ );
		}
	}
	if( CsCore::pImpl()->requestResource( "spritesheet0", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, SpriteSheetMaterials_[ 0 ], Material, BcErrorCode ) )
		{
			Parent->attach( SpriteSheetMaterials_[ 0 ] );
		}
	}
	if( CsCore::pImpl()->requestResource( "spritesheet1", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, SpriteSheetMaterials_[ 1 ], Material, BcErrorCode ) )
		{
			Parent->attach( SpriteSheetMaterials_[ 1 ] );
		}
	}
	if( CsCore::pImpl()->requestResource( "hud", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, HUDMaterial_, Material, BcErrorCode ) )
		{
			Parent->attach( HUDMaterial_ );
		}
	}
	
	// Bind input events.
	//if( TeamID_ == 0 )
	{
		OsEventInputMouse::Delegate OnMouseEvent = OsEventInputMouse::Delegate::bind< GaGameComponent, &GaGameComponent::onMouseEvent >( this );
		OsEventInputKeyboard::Delegate OnKeyEvent = OsEventInputKeyboard::Delegate::bind< GaGameComponent, &GaGameComponent::onKeyEvent >( this );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyEvent );
		OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyEvent );
	}

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
//virtual
void GaGameComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Null canvas reference.
	CanvasComponent_ = NULL;

	// Detach materials.
	Parent->detach( BackgroundMaterial_ );
	Parent->detach( SpriteSheetMaterials_[ 0 ] );
	Parent->detach( SpriteSheetMaterials_[ 1 ] );
	Parent->detach( HUDMaterial_ );

	// Unsubscribe.
	OsCore::pImpl()->unsubscribeAll( this );

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onMouseEvent
eEvtReturn GaGameComponent::onMouseEvent( EvtID ID, const OsEventInputMouse& Event )
{
	// Convert to rendering space.
	OsClient* pClient = OsCore::pImpl()->getClient( 0 );
	BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
	BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
	CursorPosition_.set( Event.MouseX_ - HW, Event.MouseY_ - HH );
	GameCursorPosition_ = CursorPosition_ / 32.0f;
	EndGameCursorPosition_ = GameCursorPosition_;

	if( MouseDown_ && ( StartGameCursorPosition_ - EndGameCursorPosition_ ).magnitudeSquared() > BcFixed( 8.0f ) )
	{
		BoxSelection_ = BcTrue;
	}

	if( ID == osEVT_INPUT_MOUSEDOWN )
	{
		StartGameCursorPosition_ = GameCursorPosition_;
		MouseDown_ = BcTrue;
		BoxSelection_ = BcFalse;
	}
	else if( ID == osEVT_INPUT_MOUSEUP )
	{
		EndGameCursorPosition_ = GameCursorPosition_;
		MouseDown_ = BcFalse;
		GaGameUnitIDList FoundUnits;

		if( BoxSelection_ )
		{
			pSimulator_->findUnits( FoundUnits, StartGameCursorPosition_, EndGameCursorPosition_, BcErrorCode, 1 << TeamID_ );
		}
		else
		{
			pSimulator_->findUnits( FoundUnits, GameCursorPosition_, 0.5f, BcErrorCode, 1 << TeamID_ );

			while( FoundUnits.size() > 1 )
			{
				FoundUnits.pop_back();
			}
		}

		// If we found units, then set selection.
		if( FoundUnits.size() > 0 )
		{
			UnitSelection_ = FoundUnits;
		}
		else
		{
			// If we aren't box selection do action.
			if( BoxSelection_ == BcFalse )
			{
				// Otherwise, tell found units to move.
				BcFixedVec2d CentralPosition;
				BcFixed Divisor;
				for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
				{
					GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
					if( pGameUnit != NULL )
					{
						CentralPosition += pGameUnit->getPosition();
						Divisor += 1.0f;
					}
				}

				if( UnitSelection_.size() > 0 && Divisor > 0.0f )
				{
					CentralPosition /= Divisor;

					GameCursorPosition_ = BcFixedVec2d( ( GameCursorPosition_.x() ), ( GameCursorPosition_.y() ) );

					BcFixed PlayfieldHW = 1280.0f * 0.5f / 32.0f;
					BcFixed PlayfieldHH = 720.0f * 0.5f / 32.0f;

					for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
					{
						GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
						if( pGameUnit != NULL )
						{
							GaGameUnitMoveEvent Event;
							Event.UnitID_ = pGameUnit->getID();
							Event.Position_ = ( pGameUnit->getPosition() - CentralPosition ) + GameCursorPosition_;

							Event.Position_.x( BcClamp( Event.Position_.x(), -PlayfieldHW, PlayfieldHW ) );
							Event.Position_.y( BcClamp( Event.Position_.y(), -PlayfieldHH, PlayfieldHH ) );
						
							pSimulator_->publish( gaEVT_UNIT_MOVE, Event );
						}
					}
				}
			}
			else
			{
				// If we were box selecting clear selection.
				UnitSelection_.clear();
			}
		}

		BoxSelection_ = BcFalse;
	}
	
	return evtRET_PASS;
}


//////////////////////////////////////////////////////////////////////////
// onKeyEvent
eEvtReturn GaGameComponent::onKeyEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_CONTROL )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			CtrlDown_ = BcTrue;
		}
		else if( ID == osEVT_INPUT_KEYUP )
		{
			CtrlDown_ = BcFalse;
		}
	}
	else if( Event.KeyCode_ >= '0' && Event.KeyCode_ <= '9' )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			BcU32 Idx = Event.KeyCode_ - '0';
			if( CtrlDown_ )
			{
				ControlGroups_[ Idx ] = UnitSelection_;
			}
			else
			{
				UnitSelection_ = ControlGroups_[ Idx ];
			}
		}
	}
	else if( Event.KeyCode_ == 'A' )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			AttackMove_ = !AttackMove_;
		}
	}
	else if( Event.KeyCode_ == 'S' )
	{
		if( ID == osEVT_INPUT_KEYDOWN )
		{
			for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
			{
				GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
				if( pGameUnit != NULL )
				{
					GaGameUnitIdleEvent Event;
					Event.UnitID_ = pGameUnit->getID();
					pSimulator_->publish( gaEVT_UNIT_IDLE, Event );
				}
			}
		}
	}

	return evtRET_PASS;
}
