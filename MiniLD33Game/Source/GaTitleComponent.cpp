/**************************************************************************
*
* File:		GaTitleComponent.cpp
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Title component.
*		
*
*
* 
**************************************************************************/

#include "GaMatchmakingState.h"

#include "GaTitleComponent.h"

#include "GaTopState.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( GaTitleComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void GaTitleComponent::initialise()
{
	Super::initialise();
}

//////////////////////////////////////////////////////////////////////////
// destroy
void GaTitleComponent::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// GaTitleComponent
//virtual
void GaTitleComponent::update( BcReal Tick )
{
	if( pState_ && pState_->process() )
	{
		delete pState_;
		pState_ = NULL;
	}

	if( Canvas_.isValid() )
	{
		OsClient* pClient = OsCore::pImpl()->getClient( 0 );
		BcReal HW = static_cast< BcReal >( pClient->getWidth() ) / 2.0f;
		BcReal HH = static_cast< BcReal >( pClient->getHeight() ) / 2.0f;
		BcReal AspectRatio = HW / HH;

		BcMat4d Ortho;
		Ortho.orthoProjection( -HW, HW, HH, -HH, -1.0f, 1.0f );

		// Clear canvas and push projection matrix.
		Canvas_->clear();   
		Canvas_->pushMatrix( Ortho );
		Canvas_->setMaterialComponent( Material_ );
		Canvas_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( 1280.0f, 720.0f ), 0, RsColour::WHITE, 0 );

		BcU32 Param = FontMaterial_->findParameter( "aAlphaTestStep" );
		FontMaterial_->setParameter( Param, BcVec2d( 0.4f, 0.5f ) );

		std::string Text0;

		if( pState_ )
		{
			switch( pState_->HandshakeState_ )
			{
			case HSS_STUN:
				Text0 = "Setting up network...";
				break;
			case HSS_IDLE:
				Text0 = "Connecting to server...";
				break;
			case HSS_WAIT_INVITE:
				Text0 = "Searching for LAN game (ESC to retry)...";
				break;
			case HSS_WAIT_ADDR:
				Text0 = "Got game! Exchanging details...";
				break;
			case HSS_COMPLETE:
				Text0 = "Ready to go!";
				GaTopState::pImpl()->startGame( BcTrue );
				break;
			}
		}

		std::string Text1 = "...Or press Enter to play AI.";

		BcVec2d TextSize0 = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Text0, RsColour::WHITE, BcTrue );
		Font_->draw( Canvas_, ( -TextSize0 / 2.0f ) + BcVec2d( 0.0f, 64.0f ), Text0, RsColour::WHITE, BcFalse );
		
		BcVec2d TextSize1 = Font_->draw( Canvas_, BcVec2d( 0.0f, 0.0f ), Text1, RsColour::WHITE, BcTrue );
		Font_->draw( Canvas_, ( -TextSize1 / 2.0f ) + BcVec2d( 0.0f, 96.0f ), Text1, RsColour::WHITE, BcFalse );
	}
}

//////////////////////////////////////////////////////////////////////////
// GaTitleComponent
//virtual
void GaTitleComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Find canvas component on parent. TODO: Make a utility function for this.
	for( BcU32 Idx = 0; Idx < Parent->getNoofComponents(); ++Idx )
	{
		Canvas_ = Parent->getComponent( Idx );

		if( Canvas_.isValid() )
		{
			break;
		}
	}

	// Materials.
	ScnMaterialRef Material;
	if( CsCore::pImpl()->requestResource( "game", "title", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, Material_, Material, BcErrorCode ) )
		{
			Parent->attach( Material_ );
		}
	}

	// Font
	ScnFontRef Font;
	if( CsCore::pImpl()->requestResource( "game", "default", Font ) && CsCore::pImpl()->requestResource( "game", "font", Material ) )
	{
		if( CsCore::pImpl()->createResource( BcName::INVALID, Font_, Font, Material ) )
		{
			FontMaterial_ = Font_->getMaterialComponent();
			Parent->attach( Font_ );
		}
	}

	// Bind input events.
	OsEventInputKeyboard::Delegate OnKeyEvent = OsEventInputKeyboard::Delegate::bind< GaTitleComponent, &GaTitleComponent::onKeyEvent >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnKeyEvent );
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYUP, OnKeyEvent );
	
	// Create state.
	pState_ = new GaMatchmakingState();

	// Don't forget to attach!
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// GaTitleComponent
//virtual
void GaTitleComponent::onDetach( ScnEntityWeakRef Parent )
{
	if( pState_ )
	{
		pState_->leaveState();
		while( !pState_->process() );
		delete pState_;
		pState_ = NULL;
	}
	delete pState_;
	pState_ = NULL;

	// Null canvas reference.
	Canvas_ = NULL;

	// Detach materials.
	Parent->detach( Material_ );
	Parent->detach( FontMaterial_ );
	Parent->detach( Font_ );

	// Unsubscribe.
	OsCore::pImpl()->unsubscribeAll( this );

	// Don't forget to detach!
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onKeyEvent
eEvtReturn GaTitleComponent::onKeyEvent( EvtID ID, const OsEventInputKeyboard& Event )
{
	if( ID == osEVT_INPUT_KEYUP )
	{
		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_RETURN )
		{
			GaTopState::pImpl()->startGame( BcFalse );
		}
		if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_ESCAPE && pState_->HandshakeState_ != HSS_IDLE )
		{
			GaTopState::pImpl()->startMatchmaking();
		}
	}
	return evtRET_PASS;
}

