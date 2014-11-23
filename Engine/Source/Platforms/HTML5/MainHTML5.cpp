#include "MainShared.h"

#include "Base/BcLogImpl.h"
#include "Base/BcLogFile.h"
#include "Base/BcRandom.h"

#include "System/SysKernel.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"
#include "System/Os/OsClientHTML5.h"

#include <emscripten.h>

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
	OsClientHTML5* pMainWindow = new OsClientHTML5();
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
}

int main(int argc, char** argv)
{
	// If we have no log, setup a default one.
#if !PSY_PRODUCTION
	if( BcLog::pImpl() == NULL )
	{
		new BcLogImpl();
	}
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
//	SYS_REGISTER( "DsCore", DsCoreImpl );
	SYS_REGISTER( "OsCore", OsCoreImplHTML5 );
//	SYS_REGISTER( "FsCore", FsCoreImplLinux );
	SYS_REGISTER( "CsCore", CsCore );
	SYS_REGISTER( "RsCore", RsCoreImpl );
	SYS_REGISTER( "SsCore", SsCoreImplSoLoud );
	SYS_REGISTER( "ScnCore", ScnCore );

	// Main shared.
	MainShared();

	// Hook up create client delegate
	SysSystemEvent::Delegate OsPostOpenDelegateCreateClient = SysSystemEvent::Delegate::bind< OnPostOsOpen_CreateClient >();
	OsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OsPostOpenDelegateCreateClient );

	SysSystemEvent::Delegate OnPostOpenDelegateLaunchGame = SysSystemEvent::Delegate::bind< OnPostOpenScnCore_LaunchGame >();
	ScnCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnPostOpenDelegateLaunchGame );

	// Init game.
	//PsyGameInit();

	// Run kernel once.
	SysKernel::pImpl()->runOnce();

	return 0;
}
