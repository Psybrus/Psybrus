/**************************************************************************
*
* File:		MainShared.cpp
* Author:	Neil Richardson 
* Ver/Date:	25/09/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "MainShared.h"

BcU32 GResolutionWidth = 1280;
BcU32 GResolutionHeight = 720;

#include "Events/EvtBinding.h"
#include "Events/EvtEvent.h"
#include "Events/EvtPublisher.h"
#include "Events/EvtProxyBuffered.h"

#include "Math/MaMat3d.h"
#include "Math/MaMat4d.h"

#include "System/SysKernel.h"

#include "System/Content/CsCore.h"
#include "System/File/FsCore.h"
#include "System/Os/OsCore.h"
#include "System/Renderer/RsCore.h"
#include "System/Sound/SsCore.h"
#include "System/Scene/ScnCore.h"


//////////////////////////////////////////////////////////////////////////
// MainUnitTests
void MainUnitTests()
{
	BcPrintf( "============================================================================\n" );
	BcPrintf( "MainUnitTests:\n" );

	// Types unit test.
	extern void BcTypes_UnitTest();
	BcTypes_UnitTest();
	
	// Fixed unit test.
	extern void BcFixed_UnitTest();
	BcFixed_UnitTest();
}

//////////////////////////////////////////////////////////////////////////
// onCsCoreOpened
eEvtReturn onCsCoreOpened( EvtID ID, const SysSystemEvent& Event )
{
	// Register reflection.
	extern void AutoGenRegisterReflection();
	AutoGenRegisterReflection();

	return evtRET_REMOVE;
}


//////////////////////////////////////////////////////////////////////////
// OnQuit
eEvtReturn onQuit( EvtID ID, const OsEventCore& Event )
{
	SysKernel::pImpl()->stop();

	return evtRET_REMOVE;
}


//////////////////////////////////////////////////////////////////////////
// OnScreenshot
eEvtReturn onScreenshot( EvtID ID, const OsEventInputKeyboard& Event )
{
	if( Event.KeyCode_ == OsEventInputKeyboard::KEYCODE_F1 )
	{
		RsCore::pImpl()->getContext( nullptr )->takeScreenshot();
	}	

	return evtRET_PASS;
}

//////////////////////////////////////////////////////////////////////////
// MainShared
void MainShared()
{
	// Setup default system job queues.
	SysKernel::DEFAULT_JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( std::thread::hardware_concurrency(), 0 );

	// Disable render thread for debugging.
	if( SysArgs_.find( "-norenderthread " ) != std::string::npos )
	{
		RsCore::JOB_QUEUE_ID = BcErrorCode;
	}

	// Disable sound thread for debugging.
	if( SysArgs_.find( "-nosoundthread ") != std::string::npos )
	{
		SsCore::JOB_QUEUE_ID = BcErrorCode;
	}

	// Disable file thread for debugging.
	if( SysArgs_.find( "-nofilethread " ) != std::string::npos )
	{
		FsCore::JOB_QUEUE_ID = BcErrorCode;
	}

	// Create default job queue.
	SysKernel::DEFAULT_JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 0, 0 );

	// Parse command line params for disabling systems.
	if( SysArgs_.find( "-noremote " ) != std::string::npos )
	{
		GPsySetupParams.Flags_ &= ~psySF_REMOTE;
	}

	if( SysArgs_.find( "-norender " ) != std::string::npos )
	{
		GPsySetupParams.Flags_ &= ~psySF_RENDER;
	}

	if( SysArgs_.find( "-nosound " ) != std::string::npos )
	{
		GPsySetupParams.Flags_ &= ~psySF_SOUND;
	}
	
	// HACK: If we are importing packages, disable renderer and sound systems.
	if( SysArgs_.find( "ImportPackages" ) != std::string::npos )
	{
		GPsySetupParams.Flags_ &= ~( psySF_RENDER | psySF_SOUND );
	}

	// Log kernel information.
	BcPrintf( "============================================================================\n" );
	BcPrintf( "MainShared:\n" );
	BcPrintf( " - Command line: %s\n", SysArgs_.c_str() );
	BcPrintf( " - Setup Flags: 0x%x\n", GPsySetupParams.Flags_ );
	BcPrintf( " - Name: %s\n", GPsySetupParams.Name_.c_str() );
	BcPrintf( " - Tick Rate: 1.0/%.1f\n", 1.0f / GPsySetupParams.TickRate_ );
	BcPrintf( " - SysKernel::DEFAULT_JOB_QUEUE_ID: 0x%x\n", SysKernel::DEFAULT_JOB_QUEUE_ID );
	BcPrintf( " - FsCore::JOB_QUEUE_ID: 0x%x\n", FsCore::JOB_QUEUE_ID );
	BcPrintf( " - RsCore::JOB_QUEUE_ID: 0x%x\n", RsCore::JOB_QUEUE_ID );
	BcPrintf( " - SsCore::JOB_QUEUE_ID: 0x%x\n", SsCore::JOB_QUEUE_ID );

	// Start debug system if not a production build.
#if !defined( PSY_PRODUCTION )
	SysKernel::pImpl()->startSystem( "DsCore" );
#endif

	// Start file system.
	SysKernel::pImpl()->startSystem( "FsCore" );

	// Start OS system.
	SysKernel::pImpl()->startSystem( "OsCore" );

	// Start render system.
	if( GPsySetupParams.Flags_ & psySF_RENDER )
	{
		SysKernel::pImpl()->startSystem( "RsCore" );
	}

	// Start sound system.
	if( GPsySetupParams.Flags_ & psySF_SOUND )
	{
		SysKernel::pImpl()->startSystem( "SsCore" );
	}

	// Start content system, depending on startup flags.
	SysKernel::pImpl()->startSystem( "CsCore" );

	// Start scene system.
	SysKernel::pImpl()->startSystem( "ScnCore" );

	// Setup callback for post CsCore open for resource registration.
	SysSystemEvent::Delegate OnCsCoreOpened = SysSystemEvent::Delegate::bind< onCsCoreOpened >();
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnCsCoreOpened );

	// Subscribe to quit.
	OsEventCore::Delegate OnQuitDelegate = OsEventCore::Delegate::bind< onQuit >();
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT, OnQuitDelegate );

	// Subscribe to F11 for screenshot
	OsEventInputKeyboard::Delegate OnScreenshotDelegate = OsEventInputKeyboard::Delegate::bind< onScreenshot >();
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, OnScreenshotDelegate );
}
