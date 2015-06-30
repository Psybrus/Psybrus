#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Os/OsClientAndroid.h"

#include "System/SysProfilerChromeTracing.h"

#include <android_native_app_glue.h>

#include <errno.h>
#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <android/log.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct android_app* GState_ = nullptr;

static OsClientAndroid* GMainWindow = nullptr;

void PsyAndroidMain(struct android_app* State)
{
	static bool IsInitialised = false;
	BcAssertMsg( IsInitialised == false, "Need to implement second tick through android_main." );

    // Make sure glue isn't stripped.
    app_dummy();

    GState_ = State;

	// Set game thread to be this one. 
	BcSetGameThread();

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

	PSY_LOG( "Psybrus for Android." );
	// Initialise RNG.
#if !PSY_DEBUG
	BcRandom::Global = BcRandom( 1337 ); // TODO LINUX
#endif

	PSY_LOG( "Init reflection" );

	// Create reflection database
	ReManager::Init();

#if PSY_USE_PROFILER
	// new profiler.
	new SysProfilerChromeTracing();
#endif

	//extern void MainUnitTests();
	//MainUnitTests();

	// Create kernel.
	new SysKernel( GPsySetupParams.TickRate_ );

	// Register systems for creation.
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
			GMainWindow = new OsClientAndroid( GState_ );
			std::string Title = ( GPsySetupParams.Name_ + std::string( " (" ) + SysArgs_ + std::string( ")" ) );
			
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
			extern void PsyLaunchGame();
			PsyLaunchGame();
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
