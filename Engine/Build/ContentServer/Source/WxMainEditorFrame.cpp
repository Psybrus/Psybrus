/**************************************************************************
*
* File:		WxMainEditorFrame.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main editor frame.
*		
*
*
* 
**************************************************************************/

#include "WxMainEditorFrame.h"

#include "WxViewPanel.h"

//////////////////////////////////////////////////////////////////////////
// Event table.
BEGIN_EVENT_TABLE( WxMainEditorFrame, wxFrame )
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////////////
// Ctor
WxMainEditorFrame::WxMainEditorFrame():
	wxFrame( (wxFrame*)NULL, wxID_ANY, GPsySetupParams.Name_.c_str(), wxDefaultPosition, wxSize( 320, 240 ) )
{
	// Create sizer.
	wxFlexGridSizer* pMainSizer = new wxFlexGridSizer( 1, 1 );
	pMainSizer->AddGrowableCol( 0 );
	pMainSizer->AddGrowableRow( 0 );

	// Add view panel to sizer.
	WxViewPanel* pViewPanel = new WxViewPanel( this );
	pMainSizer->Add( pViewPanel, 0, wxEXPAND );

	// Set sizer and layout.
	SetSizer( pMainSizer );
	Layout();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
WxMainEditorFrame::~WxMainEditorFrame()
{

}
