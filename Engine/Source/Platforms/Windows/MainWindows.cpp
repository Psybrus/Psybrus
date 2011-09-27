#include "MainShared.h"
#include "SysKernel.h"

#include "OsWindowWindows.h"

BcHandle GWindowDC_ = NULL;
BcHandle GWindowRC_ = NULL;

int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	( void )hPrevInstance;
	( void )lpCmdLine;
	( void )nCmdShow;

	// Perform unit tests.
	MainUnitTests();
	
	// Set command line params.
	//SysArgc_ = (BcU32)argc;
	//SysArgv_ = (BcChar**)&argv[0];
	
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
	GWindowDC_ = MainWindow.getDC();

	// Main shared.
	MainShared();

	// Game init.
	extern void PsyGameInit();
	PsyGameInit();

	// Run kernel threaded.
	SysKernel::pImpl()->run( BcTrue );

	// Do event pump.
	for(;;)
	{
		MSG Msg;
		
		while( ::PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) == TRUE )
		{
			::TranslateMessage( &Msg );
			::DispatchMessage( &Msg );
		}

		BcYield();
	}

	// Done.
	return 0;
}

void PsyGameInit()
{
	// Do game init here.
}

