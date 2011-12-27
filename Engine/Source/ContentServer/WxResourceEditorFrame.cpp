/**************************************************************************
*
* File:		WxResourceEditorFrame.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main editor frame.
*		
*
*
* 
**************************************************************************/

#include "WxResourceEditorFrame.h"

//////////////////////////////////////////////////////////////////////////
// Event table.
BEGIN_EVENT_TABLE( WxResourceEditorFrame, wxFrame )
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////////////
// Ctor
WxResourceEditorFrame::WxResourceEditorFrame():
	wxFrame( (wxFrame*)NULL, wxID_ANY, "Resource Editor", wxDefaultPosition, wxSize( 300, 400 ), wxRESIZE_BORDER | wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU | wxSTAY_ON_TOP )
{
	pFlexGridSizer_ = new wxFlexGridSizer( 1, 1 );
	pFlexGridSizer_->AddGrowableRow( 0 );
	pFlexGridSizer_->AddGrowableCol( 0 );

	wxBoxSizer* pSizer = new wxBoxSizer( wxHORIZONTAL );
	
	pPropertyTablePanel_ = new WxPropertyTablePanel( this );
	//pFlexGridSizer_->Add( pPropertyTablePanel_ );
	pSizer->Add( pPropertyTablePanel_, 5, wxEXPAND  );

	// Layout.
	//SetSizer( pFlexGridSizer_ );
	SetSizer( pSizer );
	Layout();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
WxResourceEditorFrame::~WxResourceEditorFrame()
{
	
}
