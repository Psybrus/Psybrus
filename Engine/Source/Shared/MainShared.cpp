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
	PSY_LOG( "============================================================================\n" );
	PSY_LOG( "MainUnitTests:\n" );

	BcAssertScopedHandler AssertHandler(
		[]( const BcChar* Message, const BcChar* File, int Line )
		{
			BcPrintf( "Caught assertion: \"%s\" in %s on line %u.\n", Message, File, Line );
			return BcFalse;
		} );

	// Types unit test.
	extern void BcTypes_UnitTest();
	BcTypes_UnitTest();
	
#if !PLATFORM_ANDROID
	// Fixed unit test.
	extern void BcFixed_UnitTest();
	BcFixed_UnitTest();

	// Relative ptr unit test.
	extern void BcRelativePtr_UnitTest();
	BcRelativePtr_UnitTest();
#endif

	// SysKernel unit test.
	extern void SysKernel_UnitTest();
	SysKernel_UnitTest();
}

//////////////////////////////////////////////////////////////////////////
// onCsCoreOpened
eEvtReturn onCsCoreOpened( EvtID ID, const EvtBaseEvent& Event )
{
	// Register reflection.
	extern void AutoGenRegisterReflection();
	AutoGenRegisterReflection();

	return evtRET_REMOVE;
}


//////////////////////////////////////////////////////////////////////////
// OnQuit
eEvtReturn onQuit( EvtID ID, const EvtBaseEvent& Event )
{
	SysKernel::pImpl()->stop();

	return evtRET_REMOVE;
}


//////////////////////////////////////////////////////////////////////////
// OnScreenshot
eEvtReturn onScreenshot( EvtID ID, const EvtBaseEvent& Event )
{
	const auto& KeyEvent = Event.get< OsEventInputKeyboard >();
	if( KeyEvent.KeyCode_ == OsEventInputKeyboard::KEYCODE_F1 )
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

	/*
	// Disable render thread for debugging.
	if( SysArgs_.find( "-norenderthread " ) != std::string::npos )
	{
		RsCore::JOB_QUEUE_ID = -1;
	}
	
	// Disable sound thread for debugging.
	if( SysArgs_.find( "-nosoundthread ") != std::string::npos )
	{
		SsCore::JOB_QUEUE_ID = -1;
	}
	
	// Disable file thread for debugging.
	if( SysArgs_.find( "-nofilethread " ) != std::string::npos )
	{
		FsCore::JOB_QUEUE_ID = -1;
	}
	*/

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


	// Start debug system if not a production build.
#if !defined( PSY_PRODUCTION )
	SysKernel::pImpl()->startSystem( "DsCoreLogging" );
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

	// Log kernel information.
	PSY_LOG( "============================================================================\n" );
	PSY_LOG( "MainShared:\n" );
	PSY_LOG( " - Command line: %s\n", SysArgs_.c_str() );
	PSY_LOG( " - Setup Flags: 0x%x\n", GPsySetupParams.Flags_ );
	PSY_LOG( " - Name: %s\n", GPsySetupParams.Name_.c_str() );
	PSY_LOG( " - Tick Rate: 1.0/%.1f\n", 1.0f / GPsySetupParams.TickRate_ );
	PSY_LOG( " - SysKernel::DEFAULT_JOB_QUEUE_ID: 0x%x\n", SysKernel::DEFAULT_JOB_QUEUE_ID );
	PSY_LOG( " - FsCore::JOB_QUEUE_ID: 0x%x\n", FsCore::JOB_QUEUE_ID );
	PSY_LOG( " - RsCore::JOB_QUEUE_ID: 0x%x\n", RsCore::JOB_QUEUE_ID );
	PSY_LOG( " - SsCore::JOB_QUEUE_ID: 0x%x\n", SsCore::JOB_QUEUE_ID );

	// Setup callback for post CsCore open for resource registration.
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, onCsCoreOpened );

	// Subscribe to quit.
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT, onQuit );

	// Subscribe to F11 for screenshot
	OsCore::pImpl()->subscribe( osEVT_INPUT_KEYDOWN, onScreenshot );
}
