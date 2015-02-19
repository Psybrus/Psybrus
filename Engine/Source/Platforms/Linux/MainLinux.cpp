#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcLogFile.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Os/SDL/OsClientSDL.h"

#include "System/Os/OsInputMindwaveLinux.h"

#include "System/SysProfilerChromeTracing.h"

BcHandle GInstance_ = NULL;

eEvtReturn OnPreOsUpdate_PumpMessages( EvtID, const SysSystemEvent& )
{
	// TODO: SDL pump?
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

static OsClientSDL* GMainWindow = nullptr;

eEvtReturn OnPostOsOpen_CreateClient( EvtID, const SysSystemEvent& )
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

eEvtReturn OnPostOsClose_DestroyClient( EvtID, const SysSystemEvent& )
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
	SYS_REGISTER( "OsCore", OsCoreImplSDL );
	SYS_REGISTER( "FsCore", FsCoreImplLinux );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImpl );
	SYS_REGISTER( "SsCore", SsCoreImplSoLoud );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

	// Hook up create client delegate
	SysSystemEvent::Delegate OsPostOpenDelegateCreateClient = SysSystemEvent::Delegate::bind< OnPostOsOpen_CreateClient >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OsPostOpenDelegateCreateClient );

	SysSystemEvent::Delegate OsPostCloseDelegateDestroyClient = SysSystemEvent::Delegate::bind< OnPostOsClose_DestroyClient >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_CLOSE, OsPostCloseDelegateDestroyClient );

	// Hook up event pump delegate.
	SysSystemEvent::Delegate OsPreUpdateDelegatePumpMessages = SysSystemEvent::Delegate::bind< OnPreOsUpdate_PumpMessages >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OsPreUpdateDelegatePumpMessages );

	SysSystemEvent::Delegate OnPostOpenDelegateLaunchGame = SysSystemEvent::Delegate::bind< OnPostOpenScnCore_LaunchGame >();
	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOpenDelegateLaunchGame );

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
