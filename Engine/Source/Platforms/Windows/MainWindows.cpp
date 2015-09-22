#include <winsock2.h>

#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "System/Os/OsClientWindows.h"
#include "System/Os/OsMinidumpWindows.h"

#include "System/SysProfilerChromeTracing.h"

BcHandle GInstance_ = NULL;

eEvtReturn OnPreOsUpdate_PumpMessages( EvtID, const EvtBaseEvent& )
{
	MSG Msg;
#if PLATFORM_WINDOWS
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
#endif
	return evtRET_PASS;
}

eEvtReturn OnPostOpenScnCore_LaunchGame( EvtID, const EvtBaseEvent& )
{
	extern void PsyLaunchGame();
	PsyLaunchGame();

	return evtRET_REMOVE;
}

extern BcU32 GResolutionWidth;
extern BcU32 GResolutionHeight;

eEvtReturn OnPostOsOpen_CreateClient( EvtID, const EvtBaseEvent& )
{
#if PLATFORM_WINDOWS
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
		BcUnusedVar( pContext );
	}
#endif
	return evtRET_REMOVE;
}

int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow );

int main(int argc, char** argv)
{
#if COMPILER_MSVC && PLATFORM_WINDOWS
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
			PSY_LOG( "Could not initialise WinSock 2.2\n" );
		}
	}

	// Start.
	std::string CommandLine;
	for( int Idx = 0; Idx < argc; ++Idx )
	{
		if( strstr( argv[ Idx ], " " ) )
		{
			CommandLine += std::string( "\"" ) + argv[ Idx ] + std::string( "\"" );	
		}
		else
		{
			CommandLine += argv[ Idx ];
		}

		if( Idx != argc - 1 )
		{
			CommandLine += " ";
		}
	}

	return WinMain( NULL, NULL, (LPSTR)CommandLine.c_str(), 0 );
}

int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	( void )hPrevInstance;
	( void )lpCmdLine;
	( void )nCmdShow;

#if COMPILER_MSVC && PLATFORM_WINDOWS
	// Initialise minidumping as early as possible.
	if( OsMinidumpWindows::pImpl() == NULL )
	{
		new OsMinidumpWindows();
	}
#endif

#if PLATFORM_WINDOWS
	// Setup for more accurate timing.
	timeBeginPeriod( 1 );
#endif

	GInstance_ = (BcHandle)hInstance;


#if PLATFORM_WINDOWS
	// Set exe path.
	char ModuleFileName[ 4096 ] = { 0 };
	::GetModuleFileNameA( ::GetModuleHandle( nullptr ), ModuleFileName, sizeof( ModuleFileName ) - 1 );
	SysExePath_ = ModuleFileName;
#endif

	// Set command line params.
	SysArgs_ = lpCmdLine;

	// HACK: Append a space to sys args for find to work.
	SysArgs_ += " ";

	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLogImpl();
	}
#endif

	// Some default suppression.
	if( BcLog::pImpl() != nullptr )
	{
		BcLog::pImpl()->setCategorySuppression( "Reflection", BcTrue );
		// Setup basic log Category.
		BcLogScopedCategory LogCategory( "Main" );
	}

	// Initialise RNG.
#if !PSY_DEBUG
	BcRandom::Global = BcRandom( (BcU32)::GetTickCount() );
#endif

	// Create reflection database
	ReManager::Init();

#if PSY_USE_PROFILER
	// new profiler.
	new SysProfilerChromeTracing();
#endif

	// Unit tests prior to full kernel initialisation.
	if( SysArgs_.find( "-unittest " ) != std::string::npos )
	{
		extern void MainUnitTests();
		MainUnitTests();
		return 0;
	}

	// Create kernel.
	new SysKernel( GPsySetupParams.TickRate_ );

	// Register systems for creation.
#if PLATFORM_WINDOWS
	SYS_REGISTER( "OsCore", OsCoreImplWindows );
	SYS_REGISTER( "FsCore", FsCoreImplWindows );
#endif
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImpl );
	SYS_REGISTER( "SsCore", SsCoreImplSoLoud );
	SYS_REGISTER( "DsCore", DsCoreImpl );
	SYS_REGISTER( "DsCoreLogging", DsCoreLoggingImpl );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

	// Game or tool init.
	if( GPsySetupParams.Flags_ & psySF_GAME )
	{
		OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOsOpen_CreateClient );
		OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OnPreOsUpdate_PumpMessages );
		ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOpenScnCore_LaunchGame );

		// Init game.
		PsyGameInit();
	}
	else if( GPsySetupParams.Flags_ & psySF_TOOL )
	{
		extern void PsyToolInit();
		PsyToolInit();

		ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, 
		[]( EvtID, const EvtBaseEvent& )
			{
				extern void PsyToolMain();
				PsyToolMain();
				SysKernel::pImpl()->stop();
				return evtRET_REMOVE;
			} );
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

#if PLATFORM_WINDOWS
	timeEndPeriod( 1 );
#endif

	// Done.
	return 0;
}
