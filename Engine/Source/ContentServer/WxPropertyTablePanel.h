/**************************************************************************
*
* File:		WxPropertyTablePanel.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Property table panel.
*		
*
*
* 
**************************************************************************/

#ifndef __WXPROPERTYTABLEPANEL_H__
#define __WXPROPERTYTABLEPANEL_H__

#include "Psybrus.h"

#include <wx/wx.h>
#include <wx/propgrid/propgrid.h>

//////////////////////////////////////////////////////////////////////////
// WxPropertyTablePanel
class WxPropertyTablePanel:
	public wxPanel
{
public:
	WxPropertyTablePanel( wxWindow* pParent );
	virtual ~WxPropertyTablePanel();
	
	void setupPropertyTable( wxPGProperty* pParent, const CsPropertyTable& PropertyTable );
	
private:
	wxPropertyGrid*					pPropertyGrid_;
};


#endif
