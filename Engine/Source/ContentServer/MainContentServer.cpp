/**************************************************************************
*
* File:		MainContentServer.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main Content Server Entrypoint.
*		
*
*
* 
**************************************************************************/

#include "Psybrus.h"

#include "WxAppContentServer.h"

#include "CsProperty.h"

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Psybrus Content Server", psySF_SERVER | psySF_MANUAL, 1.0f / 15.0f );

//////////////////////////////////////////////////////////////////////////
// PsyGameInit
void PsyGameInit()
{
	// Create app.
	WxAppContentServer* pApp = new WxAppContentServer(); 
	wxApp::SetInstance( pApp );

	// wxWidgets entry.
	wxEntry( NULL, NULL, (char*)SysArgs_.c_str(), 1 );
}
