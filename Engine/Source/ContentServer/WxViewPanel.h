/**************************************************************************
*
* File:		WxViewPanel.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		View panel.
*		
*
*
* 
**************************************************************************/

#ifndef __WXVIEWPANEL_H__
#define __WXVIEWPANEL_H__

#include "Psybrus.h"

#include <wx/wx.h>

//////////////////////////////////////////////////////////////////////////
// WxViewPanel
class WxViewPanel: public wxPanel
{
public:
	WxViewPanel( wxWindow* pParent );
	virtual ~WxViewPanel();

private:
	void						OnSize( wxSizeEvent& Event );
	void						OnPaint( wxPaintEvent& Event );
	void						OnKeyUp( wxKeyEvent& Event );
	void						OnKeyDown( wxKeyEvent& Event );
	void						OnMouseMove( wxMouseEvent& Event );
	void						OnMouseDown( wxMouseEvent& Event );
	void						OnMouseUp( wxMouseEvent& Event );
	void						OnMouseWheel( wxMouseEvent& Event );
	void						OnLeaveWindow( wxMouseEvent& Event );

private:
	DECLARE_EVENT_TABLE();

	BcU32						Width_;
	BcU32						Height_;

	wxPaintEvent				PaintEvent_;
	BcBool						PaintDirty_;
};

#endif
