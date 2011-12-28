/**************************************************************************
*
* File:		WxAppContentServer.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		wxWidgets Content Server App
*		
*
*
* 
**************************************************************************/

#ifndef __WXAPPCONTENTSERVER_H__
#define __WXAPPCONTENTSERVER_H__

#include "Psybrus.h"

#include <wx/wx.h>
#include <wx/splash.h>
#include <wx/taskbar.h>

//////////////////////////////////////////////////////////////////////////
// WxAppContentServer
class WxAppContentServer: 
	public wxApp
{
public:
 	virtual bool OnInit();
	virtual int OnExit();

	void OnTaskBarRightDown( wxTaskBarIconEvent& Event );
	void OnMenuScanForContent( wxCommandEvent& Event );
	void OnMenuImportContent( wxCommandEvent& Event );
	void OnMenuExit( wxCommandEvent& Event );

private:
	DECLARE_EVENT_TABLE();

	wxTaskBarIcon* pTaskBarIcon_;
	wxIcon* pTaskBarIconImage_;

};

#endif