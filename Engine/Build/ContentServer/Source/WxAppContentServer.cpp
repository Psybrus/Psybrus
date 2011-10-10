/**************************************************************************
*
* File:		WxAppContentServer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		wxWidgets Content Server App
*		
*
*
* 
**************************************************************************/

#include "WxAppContentServer.h"

#include "WxMainEditorFrame.h"
#include "WxResourceEditorFrame.h"

IMPLEMENT_APP_NO_MAIN( WxAppContentServer );

//////////////////////////////////////////////////////////////////////////
// OnInit
//virtual
bool WxAppContentServer::OnInit()
{
	// Splash screen.
	wxSplashScreen* pSplash = new wxSplashScreen( wxBitmap( "Splash/splash.bmp", wxBITMAP_TYPE_BMP ), wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 0, NULL, wxID_ANY );
	pSplash->Show();

	// Start up the kernel multithreaded.
	SysKernel::pImpl()->run( BcTrue );

	// Create frame (temp).
	WxResourceEditorFrame* pFrame = new WxResourceEditorFrame();
	pFrame->Show();

	// Delete splash screen.
	delete pSplash;

	// Initialised.
	return true;
}

//////////////////////////////////////////////////////////////////////////
// OnExit
//virtual
int WxAppContentServer::OnExit()
{
	return 0;
}
