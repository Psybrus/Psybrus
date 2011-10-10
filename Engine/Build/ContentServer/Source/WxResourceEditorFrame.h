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

#include <wx/wx.h>
#include <wx/treectrl.h>
#include <wx/imaglist.h>
#include <wx/grid.h>
#include <wx/dnd.h>

//////////////////////////////////////////////////////////////////////////
// WxResourceEditorFrame
class WxResourceEditorFrame:
	public wxFrame
{
public:
	WxResourceEditorFrame();
	virtual ~WxResourceEditorFrame();

	void populateGrid();

private:
	DECLARE_EVENT_TABLE();
	
	wxGrid*								pGrid_;
	class WxPropertyTableDatabase*		pTableData_;
		
};

#endif
