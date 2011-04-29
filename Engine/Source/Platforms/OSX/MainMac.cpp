#include "SysKernel.h"

#include <SDL.h>

int main( int argc, char* argv[] )
{
	// Create kernel.
	SysKernel Kernel;
	
	// Set command line params.
	SysArgc_ = (BcU32)argc;
	SysArgv_ = (BcChar**)&argv[0];
	
	// Register systems for creation.
	SYS_REGISTER( &Kernel, GaCore );
	SYS_REGISTER( &Kernel, RmCore );
	SYS_REGISTER( &Kernel, OsCoreImplSDL );
	SYS_REGISTER( &Kernel, FsCoreImplOSX );
	SYS_REGISTER( &Kernel, CsCoreClient );
	SYS_REGISTER( &Kernel, RsCoreImplGL );
	SYS_REGISTER( &Kernel, SsCoreALInternal );
	
	// Pass over to user for additional configuration
	extern void PsyGameInit( SysKernel& Kernel );
	PsyGameInit( Kernel );
	
	// No need to unbind these delegates.
	return 0;
}

// TEST BED STUFF...
#include "SysKernel.h"
#include "OsCore.h"
#include "CsCore.h"
#include "FsCore.h"
#include "RsCore.h"
#include "RmCore.h"
#include "GaCore.h"

#include "CsFile.h"

#include "RsCoreImplGL.h"

#include "ScnTexture.h"
#include "ScnTextureAtlas.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnCanvas.h"
#include "ScnShader.h"
#include "ScnScript.h"
#include "ScnFont.h"
#include "ScnPackage.h"
#include "ScnSound.h"
#include "ScnSoundEmitter.h"

OsEventCore::Delegate DelegateQuit;
SysSystemEvent::Delegate DelegateRender;
SysSystemEvent::Delegate DelegateUpdate;
SysSystemEvent::Delegate DelegateRemoteOpened;

eEvtReturn doQuit( EvtID ID, const OsEventCore& Event )
{
	// Stop all systems and quit.
	OsCore::pImpl()->pKernel()->stop();
	
	//
	return evtRET_PASS;
}

eEvtReturn doUpdate( EvtID ID, const SysSystemEvent& Event )
{

	return evtRET_PASS;
}

eEvtReturn doRender( EvtID ID, const SysSystemEvent& Event )
{
	
	return evtRET_PASS;
}

eEvtReturn doRemoteOpened( EvtID, const SysSystemEvent& Event )
{
	//RmCore::pImpl()->connect( "localhost" );
	
	return evtRET_PASS;
}

void PsyGameInit( SysKernel& Kernel )
{
	// TODO: Move this into a types unit test.
	BcAssert( sizeof( BcU8 ) == 1 );
	BcAssert( sizeof( BcS8 ) == 1 );
	BcAssert( sizeof( BcU16 ) == 2 );
	BcAssert( sizeof( BcS16 ) == 2 );
	BcAssert( sizeof( BcU32 ) == 4 );
	BcAssert( sizeof( BcS32 ) == 4 );
	BcAssert( sizeof( BcU64 ) == 8 );
	BcAssert( sizeof( BcS64 ) == 8 );
	BcAssert( sizeof( BcF32 ) == 4 );
	BcAssert( sizeof( BcF64 ) == 8 );
	
	//
	extern void BcAtomic_UnitTest();
	BcAtomic_UnitTest();
	
	// Start up systems.
	Kernel.startSystem( "RmCore" );
	Kernel.startSystem( "OsCoreImplSDL" );
	Kernel.startSystem( "RsCoreImplGL" );
	Kernel.startSystem( "SsCoreALInternal" );
	Kernel.startSystem( "FsCoreImplOSX" );
	Kernel.startSystem( "CsCoreClient" );
	Kernel.startSystem( "GaCore" );
	
	// Bind delegates
	DelegateQuit = OsEventCore::Delegate::bind< doQuit >();
	DelegateRender = SysSystemEvent::Delegate::bind< doRender >();	
	DelegateUpdate = SysSystemEvent::Delegate::bind< doUpdate >();	
	DelegateRemoteOpened = SysSystemEvent::Delegate::bind< doRemoteOpened >();	
		
	// Hook engine events to begin processing.
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT,			DelegateQuit );
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateUpdate );
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateRender );
	RmCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,	DelegateRemoteOpened );	
	
	// Register resources.
	CsCore::pImpl()->registerResource< ScnTexture >();
	CsCore::pImpl()->registerResource< ScnTextureAtlas >();
	CsCore::pImpl()->registerResource< ScnMaterial >();
	CsCore::pImpl()->registerResource< ScnMaterialInstance >();
	CsCore::pImpl()->registerResource< ScnModel >();
	CsCore::pImpl()->registerResource< ScnModelInstance >();
	CsCore::pImpl()->registerResource< ScnCanvas >();
	CsCore::pImpl()->registerResource< ScnShader >();
	CsCore::pImpl()->registerResource< ScnFont >();
	CsCore::pImpl()->registerResource< ScnFontInstance >();
	CsCore::pImpl()->registerResource< ScnPackage >();
	CsCore::pImpl()->registerResource< ScnScript >();
	CsCore::pImpl()->registerResource< ScnSound >();
	CsCore::pImpl()->registerResource< ScnSoundEmitter >();

	// Run the kernel.
	Kernel.run();
}


