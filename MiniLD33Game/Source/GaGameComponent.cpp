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
	BcFixed( 0.5f ),				// Rate of attack.
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
	BcFixed( 0.25f ),				// Rate of attack.
	BcFixed( 0.25f ),				// Range.
	BcFixed( 0.25f ),				// Range.
	BcFixed( 15.0f ),				// Health.
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
	BcFixed( 50.0f ),				// Health.
	BcFalse,						// Armoured.
	NULL,
};

static GaGameUnitDescriptor GGameUnit_Soldier = 
{
	2,
	BcFixedVec2d( 1.0f, 1.0f ),		// Unit size.
	BcFixed( 1.0f ),				// Move speed.
	BcFixed( 0.5f ),				// Rate of attack.
	BcFixed( 1.0f ),				// Range.
	BcFixed( 1.0f ),				// Range.
	BcFixed( 20.0f ),				// Health.
	BcTrue,							// Armoured.
	&GGameProjectile_Soldier,
};

static GaGameUnitDescriptor GGameUnit_Archer = 
{
	3,
	BcFixedVec2d( 1.0f, 1.0f ),		// Unit size.
	BcFixed( 1.0f ),				// Move speed.
	BcFixed( 0.2f ),				// Rate of attack.
	BcFixed( 14.0f ),				// Range.
	BcFixed( 2.0f ),				// Range.
	BcFixed( 30.0f ),				// Health.
	BcFalse,						// Armoured.
	&GGameProjectile_Archer,
};

static GaGameUnitDescriptor GGameUnit_Trebuchet = 
{
	4,
	BcFixedVec2d( 2.0f, 2.0f ),		// Unit size.
	BcFixed( 0.25f ),				// Move speed.
	BcFixed( 0.1f ),				// Rate of attack.
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
void GaGameComponent::initialise()
{
	Super::initialise();

	MouseDown_ = BcFalse;

	pSimulator_ = new GaGameSimulator( 1.0f / 15.0f, 1.0f );

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
		CanvasComponent_->drawLine( BcVec2d( -100.0f, -100.0f ), BcVec2d( 100.0f, 100.0f ), RsColour::GREEN, 0 );
		//CanvasComponent_->drawBox( BcVec2d( 0.0f, 0.0f ), BcVec2d( 1.0f, 1.0f ), RsColour::WHITE, 0 );

		pSimulator_->render( CanvasComponent_ );
	}

	// Draw cursor.
	CanvasComponent_->drawSpriteCentered( BcVec2d( CursorPosition_.x(), CursorPosition_.y() ), BcVec2d( 64.0f, 64.0f ), 1, RsColour::WHITE, 10 );

	// Draw selection box.
	if( MouseDown_ ) 
	{
		BcVec2d Min = BcVec2d( StartGameCursorPosition_.x(), StartGameCursorPosition_.y() ) * 32.0f;
		BcVec2d Max = BcVec2d( GameCursorPosition_.x(), GameCursorPosition_.y() ) * 32.0f;

		CanvasComponent_->drawSprite( Min, Max - Min, 0, RsColour::GREEN * RsColour( 1.0f, 1.0f, 1.0f, 0.1f ), 11 );
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

	// Bind input events.
	OsEventInputMouse::Delegate OnMouseEvent = OsEventInputMouse::Delegate::bind< GaGameComponent, &GaGameComponent::onMouseEvent >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseEvent );

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

	if( ID == osEVT_INPUT_MOUSEDOWN )
	{
		StartGameCursorPosition_ = GameCursorPosition_;
		MouseDown_ = BcTrue;
	}
	else if( ID == osEVT_INPUT_MOUSEUP )
	{
		EndGameCursorPosition_ = GameCursorPosition_;
		MouseDown_ = BcFalse;
		GaGameUnitIDList FoundUnits;

		if( ( StartGameCursorPosition_ - EndGameCursorPosition_ ).magnitudeSquared() > BcFixed( 8.0f ) )
		{
			pSimulator_->findUnits( FoundUnits, StartGameCursorPosition_, EndGameCursorPosition_, BcErrorCode, 1 << 0 );
		}
		else
		{
			pSimulator_->findUnits( FoundUnits, GameCursorPosition_, 0.8f, BcErrorCode, 1 << 0 );

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
			// Otherwise, tell found units to move.
			BcFixedVec2d CentralPosition;
			for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
			{
				GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
				if( pGameUnit != NULL )
				{
					CentralPosition += pGameUnit->getPosition();
				}
			}

			if( UnitSelection_.size() > 0 )
			{
				CentralPosition /= BcFixed( (int)UnitSelection_.size() );

				GameCursorPosition_ = BcFixedVec2d( ( GameCursorPosition_.x() ), ( GameCursorPosition_.y() ) );

				for( BcU32 Idx = 0; Idx < UnitSelection_.size(); ++Idx )
				{
					GaGameUnit* pGameUnit( pSimulator_->getUnit( UnitSelection_[ Idx ] ) );
					if( pGameUnit != NULL )
					{
						GaGameUnitMoveEvent Event;
						Event.UnitID_ = pGameUnit->getID();
						Event.Position_ = ( pGameUnit->getPosition() - CentralPosition ) + GameCursorPosition_;
						
						pSimulator_->publish( gaEVT_UNIT_MOVE, Event );
					}
				}
			}
		}
	}
	
	return evtRET_PASS;
}
