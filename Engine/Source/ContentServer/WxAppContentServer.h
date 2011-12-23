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

//////////////////////////////////////////////////////////////////////////
// WxAppContentServer
class WxAppContentServer: 
	public wxApp
{
public:
 	virtual bool OnInit();
	virtual int OnExit();
};

#endif