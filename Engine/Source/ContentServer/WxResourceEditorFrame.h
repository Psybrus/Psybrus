/**************************************************************************
*
* File:		WxResourceEditorFrame.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main editor frame.
*		
*
*
* 
**************************************************************************/

#ifndef __WXRESOURCEEDITORFRAME_H__
#define __WXRESOURCEEDITORFRAME_H__

#include "Psybrus.h"

#include "WxPropertyTablePanel.h"

#include <wx/wx.h>

//////////////////////////////////////////////////////////////////////////
// WxResourceEditorFrame
class WxResourceEditorFrame:
	public wxFrame
{
public:
	WxResourceEditorFrame();
	virtual ~WxResourceEditorFrame();

private:
	DECLARE_EVENT_TABLE();
	
	wxFlexGridSizer*					pFlexGridSizer_;
	WxPropertyTablePanel*				pPropertyTablePanel_;
		
};

#endif
