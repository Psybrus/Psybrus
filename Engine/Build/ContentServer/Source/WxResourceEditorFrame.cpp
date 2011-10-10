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

#include "WxPropertyTableDatabase.h"

//////////////////////////////////////////////////////////////////////////
// Event table.
BEGIN_EVENT_TABLE( WxResourceEditorFrame, wxFrame )
END_EVENT_TABLE();

//////////////////////////////////////////////////////////////////////////
// Ctor
WxResourceEditorFrame::WxResourceEditorFrame():
	wxFrame( (wxFrame*)NULL, wxID_ANY, "Resource Editor", wxDefaultPosition, wxSize( 500, 600 ), wxRESIZE_BORDER | wxCLOSE_BOX | wxCAPTION | wxSYSTEM_MENU )
{
	wxFlexGridSizer* pSizerA = new wxFlexGridSizer( 2, 1 );
	pSizerA->AddGrowableRow( 0 );
	pSizerA->AddGrowableCol( 0 );
	
	pGrid_ = new wxGrid( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxALL | wxVSCROLL );

	// Table data.
	pTableData_ = new WxPropertyTableDatabase();
	pGrid_->SetTable( pTableData_, true );

	// Grid.
	pGrid_->SetRowLabelSize( 0 );
	pGrid_->SetMargins( 0, 0 );
	pGrid_->AutoSizeColumns( false );

	//
	pSizerA->Add( pGrid_, 8, wxEXPAND );

	// Layout.
	SetSizer( pSizerA );
	Layout();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
WxResourceEditorFrame::~WxResourceEditorFrame()
{
	
}

//////////////////////////////////////////////////////////////////////////
// populateGrid
void WxResourceEditorFrame::populateGrid()
{

}
