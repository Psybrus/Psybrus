#include "MainShared.h"
#include "SysKernel.h"
#include "OsCore.h"

#include "OsClientWindows.h"

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
	if( GPsySetupParams.Flags_ & psySF_WINDOW )
	{
		// Create window.
		//for( BcU32 Idx = 0; Idx < 2; ++Idx )
		{
			OsClientWindows* pMainWindow = new OsClientWindows();
			if( pMainWindow->create( GPsySetupParams.Name_.c_str(), GInstance_, GResolutionWidth, GResolutionHeight, BcFalse ) == BcFalse )
			{
				BcAssertMsg( BcFalse, "Failed to create client!" );
				return evtRET_REMOVE;
			}
		
			// Get rendering context.
			RsContext* pContext = RsCore::pImpl()->getContext( pMainWindow );
			BcAssertMsg( pContext != NULL, "Failed to create render context!" );
		}
	}

	return evtRET_REMOVE;	
}

int PASCAL WinMain ( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow )
{
	( void )hPrevInstance;
	( void )lpCmdLine;
	( void )nCmdShow;

	GInstance_ = (BcHandle)hInstance;

	// Perform unit tests.
	MainUnitTests();

	// Set command line params.
	SysArgs_ = lpCmdLine;
	
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

	if( ( GPsySetupParams.Flags_ & psySF_MANUAL ) == 0 )
	{
		// Run kernel unthreaded.
		SysKernel::pImpl()->run( BcFalse );
	}

	// Delete kernel.
	delete SysKernel::pImpl();

	// Done.
	return 0;
}
