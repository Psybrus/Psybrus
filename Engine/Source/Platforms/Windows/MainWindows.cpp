#include <winsock2.h>

#include "MainShared.h"

#include "Base/BcLog.h"
#include "Base/BcLogFile.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "System/Os/OsClientWindows.h"
#include "System/Os/OsMinidumpWindows.h"

#include "System/SysProfilerChromeTracing.h"

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

eEvtReturn OnPostOpenScnCore_LaunchGame( EvtID, const SysSystemEvent& )
{
	extern void PsyLaunchGame();
	PsyLaunchGame();

	return evtRET_REMOVE;
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

// HACK HACK HACK: Offline package importing is a major hack for now.
eEvtReturn OnPostCsOpen_ImportPackages( EvtID, const SysSystemEvent& )
{
	WIN32_FIND_DATA FindFileData;
	HANDLE Handle = ::FindFirstFileA( "Content/*.pkg", &FindFileData );

	if( Handle != INVALID_HANDLE_VALUE )
	{
		do
		{
			BcPath PackagePath( FindFileData.cFileName );
			CsPackage* pPackage = new CsPackage( PackagePath.getFileNameNoExtension() );

			// HACK: Package importing is a major hack currently so we can automate offline building and packaging for LD25.
			//       The system is due a change soon, this is a purely temporary measure until we break out importing into
			//       a seperate tool.
			// delete pPackage;
		}
		while(::FindNextFileA( Handle, &FindFileData ));

		::FindClose( Handle );
	}

	// HACK: We just wanna bail here. No clean shutdown yet.
	exit(0);
	
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

	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLog();
	}
#endif

	// Initialise RNG.
#if !PSY_DEBUG
	BcRandom::Global = BcRandom( (BcU32)::GetTickCount() );
#endif

	// Create reflection database
	new BcReflection();

#if PSY_USE_PROFILER
	// new profiler.
	new SysProfilerChromeTracing();
#endif

	// Create kernel.
	new SysKernel( GPsySetupParams.TickRate_ );

	// Register systems for creation.
	SYS_REGISTER( "DsCore", DsCore );
	SYS_REGISTER( "OsCore", OsCoreImplWindows );
	SYS_REGISTER( "FsCore", FsCoreImplWindows );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImplGL );
	SYS_REGISTER( "SsCore", SsCoreImplAL );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

#if !PSY_PRODUCTION
	// Perform unit tests.
	MainUnitTests();
#endif

	// HACK HACK HACK: Offline package importing is a major hack for now.
	if( SysArgs_.find( "ImportPackages" ) == std::string::npos )
	{
		// Hook up create client delegate
		SysSystemEvent::Delegate OsPostOpenDelegateCreateClient = SysSystemEvent::Delegate::bind< OnPostOsOpen_CreateClient >();
		OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OsPostOpenDelegateCreateClient );

		// Hook up event pump delegate.
		SysSystemEvent::Delegate OsPreUpdateDelegatePumpMessages = SysSystemEvent::Delegate::bind< OnPreOsUpdate_PumpMessages >();
		OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OsPreUpdateDelegatePumpMessages );

		SysSystemEvent::Delegate OnPostOpenDelegateLaunchGame = SysSystemEvent::Delegate::bind< OnPostOpenScnCore_LaunchGame >();
		ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOpenDelegateLaunchGame );

		// Init game.
		PsyGameInit();
	}
	else
	{
		// HACK HACK HACK: Offline package importing is a major hack for now.
		SysSystemEvent::Delegate OsPostOpenDelegateImportPackages = SysSystemEvent::Delegate::bind< OnPostCsOpen_ImportPackages >();
		CsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OsPostOpenDelegateImportPackages );
	}

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
