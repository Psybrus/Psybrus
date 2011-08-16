#include <iostream>

#include "RmTCPConnection.h"
#include "BcThread.h"

#include "RmCore.h"
#include "CsCore.h"
#include "BcHash.h"

#include "SysKernel.h"

#include "ScnTexture.h"
#include "ScnTextureAtlas.h"
#include "ScnRenderTarget.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnCanvas.h"
#include "ScnShader.h"
#include "ScnFont.h"
#include "ScnSound.h"
#include "ScnSoundEmitter.h"

#include "GaPackage.h"
#include "GaScript.h"

SysSystemEvent::Delegate DelegateUpdate;
SysSystemEvent::Delegate DelegateRemoteOpened;
RmEventCore::Delegate DelegateRemoteDisconnected;

ScnModelRef Model;
ScnMaterialRef Material;
GaPackageRef Package;
GaScriptRef Script;
ScnFontRef Font;
eEvtReturn doUpdate( EvtID ID, const SysSystemEvent& Event )
{
	//*
	if( Package.isValid() == BcFalse )
	{
		CsCore::pImpl()->registerResource< ScnTexture >();
		CsCore::pImpl()->registerResource< ScnTextureAtlas >();
		CsCore::pImpl()->registerResource< ScnRenderTarget >();
		CsCore::pImpl()->registerResource< ScnMaterial >();
		CsCore::pImpl()->registerResource< ScnModel >();
		CsCore::pImpl()->registerResource< ScnCanvas >();
		CsCore::pImpl()->registerResource< ScnShader >();
		CsCore::pImpl()->registerResource< ScnFont >();
		CsCore::pImpl()->registerResource< ScnSound >();
		CsCore::pImpl()->registerResource< ScnSoundEmitter >();

		CsCore::pImpl()->registerResource< GaPackage >();
		CsCore::pImpl()->registerResource< GaScript >();

		CsCore::pImpl()->importResource( "GameContent/boot.package", Package );
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
	
	SYS_REGISTER( &Kernel, RmCore );
	SYS_REGISTER( &Kernel, FsCoreImplOSX );
	SYS_REGISTER( &Kernel, CsCoreServer );

	Kernel.startSystem( "RmCore" );
	Kernel.startSystem( "FsCoreImplOSX" );
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
