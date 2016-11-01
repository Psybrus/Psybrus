#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Game", psySF_GAME_DEV, 1.0f / 60.0f );	

//////////////////////////////////////////////////////////////////////////
// PsyToolInit
void PsyToolInit()
{

}

//////////////////////////////////////////////////////////////////////////
// PsyToolMain
void PsyToolMain()
{

}

//////////////////////////////////////////////////////////////////////////
// Android entry point
#if PLATFORM_ANDROID
#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsClientAndroid.h"
#include "System/Os/OsCore.h"
#include "System/Renderer/RsCore.h"

#include <android_native_app_glue.h>
#include <android/log.h>

#include <errno.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <iostream>

struct android_app* GAndroidApp = nullptr;

static OsClientAndroid* GMainWindow = nullptr;

static struct sigaction OldSignalHandler[ NSIG ];

void android_sigaction( int Signal, siginfo_t* Info, void* Reserved )
{
	__android_log_print( ANDROID_LOG_INFO, "Psybrus", "Caught signal. Exiting." );	
	BcPrintBacktrace( BcBacktrace() );
	OldSignalHandler[ Signal ].sa_handler( Signal );
	exit(1);
}

extern void AutoGenRegisterReflection();

void PsyAndroidMain( struct android_app* State )
{
	extern void engine_dummy();
	engine_dummy();

	// Do not remove this code. It makes libgpg link (wtf inorite?)
	auto start = std::chrono::steady_clock::now();
	std::cerr << "whatever";

	static bool IsInitialised = false;
	BcAssertMsg( IsInitialised == false, "Need to implement second tick through android_main." );
	IsInitialised = BcTrue;

	// Make sure glue isn't stripped.
	app_dummy();

#if 0
	// Catch signals.
	struct sigaction Handler;
	memset( &Handler, 0, sizeof( Handler ) );
	Handler.sa_sigaction = android_sigaction;
	Handler.sa_flags = SA_RESETHAND;
#define CATCHSIG( X ) sigaction( X, &Handler, &OldSignalHandler[ X ] )
	CATCHSIG( SIGILL );
	CATCHSIG( SIGABRT );
	CATCHSIG( SIGBUS );
	CATCHSIG( SIGFPE );
	CATCHSIG( SIGSEGV );
	CATCHSIG( SIGSTKFLT );
	CATCHSIG( SIGPIPE );
#undef CATCHSIG
#endif

	GAndroidApp = State;

	// Set game thread to be this one. 
	BcSetGameThread();

	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLogImpl();
	}

	// Some default suppression.
	if( BcLog::pImpl() != nullptr )
	{
		BcLog::pImpl()->setCategorySuppression( "Reflection", BcTrue );

		// Render & sound are noisy.
		BcLog::pImpl()->setCategorySuppression( "RsCore", BcTrue );
		BcLog::pImpl()->setCategorySuppression( "SsCore", BcTrue );
		
	}

	// Setup basic log Category.
	BcLogScopedCategory LogCategory( "Main" );
#endif

	PSY_LOG( "Psybrus for Android." );
	// Initialise RNG.
#if !PSY_DEBUG
	BcRandom::Global = BcRandom( 1337 ); // TODO LINUX
