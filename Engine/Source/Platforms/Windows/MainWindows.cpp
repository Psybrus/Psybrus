#include "MainShared.h"
#include "SysKernel.h"
#include "OsCore.h"

#include "OsWindowWindows.h"

BcHandle GWindowDC_ = NULL;
BcHandle GWindowRC_ = NULL;

eEvtReturn OnPreOsUpdate( EvtID, const SysSystemEvent& )
{
	MSG Msg;

	while( ::PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) == TRUE )
	{
		::TranslateMessage( &Msg );
		::DispatchMessage( &Msg );

		// Check for quit.
		if( Msg.message == WM_QUIT )
		{
			// Stop, join, and free kernel.
			SysKernel::pImpl()->stop();
		}
	}

	return evtRET_PASS;
}

int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	( void )hPrevInstance;
	( void )lpCmdLine;
	( void )nCmdShow;

	// Perform unit tests.
	MainUnitTests();
	
	// Set command line params.
	SysArgs_ = lpCmdLine;
	
	// Create kernel.
	new SysKernel();
	
	// Register systems for creation.
	SYS_REGISTER( "GaCore", GaCore );
	//SYS_REGISTER( "RmCore", RmCore );
	SYS_REGISTER( "OsCore", OsCoreImplSDL );
	SYS_REGISTER( "FsCore", FsCoreImplWindows );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImplGL );
	SYS_REGISTER( "SsCore", SsCoreImplAL );
	
	// Create window.
	OsWindowWindows MainWindow;
	if( MainWindow.create( "Psybrus Testbed", (BcHandle)hInstance, 1280, 720, BcFalse ) == BcFalse )
	{
		BcPrintf( "Failed to create window!\n" );
		return 1;
	}

	// Cache handle globally.
	// TODO: OsViewContext class to expose these.
	GWindowDC_ = MainWindow.getDC();
	
	// Main shared.
	MainShared();

	// Game init.
	extern void PsyGameInit();
	PsyGameInit();

	// Hook up event pump delegate.
	SysSystemEvent::Delegate OsPreUpdateDelegate = SysSystemEvent::Delegate::bind< OnPreOsUpdate >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OsPreUpdateDelegate );

	// Run kernel unthreaded.
	SysKernel::pImpl()->run( BcFalse );

	// Delete kernel.
	delete SysKernel::pImpl();

	// Done.
	return 0;
}

void PsyGameInit()
{
	// Do game init here.
}
