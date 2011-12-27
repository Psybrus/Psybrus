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
// OnPreCsUpdate
eEvtReturn OnPreCsUpdate( EvtID, const SysSystemEvent& )
{
	std::list< BcPath > OutputFiles;
	
	// Search game content & engine content paths.
	FsCore::pImpl()->findFiles( "./GameContent", BcTrue, BcFalse, OutputFiles );
	FsCore::pImpl()->findFiles( "./EngineContent", BcTrue, BcFalse, OutputFiles );

	// Import all files that were found.
	for( std::list< BcPath >::iterator It( OutputFiles.begin() ); It != OutputFiles.end(); ++It )
	{
		const BcPath& Path = (*It);
		const BcChar* pExtension = Path.getExtension();

		// Attempt to import resource.
		CsCore::pImpl()->importResource( *Path );
	}

	// Pass to next handler.
	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// OnInit
//virtual
bool WxAppContentServer::OnInit()
{
	// Splash screen.
	wxSplashScreen* pSplash = new wxSplashScreen( wxBitmap( "Splash/splash.bmp", wxBITMAP_TYPE_BMP ), wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_NO_TIMEOUT, 0, NULL, wxID_ANY );
	pSplash->Show();

	// Hook file search for importing.
	SysSystemEvent::Delegate CsPreUpdateDelegate = SysSystemEvent::Delegate::bind< OnPreCsUpdate >();
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, CsPreUpdateDelegate );
	
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
