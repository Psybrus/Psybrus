/**************************************************************************
*
* File:		WxMainEditorFrame.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main editor frame.
*		
*
*
* 
**************************************************************************/

#ifndef __WXMAINEDITORFRAME_H__
#define __WXMAINEDITORFRAME_H__

#include "Psybrus.h"

#include <wx/wx.h>

//////////////////////////////////////////////////////////////////////////
// WxMainEditorFrame
class WxMainEditorFrame:
	public wxFrame
{
public:
	WxMainEditorFrame();
	virtual ~WxMainEditorFrame();

private:
	DECLARE_EVENT_TABLE();

};

#endif
