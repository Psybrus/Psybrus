#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Os/SDL/OsClientSDL.h"

#include "System/Os/OsInputMindwaveLinux.h"

#include "System/SysProfilerChromeTracing.h"

BcHandle GInstance_ = NULL;

eEvtReturn OnPreOsUpdate_PumpMessages( EvtID, const EvtBaseEvent& )
{
	// TODO: SDL pump?
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

static OsClientSDL* GMainWindow = nullptr;

eEvtReturn OnPostOsOpen_CreateClient( EvtID, const EvtBaseEvent& )
{
	GMainWindow = new OsClientSDL();
	if( GMainWindow->create( GPsySetupParams.Name_.c_str(), GInstance_, GResolutionWidth, GResolutionHeight, BcFalse, GPsySetupParams.Flags_ & psySF_WINDOW ? BcTrue : BcFalse ) == BcFalse )
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
}

eEvtReturn OnPostOsClose_DestroyClient( EvtID, const EvtBaseEvent& )
{
	GMainWindow->destroy();
	delete GMainWindow;
	GMainWindow = nullptr;
	return evtRET_REMOVE;
}



int main(int argc, char** argv)
{
	// Start.
	std::string CommandLine;

	for( int Idx = 0; Idx < argc; ++Idx )
	{
		CommandLine += argv[ Idx ];
		CommandLine += " ";
	}

	GInstance_ = (BcHandle)0;

	// Set command line params.
	SysArgs_ = CommandLine;

	// HACK: Append a space to sys args for find to work.
	SysArgs_ += " ";
	
	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLogImpl();
	}
#endif

	// Setup basic log Category.
	BcLogScopedCategory LogCategory( "Main" );

	// Initialise RNG.
#if !PSY_DEBUG
	BcRandom::Global = BcRandom( 1337 ); // TODO LINUX
#endif

	new OsInputMindwaveLinux();

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
	SYS_REGISTER( "DsCore", DsCoreImpl );
	SYS_REGISTER( "DsCoreLogging", DsCoreLoggingImpl );
	SYS_REGISTER( "OsCore", OsCoreImplSDL );
	SYS_REGISTER( "FsCore", FsCoreImplLinux );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImpl );
	SYS_REGISTER( "SsCore", SsCoreImplSoLoud );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOsOpen_CreateClient );
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_CLOSE, OnPostOsClose_DestroyClient );
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OnPreOsUpdate_PumpMessages );
	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOpenScnCore_LaunchGame );

	// Init game.
	PsyGameInit();

	//
	if( ( GPsySetupParams.Flags_ & psySF_MANUAL ) == 0 )
	{
		// Run kernel unthreaded.
		SysKernel::pImpl()->run( BcFalse );
	}

	// Delete kernel.
	delete SysKernel::pImpl();

	// Delete log.
	delete BcLog::pImpl();

	// Done.
	return 0;
}
