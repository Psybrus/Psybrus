#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Os/SDL/OsClientSDL.h"

#include "System/Renderer/RsCore.h"

#include <iostream>

namespace
{
	BcHandle GInstance_ = nullptr;
}

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
	std::string Title = ( GPsySetupParams.Name_ + std::string( " (" ) + GCommandLine_.c_str() + std::string( ")" ) );
	
	if( GMainWindow->create( Title.c_str(), GInstance_, GResolutionWidth, GResolutionHeight, BcFalse, GPsySetupParams.Flags_ & psySF_WINDOW ? BcTrue : BcFalse ) == BcFalse )
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

int main(int argc, const char* argv[])
{
	GCommandLine_ = BcCommandLine( argc, argv );
	
	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLogImpl();
	}

	// Setup basic log Category.
	BcLogScopedCategory LogCategory( "Main" );
#endif

	// Some default suppression.
	if( BcLog::pImpl() != nullptr )
	{
		BcLog::pImpl()->setCategorySuppression( "Reflection", BcTrue );

		// Render & sound are noisy.
		BcLog::pImpl()->setCategorySuppression( "RsCore", BcTrue );
		BcLog::pImpl()->setCategorySuppression( "SsCore", BcTrue );
		
	}

	// Initialise RNG.
#if !PSY_DEBUG
	BcRandom::Global = BcRandom( 1337 ); // TODO LINUX
#endif

	// Create reflection database
	ReManager::Init();

	// Unit tests prior to full kernel initialisation.
	if( GCommandLine_.hasArg( '\0', "unittest" ) )
	{
		return MainUnitTests();
	}

	// Create kernel.
	new SysKernel( GPsySetupParams.TickRate_ );

	// Register systems for creation.
	SYS_REGISTER( "OsCore", OsCoreImplSDL );
	SYS_REGISTER( "FsCore", FsCoreImplOSX );
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
		OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_CLOSE, OnPostOsClose_DestroyClient );
		OsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_UPDATE, OnPreOsUpdate_PumpMessages );
		ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOpenScnCore_LaunchGame );

		// Init game.
		PsyGameInit();
	}
	else if( GPsySetupParams.Flags_ & psySF_TOOL )
	{
		SysSystem* PostOpenSystem = ScnCore::pImpl();
		if( !PostOpenSystem )
		{
			PostOpenSystem = CsCore::pImpl();
		}

		PostOpenSystem->subscribe( sysEVT_SYSTEM_POST_OPEN, 
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

	// Done.
	return 0;
}