#endif

	PSY_LOG( "Init reflection" );

	// Create reflection database
	ReManager::Init();

	// Create kernel.
	new SysKernel( GPsySetupParams.TickRate_ );

	// Register systems for creation.
	// TODO: Load from config or similar?
	SYS_REGISTER( "OsCore", OsCoreImplAndroid );
	SYS_REGISTER( "FsCore", FsCoreImplAndroid );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImpl );
	SYS_REGISTER( "SsCore", SsCoreImplSoLoud );
	SYS_REGISTER( "DsCore", DsCoreImpl );
	SYS_REGISTER( "DsCoreLogging", DsCoreLoggingImpl );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

	// Create client event.
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, 
		[]( EvtID, const EvtBaseEvent& )
		{
			GMainWindow = new OsClientAndroid( GAndroidApp );
			std::string Title = ( GPsySetupParams.Name_ + std::string( " (" ) + GCommandLine_.c_str() + std::string( ")" ) );
			
			if( GMainWindow->create( Title.c_str() ) == BcFalse )
			{
				BcAssertMsg( BcFalse, "Failed to create client!" );
				return evtRET_REMOVE;
			} 

			// Get rendering context.
			if( RsCore::pImpl() != NULL )
			{
				RsContext* pContext = RsCore::pImpl()->getContext( GMainWindow );
				BcAssertMsg( pContext != NULL, "Failed to create render context!" );
			}

			return evtRET_REMOVE;
		} );

	// Create launch game event.
	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, 
		[]( EvtID, const EvtBaseEvent& )
		{
			PsyLaunchGame();
			return evtRET_REMOVE;
		} );


	// Setup callback for post CsCore open for resource registration.
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, 
		[]( EvtID, const EvtBaseEvent& )
		{
			// Register reflection.
			AutoGenRegisterReflection();
			return evtRET_REMOVE;
		} );

	// Init game.
	PsyGameInit();
	
	// Run kernel.
	if( ( GPsySetupParams.Flags_ & psySF_MANUAL ) == 0 )
	{
		// Run kernel unthreaded.
		SysKernel::pImpl()->run( BcFalse );
	}

	// Delete kernel.
	delete SysKernel::pImpl();

	// Delete log.
	delete BcLog::pImpl();
}

extern "C"
{
	void android_main( struct android_app* State )
	{
		PsyAndroidMain( State );
	}
}

int main()
{
	return 1;
}

#endif // PLATFORM_ANDROID

//////////////////////////////////////////////////////////////////////////
// HTML5 entry point
#if PLATFORM_HTML5

#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Os/SDL/OsClientSDL.h"
#include "System/Scene/ScnCore.h"
#include "System/Renderer/RsCore.h"

#include <emscripten.h>

extern void AutoGenRegisterReflection();

extern BcU32 GResolutionWidth;
extern BcU32 GResolutionHeight;

void emscriptenMain()
{
	SysKernel::pImpl()->runOnce();
}

int main(int argc, char** argv)
{
	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLogImpl();
	}
	// Some default suppression.
	BcLog::pImpl()->setCategorySuppression( "Reflection", BcTrue );

	// Setup basic log Category.
	BcLogScopedCategory LogCategory( "Main" );

#endif

	// Initialise RNG.
#if !PSY_DEBUG
	BcRandom::Global = BcRandom( 1337 ); // TODO LINUX
#endif

	// Create reflection database
	ReManager::Init();

	// Create kernel.
	new SysKernel( GPsySetupParams.TickRate_ );

	// Register systems for creation.
	SYS_REGISTER( "OsCore", OsCoreImplSDL );
	SYS_REGISTER( "FsCore", FsCoreImplHTML5 );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImpl );
	SYS_REGISTER( "SsCore", SsCoreImplSoLoud );
	SYS_REGISTER( "DsCore", DsCoreImpl );
	SYS_REGISTER( "DsCoreLogging", DsCoreLoggingImpl );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

	// Hook up create client delegate
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, 
		[]( EvtID, const EvtBaseEvent& )
		{
			OsClientSDL* pMainWindow = new OsClientSDL();
			if( pMainWindow->create( GPsySetupParams.Name_.c_str(), 0, GResolutionWidth, GResolutionHeight, BcFalse, GPsySetupParams.Flags_ & psySF_WINDOW ? BcTrue : BcFalse ) == BcFalse )
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
		} );

	// Create launch game event.
	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, 
		[]( EvtID, const EvtBaseEvent& )
		{
			PsyLaunchGame();
			return evtRET_REMOVE;
		} );


	// Setup callback for post CsCore open for resource registration.
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, 
		[]( EvtID, const EvtBaseEvent& )
		{
			// Register reflection.
			AutoGenRegisterReflection();
			return evtRET_REMOVE;
		} );

	// Init game.
	PsyGameInit();

	// Setup main loop.
	//emscripten_set_main_loop( 
	//	emscriptenMain, static_cast< int >( 1.0f / GPsySetupParams.TickRate_ ), 0 );
	emscripten_set_main_loop( emscriptenMain, 0, 0 );

	// Debug end.
	PSY_LOG( "Done main initialisation.\n" );

	return 0;
}

#endif