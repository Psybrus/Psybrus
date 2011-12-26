#include "MainShared.h"
#include "SysKernel.h"
#include "OsCore.h"

#include "OsClientWindows.h"

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
	new SysKernel( GPsySetupParams.TickRate_ );
	
	// Register systems for creation.
	SYS_REGISTER( "RmCore", RmCore );
	SYS_REGISTER( "OsCore", OsCoreImplWindows );
	SYS_REGISTER( "FsCore", FsCoreImplWindows );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImplGL );
	SYS_REGISTER( "SsCore", SsCoreImplAL );
	
	// Create window.
	if( GPsySetupParams.Flags_ & psySF_WINDOW )
	{
		OsClientWindows MainWindow;
		if( MainWindow.create( GPsySetupParams.Name_.c_str(), (BcHandle)hInstance, GResolutionWidth, GResolutionHeight, BcFalse ) == BcFalse )
		{
			BcPrintf( "Failed to create window!\n" );
			return 1;
		}

		// Cache handle globally.
		// TODO: OsViewContext class to expose these.
		GWindowDC_ = MainWindow.getDC();
	}
	
	// Main shared.
	MainShared();

	// Hook up event pump delegate.
	if( GPsySetupParams.Flags_ & psySF_WINDOW )
	{	
		SysSystemEvent::Delegate OsPreUpdateDelegate = SysSystemEvent::Delegate::bind< OnPreOsUpdate >();
		OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OsPreUpdateDelegate );
	}

	// Game init.
	PsyGameInit();

	if( ( GPsySetupParams.Flags_ & psySF_MANUAL ) == 0 )
	{
		// Run kernel unthreaded.
		SysKernel::pImpl()->run( BcFalse );
	}

	// Delete kernel.
	delete SysKernel::pImpl();

	// Done.
	return 0;
}
