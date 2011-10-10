/**************************************************************************
*
* File:		WxPropertyTablePanel.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Property table panel.
*		
*
*
* 
**************************************************************************/

#include "WxPropertyTablePanel.h"


//////////////////////////////////////////////////////////////////////////
// Ctor
WxPropertyTablePanel::WxPropertyTablePanel( wxWindow* pParent ):
	wxPanel( pParent )
{
	wxBoxSizer* pSizer = new wxBoxSizer( wxHORIZONAL );

	pPropertyGrid_ = new wxPropertyGrid( pParent );

	// Layout.
	SetSizer( pSizer );
	Layout();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
WxPropertyTablePanel::~WxPropertyTablePanel()
{
	
}
