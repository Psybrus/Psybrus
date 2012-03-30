#include "MainShared.h"
#include "System/SysKernel.h"
#include "System/Os/OsCore.h"

#include "System/Os/OsClientWindows.h"
#include "System/Os/OsMinidumpWindows.h"

BcHandle GInstance_ = NULL;

eEvtReturn OnPreOsUpdate_PumpMessages( EvtID, const SysSystemEvent& )
{
	MSG Msg;

	while( ::PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) == TRUE )
	{
		::TranslateMessage( &Msg );
		::DispatchMessage( &Msg );

		// Check for quit.
		if( Msg.message == WM_QUIT )
		{
			// Send event to quit.
			OsCore::pImpl()->publish( osEVT_CORE_QUIT, OsEventCore() );
		}
	}

	return evtRET_PASS;
}

extern BcU32 GResolutionWidth;
extern BcU32 GResolutionHeight;

eEvtReturn OnPostOsOpen_CreateClient( EvtID, const SysSystemEvent& )
{
	OsClientWindows* pMainWindow = new OsClientWindows();
	if( pMainWindow->create( GPsySetupParams.Name_.c_str(), GInstance_, GResolutionWidth, GResolutionHeight, BcFalse, GPsySetupParams.Flags_ & psySF_WINDOW ? BcTrue : BcFalse ) == BcFalse )
	{
		BcAssertMsg( BcFalse, "Failed to create client!" );
		return evtRET_REMOVE;
	}

	// Get rendering context.
	if( RsCore::pImpl() != NULL )
	{
		RsContext* pContext = RsCore::pImpl()->getContext( pMainWindow );
		BcAssertMsg( pContext != NULL, "Failed to create render context!" );
	}

	return evtRET_REMOVE;
}

int main(int argc, char** argv)
{
#if COMPILER_MSVC
	if( OsMinidumpWindows::pImpl() == NULL )
	{
		new OsMinidumpWindows();
	}
#endif

	static BcBool IsInitialised = BcFalse;
	if( IsInitialised == BcFalse )
	{
		WSADATA WsaData;
		int err = ::WSAStartup( MAKEWORD(2,2), &WsaData );
		if( err == 0 )
		{
			IsInitialised = BcTrue;
		}
		else
		{
			BcPrintf( "Could not initialise WinSock 2.2\n" );
		}
	}

	// Start.
	std::string CommandLine;

	for( int Idx = 0; Idx < argc; ++Idx )
	{
		CommandLine += argv[ Idx ];
		CommandLine += " ";
	}

	return WinMain( NULL, NULL, (LPSTR)CommandLine.c_str(), 0 );
}

int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	( void )hPrevInstance;
	( void )lpCmdLine;
	( void )nCmdShow;

#if COMPILER_MSVC
	// Initialise minidumping as early as possible.
	if( OsMinidumpWindows::pImpl() == NULL )
	{
		new OsMinidumpWindows();
	}
#endif

	// Setup for more accurate timing.
	timeBeginPeriod( 1 );

	GInstance_ = (BcHandle)hInstance;

	// Set command line params.
	SysArgs_ = lpCmdLine;

	// HACK: Append a space to sys args for find to work.
	SysArgs_ += " ";

	// Setup log if we have a commandline for it.
	if( SysArgs_.find( "-log " ) != std::string::npos )
	{
		// TODO: File name generation.
		new BcLogFile( "log.txt" );
	}

	// Perform unit tests.
	MainUnitTests();

	// Create kernel.
	new SysKernel( GPsySetupParams.TickRate_ );

	// Register systems for creation.
	SYS_REGISTER( "RmCore", RmCore );
	SYS_REGISTER( "OsCore", OsCoreImplWindows );
	SYS_REGISTER( "FsCore", FsCoreImplWindows );
	SYS_REGISTER( "CsCore", CsCore );
#ifndef PSY_PRODUCTION
	SYS_REGISTER( "CsCoreClient", CsCoreClient );
#if PSY_SERVER
	SYS_REGISTER( "CsCoreServer", CsCoreServer );
#endif
#endif
	SYS_REGISTER( "RsCore", RsCoreImplGL );
	SYS_REGISTER( "SsCore", SsCoreImplAL );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

	// Hook up create client delegate
	SysSystemEvent::Delegate OsPostOpenDelegateCreateClient = SysSystemEvent::Delegate::bind< OnPostOsOpen_CreateClient >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OsPostOpenDelegateCreateClient );

	// Hook up event pump delegate.
	SysSystemEvent::Delegate OsPreUpdateDelegatePumpMessages = SysSystemEvent::Delegate::bind< OnPreOsUpdate_PumpMessages >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OsPreUpdateDelegatePumpMessages );

	// Game init.
	PsyGameInit();

	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLog();
	}
#endif

	if( ( GPsySetupParams.Flags_ & psySF_MANUAL ) == 0 )
	{
		// Run kernel unthreaded.
		SysKernel::pImpl()->run( BcFalse );
	}

	// Delete kernel.
	delete SysKernel::pImpl();

	// Delete log.
	delete BcLog::pImpl();

	//
	timeEndPeriod( 1 );

	// Done.
	return 0;
}
