#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

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

void PsyAndroidMain(struct android_app* State)
{
    // Make sure glue isn't stripped.
    app_dummy();

    GState_ = State;

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
