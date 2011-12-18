/**************************************************************************
*
* File:		GaOverlayState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Overlay state.
*		
*
*
* 
**************************************************************************/

#include "GaOverlayState.h"
#include "GaTopState.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaOverlayState::GaOverlayState( ScnMaterialRef Material, const std::string& Text, GaBaseGameState* pNextState )
{
	Text_ = Text;
	if( Material.isValid() )
	{
		Material->createInstance( *Material->getName() + "materialinstance", MaterialInstance_, BcErrorCode );
	}

	CurrOverlay_ = 0;
	pNextState_ = pNextState;

	// Bind input events.
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaOverlayState, &GaOverlayState::onMouseDown >( this );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaOverlayState::~GaOverlayState()
{
	OsCore::pImpl()->unsubscribeAll( this );
}

////////////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaOverlayState::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	if( MaterialInstance_.isValid() )
	{
		CurrOverlay_++;

		if( CurrOverlay_ == MaterialInstance_->getTexture( 0 )->noofRects() )
		{
			if( pNextState_ != NULL ) 
			{
				GaTopState::pImpl()->spawnSubState( 0, pNextState_ );
			}
			leaveState();
		}
	}
	else
	{
		if( pNextState_ != NULL ) 
		{
			GaTopState::pImpl()->spawnSubState( 0, pNextState_ );
		}
		leaveState();
	}
	return evtRET_BLOCK;
}

////////////////////////////////////////////////////////////////////////////////
// enterOnce
void GaOverlayState::enterOnce()
{
	CsCore::pImpl()->requestResource( "font", FontMaterial_ );
	
	GaBaseGameState::enterOnce();
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaOverlayState::enter()
{
	if( GaBaseGameState::enter() == sysSR_CONTINUE )
	{
		return sysSR_CONTINUE;
	}

	if( FontMaterial_->isReady() )
	{
		ScnFont::Default->createInstance( "defaultfontinstance", FontInstance_, FontMaterial_ );

		return sysSR_FINISHED;
	}

	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaOverlayState::preMain()
{

}

////////////////////////////////////////////////////////////////////////////////
// GaOverlayState
eSysStateReturn GaOverlayState::main()
{
	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// leave
eSysStateReturn GaOverlayState::leave()
{
	return GaBaseGameState::leave();
}

////////////////////////////////////////////////////////////////////////////////
// leaveOnce
void GaOverlayState::leaveOnce()
{

}

////////////////////////////////////////////////////////////////////////////////
// render
void GaOverlayState::render( RsFrame* pFrame )
{
	Canvas_->clear();

	BcMat4d Projection;

	BcReal Aspect = (BcReal)GResolutionWidth / (BcReal)GResolutionHeight;
	BcReal DesiredHeight = 240.0f;
	BcReal DesiredWidth = Aspect * DesiredHeight;

	Projection.orthoProjection( -DesiredWidth, DesiredWidth, DesiredHeight, -DesiredHeight, -1.0f, 1.0f );

	Canvas_->pushMatrix( Projection );

	if( MaterialInstance_.isValid() )
	{
		Canvas_->setMaterialInstance( MaterialInstance_ );
		Canvas_->drawSpriteCentered( BcVec2d( 0.0f, 0.0f ), BcVec2d( 512.0f, 300.0f ), CurrOverlay_, RsColour::WHITE, 0 );
	}

	// Build up list of strings.
	if( Text_.length() > 0 )
	{
		std::vector<std::string> StringList;
		std::string TextCopy = Text_;
		if( TextCopy.find( '\n' ) != std::string::npos )
		{
			BcU32 CurrPos = 0;
			BcU32 NextPos = 0;
			while( ( NextPos = TextCopy.find( '\n' ) ) != std::string::npos )
			{
				std::string Temp = TextCopy.substr( 0, NextPos );
				TextCopy = TextCopy.substr( NextPos + 1 );
				CurrPos = NextPos;
				StringList.push_back( Temp );
			}
			StringList.push_back( TextCopy );
		}
		else
		{
			StringList.push_back( Text_ );
		}

		BcVec2d TopPosition( 0.0f, 0.0f );
		ScnMaterialInstanceRef FontMaterialInstance = FontInstance_->getMaterialInstance();
		BcU32 ParameterIdx = FontMaterialInstance->findParameter( "aAlphaTestStep" );
		FontMaterialInstance->setParameter( ParameterIdx, BcVec2d( 0.0f, 0.5f ) );
	
		for( BcU32 Idx = 0; Idx < StringList.size(); ++Idx )
		{
			BcVec2d Size = FontInstance_->draw( Canvas_, StringList[ Idx ], BcTrue );

			BcMat4d Translation;
			Translation.translation( -BcVec3d( Size.x() + TopPosition.x(), Size.y() + TopPosition.y(), 0.0f ) * 0.5f );

			Canvas_->pushMatrix( Translation );

			FontInstance_->draw( Canvas_, StringList[ Idx ] );

			TopPosition -= BcVec2d( 0.0f, BcAbs( Size.y() * 2.0f ) );

			Canvas_->popMatrix();
		}
	}

	Canvas_->popMatrix();


	GaBaseGameState::render( pFrame );
}
