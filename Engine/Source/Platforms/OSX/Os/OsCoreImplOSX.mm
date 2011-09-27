/**************************************************************************
*
* File:		OsCoreImplOSX.mm
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "OsCoreImplOSX.h"

#include "OsViewOSX.h"

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( OsCoreImplOSX );

//////////////////////////////////////////////////////////////////////////
// Ctor
OsCoreImplOSX::OsCoreImplOSX()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
OsCoreImplOSX::~OsCoreImplOSX()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void OsCoreImplOSX::open()
{
	BcMessageBox( "Something has happened.", "These are the details about whatever has happened.", "OK", "Cancel" );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void OsCoreImplOSX::update()
{
	processInputEvents();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void OsCoreImplOSX::close()
{
	
}

//////////////////////////////////////////////////////////////////////////
// processInputEvents
void OsCoreImplOSX::processInputEvents()
{
	// Handle view input events.
	OsViewOSX_Interface::TInputEvent InputEvent;
	
	while( OsViewOSX_Interface::GetInputEvent( InputEvent ) )
	{
		switch( InputEvent.EventID_ )
		{
			// Mouse events.
			case osEVT_INPUT_MOUSEMOVE:
			case osEVT_INPUT_MOUSEDOWN:
			case osEVT_INPUT_MOUSEUP:
				EventInputMouse_.DeviceID_ = InputEvent.DeviceID_;
				EventInputMouse_.MouseX_ = InputEvent.MouseX_;
				EventInputMouse_.MouseY_ = InputEvent.MouseY_;
				EventInputMouse_.ButtonCode_ = InputEvent.ButtonCode_;
				EvtPublisher::publish( InputEvent.EventID_, EventInputMouse_ );
				break;
				
			// Key events.				
			case osEVT_INPUT_KEYDOWN:
			case osEVT_INPUT_KEYUP:
				EventInputKeyboard_.DeviceID_ = InputEvent.DeviceID_;
				EventInputKeyboard_.KeyCode_ = InputEvent.KeyCode_;
				EvtPublisher::publish( InputEvent.EventID_, EventInputKeyboard_ );
				break;
		}
	}
}
