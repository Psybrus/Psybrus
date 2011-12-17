/**************************************************************************
*
* File:		GaPlayer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Player.
*		
*
*
* 
**************************************************************************/

#include "GaPlayer.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaPlayer::GaPlayer()
{
	// Bind input events.
	OsEventInputMouse::Delegate OnMouseMove = OsEventInputMouse::Delegate::bind< GaPlayer, &GaPlayer::onMouseMove >( this );
	OsEventInputMouse::Delegate OnMouseDown = OsEventInputMouse::Delegate::bind< GaPlayer, &GaPlayer::onMouseDown >( this );
	OsEventInputMouse::Delegate OnMouseUp = OsEventInputMouse::Delegate::bind< GaPlayer, &GaPlayer::onMouseUp >( this );

	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEMOVE, OnMouseMove );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEDOWN, OnMouseDown );
	OsCore::pImpl()->subscribe( osEVT_INPUT_MOUSEUP, OnMouseUp );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
GaPlayer::~GaPlayer()
{
	// Unbind all events.
	OsCore::pImpl()->unsubscribeAll( this );
}

////////////////////////////////////////////////////////////////////////////////
// onMouseMove
eEvtReturn GaPlayer::onMouseMove( EvtID ID, const OsEventInputMouse& Event )
{
	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// onMouseDown
eEvtReturn GaPlayer::onMouseDown( EvtID ID, const OsEventInputMouse& Event )
{
	return evtRET_PASS;
}

////////////////////////////////////////////////////////////////////////////////
// onMouseUp
eEvtReturn GaPlayer::onMouseUp( EvtID ID, const OsEventInputMouse& Event )
{
	return evtRET_PASS;
}
