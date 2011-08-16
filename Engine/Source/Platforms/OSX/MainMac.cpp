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

#include "SysJobQueue.h"

OsEventCore::Delegate DelegateQuit;
SysSystemEvent::Delegate DelegateRender;
SysSystemEvent::Delegate DelegateUpdate;
SysSystemEvent::Delegate DelegateRemoteOpened;
SysSystemEvent::Delegate DelegateSetupEngine;

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

eEvtReturn doSetupEngine( EvtID, const SysSystemEvent& Event )
{
	OsCore::pImpl()->pKernel()->startSystem( "RmCore" );
	OsCore::pImpl()->pKernel()->startSystem( "RsCoreImplGL" );
	OsCore::pImpl()->pKernel()->startSystem( "SsCoreALInternal" );
	OsCore::pImpl()->pKernel()->startSystem( "FsCoreImplOSX" );
	OsCore::pImpl()->pKernel()->startSystem( "CsCoreClient" );
	OsCore::pImpl()->pKernel()->startSystem( "GaCore" );

	// Bind delegates
	DelegateQuit = OsEventCore::Delegate::bind< doQuit >();
	DelegateRender = SysSystemEvent::Delegate::bind< doRender >();	
	DelegateUpdate = SysSystemEvent::Delegate::bind< doUpdate >();	
	DelegateRemoteOpened = SysSystemEvent::Delegate::bind< doRemoteOpened >();	

	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT,			DelegateQuit );
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateUpdate );
	RsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE,	DelegateRender );
	RmCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,	DelegateRemoteOpened );	
	
	// Register resources.
	CsCore::pImpl()->registerResource< ScnTexture >();
	CsCore::pImpl()->registerResource< ScnTextureAtlas >();
	CsCore::pImpl()->registerResource< ScnRenderTarget >();
	CsCore::pImpl()->registerResource< ScnMaterial >();
	CsCore::pImpl()->registerResource< ScnMaterialInstance >();
	CsCore::pImpl()->registerResource< ScnModel >();
	CsCore::pImpl()->registerResource< ScnModelInstance >();
	CsCore::pImpl()->registerResource< ScnCanvas >();
	CsCore::pImpl()->registerResource< ScnShader >();
	CsCore::pImpl()->registerResource< ScnFont >();
	CsCore::pImpl()->registerResource< ScnFontInstance >();
	CsCore::pImpl()->registerResource< ScnSound >();
	CsCore::pImpl()->registerResource< ScnSoundEmitter >();

	CsCore::pImpl()->registerResource< GaPackage >();
	CsCore::pImpl()->registerResource< GaScript >();

	return evtRET_PASS;
}

class BcDelegateDispatcher
{
public:
	virtual void enqueueDelegateCall( BcDelegateCallBase* pDelegateCall ) = 0;
	virtual void dispatch() = 0;	

public:
	template< typename _Ty >
	BcForceInline void enqueue( _Ty& DelegateCall )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall() );
		enqueueDelegateCall( pDelegateCall );
	}

	template< typename _Ty, typename _P0 >
	BcForceInline void enqueue( _Ty& DelegateCall, _P0 P0 )
	{
		_Ty* pDelegateCall = new _Ty( DelegateCall.deferCall( P0 ) );
		enqueueDelegateCall( pDelegateCall );
	}
};


void testDelegateCall( int a )
{
	BcPrintf( "a: %i\n", a );
}

void testDelegateCall2( int a, int b )
{
	BcPrintf( "ab: %i, %i\n", a, b );
}

void PsyGameInit( SysKernel& Kernel )
{
	/*
	// Test code.
	typedef BcDelegate< void(*)(int) > TDelegate;
	typedef BcDelegateCall< void(*)(int) > TDelegateCall;

	typedef BcDelegate< void(*)(int,int) > TDelegate2;
	typedef BcDelegateCall< void(*)(int,int) > TDelegateCall2;
	
	BcDelegateDispatcher DelegateCallDispatcher;
	
	TDelegateCall DelegateCall( TDelegate::bind< testDelegateCall >() );
	TDelegateCall2 DelegateCall2( TDelegate2::bind< testDelegateCall2 >() );
	
	DelegateCallDispatcher.queueCall( TDelegate::bind< testDelegateCall >(), 0 );
	DelegateCallDispatcher.queueCall( TDelegate::bind< testDelegateCall >(), 1 );
	DelegateCallDispatcher.queueCall( TDelegate2::bind< testDelegateCall2 >(), 2, 3 );
	DelegateCallDispatcher.queueCall( TDelegate2::bind< testDelegateCall2 >(), 4, 5 );

	DelegateCallDispatcher.dispatch();
	 */
	
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
	
	// Start up OS system.
	Kernel.startSystem( "OsCoreImplSDL" );

	// Setup hook to start other systems when OS has been initialised.
	DelegateSetupEngine = SysSystemEvent::Delegate::bind< doSetupEngine >();	
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN,	DelegateSetupEngine );

	// Run the kernel.
	Kernel.run();
}


