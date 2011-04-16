#include <iostream>

#include "RmTCPConnection.h"
#include "BcThread.h"

#include "RmCore.h"
#include "CsCore.h"
#include "BcHash.h"

#include "SysKernel.h"

#include "ScnTexture.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnScript.h"
#include "ScnCanvas.h"
#include "ScnShader.h"
#include "ScnPackage.h"

SysSystemEvent::Delegate DelegateUpdate;
SysSystemEvent::Delegate DelegateRemoteOpened;
RmEventCore::Delegate DelegateRemoteDisconnected;

ScnModelRef Model;
ScnScriptRef Script;
ScnMaterialRef Material;
ScnPackageRef Package;

eEvtReturn doUpdate( EvtID ID, const SysSystemEvent& Event )
{
	//*
	if( Package.isValid() == BcFalse )
	{
		CsCore::pImpl()->registerResource< ScnTexture >();
		CsCore::pImpl()->registerResource< ScnMaterial >();
		CsCore::pImpl()->registerResource< ScnModel >();
		CsCore::pImpl()->registerResource< ScnScript >();
		CsCore::pImpl()->registerResource< ScnCanvas >();
		CsCore::pImpl()->registerResource< ScnShader >();
		CsCore::pImpl()->registerResource< ScnPackage >();
		CsCore::pImpl()->importResource( "test.package", Package );
	}
	
	//*/
	return evtRET_PASS;
}

eEvtReturn doRemoteOpened( EvtID, const SysSystemEvent& Event )
{
	// Listen for a connection.
	//RmCore::pImpl()->listen();
	return evtRET_PASS;
}

eEvtReturn doRemoteDisconnected( EvtID, const RmEventCore& Event )
{
	// Listen for a connection.
	//RmCore::pImpl()->listen();
	return evtRET_PASS;
}

int main (int argc, char * const argv[])
{
	// Start up systems.
	SysKernel Kernel;
	
	SYS_REGISTER( &Kernel, FsCoreImplOSX );
	SYS_REGISTER( &Kernel, RmCore );
	SYS_REGISTER( &Kernel, CsCoreServer );

	Kernel.startSystem( "FsCoreImplOSX" );
	Kernel.startSystem( "RmCore" );
	Kernel.startSystem( "CsCoreServer" );
	
	// Bind delegates
	DelegateUpdate = SysSystemEvent::Delegate::bind< doUpdate >();	
	DelegateRemoteOpened = SysSystemEvent::Delegate::bind< doRemoteOpened >();	
	DelegateRemoteDisconnected = RmEventCore::Delegate::bind< doRemoteDisconnected >();	
	
	// Hook engine events to begin processing.
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,		DelegateUpdate );
	RmCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,		DelegateRemoteOpened );
	RmCore::pImpl()->subscribe( rmEVT_CONNECTION_DISCONNECTED,	DelegateRemoteDisconnected );
	
	// Run the kernel.
	Kernel.run();	
		
	return 0;
}
