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

//////////////////////////////////////////////////////////////////////////
// Implement app.
IMPLEMENT_APP_NO_MAIN( WxAppContentServer );

//////////////////////////////////////////////////////////////////////////
// Event table.
BEGIN_EVENT_TABLE( WxAppContentServer, wxApp )
	EVT_TASKBAR_RIGHT_DOWN( WxAppContentServer::OnTaskBarRightDown )
	EVT_MENU( 0, WxAppContentServer::OnMenuScanForContent )
	EVT_MENU( 1, WxAppContentServer::OnMenuImportContent )
	EVT_MENU( 2, WxAppContentServer::OnMenuExit )
END_EVENT_TABLE();


//////////////////////////////////////////////////////////////////////////
// OnPostRmOpen
eEvtReturn OnPostRmOpen( EvtID, const SysSystemEvent& )
{
	// Listen for connection.
	RmCore::pImpl()->listen();

	//
	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// OnInit
//virtual
bool WxAppContentServer::OnInit()
{
	// Splash screen.
	wxSplashScreen* pSplash = new wxSplashScreen( wxBitmap( "Splash/splash.bmp", wxBITMAP_TYPE_BMP ), wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT, 3000, NULL, wxID_ANY );
	pSplash->Show();

	// Hook file search for importing.
#if 0 // neilogd: This will be triggered by a context menu
	SysSystemEvent::Delegate CsPreUpdateDelegate = SysSystemEvent::Delegate::bind< OnPreCsUpdate >();
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, CsPreUpdateDelegate );
#endif 

	// Hook remoting core post open for connection listening.
#if 0 // neilogd: Need to reimplement this!
	SysSystemEvent::Delegate RmPostOpenDelegate = SysSystemEvent::Delegate::bind< OnPostRmOpen >();
	RmCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, RmPostOpenDelegate );
#endif

	// Setup taskbar icon.
	pTaskBarIconImage_ = new wxIcon( "Splash/psybrus_logo_16x16.ico", wxBITMAP_TYPE_ICO );
	pTaskBarIcon_ = new wxTaskBarIcon();
	pTaskBarIcon_->SetIcon( *pTaskBarIconImage_, "Psybrus Content Server" ); 
		
	// Start up the kernel multithreaded.
	SysKernel::pImpl()->run( BcTrue );
	
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

//////////////////////////////////////////////////////////////////////////
// OnTaskBarRightDown
void WxAppContentServer::OnTaskBarRightDown( wxTaskBarIconEvent& Event )
{
	wxMenu* pMenu = new wxMenu( "Psybrus Content Server" );
	pMenu->Append( new wxMenuItem( pMenu, 0, "&Scan for Content" ) );
	pMenu->Append( new wxMenuItem( pMenu, 1, "&Import Content" ) );
	pMenu->Append( new wxMenuItem( pMenu, wxID_SEPARATOR ) );
	pMenu->Append( new wxMenuItem( pMenu, 2, "E&xit" ) );

	pTaskBarIcon_->PopupMenu( pMenu );
}

//////////////////////////////////////////////////////////////////////////
// OnMenuScanForContent
void WxAppContentServer::OnMenuScanForContent( wxCommandEvent& Event )
{
	std::list< BcPath > OutputFiles;
	
	// Search game content & engine content paths.
	FsCore::pImpl()->findFiles( "./EngineContent", BcTrue, BcFalse, OutputFiles );
	FsCore::pImpl()->findFiles( "./GameContent", BcTrue, BcFalse, OutputFiles );

	// Import all files that were found.
	for( std::list< BcPath >::iterator It( OutputFiles.begin() ); It != OutputFiles.end(); ++It )
	{
		const BcPath& Path = (*It);
		const BcName Extension = Path.getExtension();

		// Attempt to import resource, but don't force.
		CsCore::pImpl()->importResource( *Path, BcFalse );
	}
}

//////////////////////////////////////////////////////////////////////////
// OnMenuImportContent
void WxAppContentServer::OnMenuImportContent( wxCommandEvent& Event )
{
	std::list< BcPath > OutputFiles;
	
	// Search game content & engine content paths.
	FsCore::pImpl()->findFiles( "./EngineContent", BcTrue, BcFalse, OutputFiles );
	FsCore::pImpl()->findFiles( "./GameContent", BcTrue, BcFalse, OutputFiles );

	// Import all files that were found.
	for( std::list< BcPath >::iterator It( OutputFiles.begin() ); It != OutputFiles.end(); ++It )
	{
		const BcPath& Path = (*It);
		const BcName Extension = Path.getExtension();

		// Attempt to import resource.
		CsCore::pImpl()->importResource( *Path, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// OnMenuExit
void WxAppContentServer::OnMenuExit( wxCommandEvent& Event )
{
	Exit();
}
