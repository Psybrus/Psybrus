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

//////////////////////////////////////////////////////////////////////////
// MainUnitTests
void MainUnitTests()
{
	BcPrintf( "============================================================================\n" );
	BcPrintf( "MainUnitTests:\n" );

	// Types unit test.
	extern void BcTypes_UnitTest();
	BcTypes_UnitTest();
	
	// Atomic unit test.
	extern void BcAtomic_UnitTest();
	BcAtomic_UnitTest();

	// Fixed unit test.
	extern void BcFixed_UnitTest();
	BcFixed_UnitTest();
}

//////////////////////////////////////////////////////////////////////////
// onCsCoreOpened
eEvtReturn onCsCoreOpened( EvtID ID, const SysSystemEvent& Event )
{
	// Register scene resources.
	CsCore::pImpl()->registerResource< ScnComponent >();
	CsCore::pImpl()->registerResource< ScnRenderableComponent >();
	CsCore::pImpl()->registerResource< ScnEntity >();

	CsCore::pImpl()->registerResource< ScnShader >();
	CsCore::pImpl()->registerResource< ScnTexture >();
	CsCore::pImpl()->registerResource< ScnTextureAtlas >();

	CsCore::pImpl()->registerResource< ScnRenderTarget >();

	CsCore::pImpl()->registerResource< ScnMaterial >();
	CsCore::pImpl()->registerResource< ScnMaterialComponent >();

	CsCore::pImpl()->registerResource< ScnFont >();
	CsCore::pImpl()->registerResource< ScnFontComponent >();

	CsCore::pImpl()->registerResource< ScnModel >();
	CsCore::pImpl()->registerResource< ScnModelComponent >();

	CsCore::pImpl()->registerResource< ScnParticleSystemComponent >();

	CsCore::pImpl()->registerResource< ScnSound >();
	CsCore::pImpl()->registerResource< ScnSoundListenerComponent >();
	CsCore::pImpl()->registerResource< ScnSoundEmitterComponent >();
		
	CsCore::pImpl()->registerResource< ScnCanvasComponent >();
	CsCore::pImpl()->registerResource< ScnViewComponent >();

	PsyGameRegisterResources();

	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// onCsCorePreClose
eEvtReturn onCsCorePreClose( EvtID ID, const SysSystemEvent& Event )
{
	// Unregister scene resources.
	CsCore::pImpl()->unregisterResource< ScnComponent >();
	CsCore::pImpl()->unregisterResource< ScnRenderableComponent >();
	CsCore::pImpl()->unregisterResource< ScnEntity >();

	CsCore::pImpl()->unregisterResource< ScnShader >();
	CsCore::pImpl()->unregisterResource< ScnTexture >();
	CsCore::pImpl()->unregisterResource< ScnTextureAtlas >();
	CsCore::pImpl()->unregisterResource< ScnRenderTarget >();

	CsCore::pImpl()->unregisterResource< ScnMaterial >();
	CsCore::pImpl()->unregisterResource< ScnMaterialComponent >();

	CsCore::pImpl()->unregisterResource< ScnFont >();
	CsCore::pImpl()->unregisterResource< ScnFontComponent >();

	CsCore::pImpl()->unregisterResource< ScnModel >();
	CsCore::pImpl()->unregisterResource< ScnModelComponent >();

	CsCore::pImpl()->unregisterResource< ScnParticleSystemComponent >();

	CsCore::pImpl()->unregisterResource< ScnSound >();
	CsCore::pImpl()->unregisterResource< ScnSoundEmitterComponent >();
	CsCore::pImpl()->unregisterResource< ScnSoundListenerComponent >();

	CsCore::pImpl()->unregisterResource< ScnCanvasComponent >();
	CsCore::pImpl()->unregisterResource< ScnViewComponent >();

	PsyGameUnRegisterResources();

	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// MainShared
void MainShared()
{
	// Setup system threads.
	FsCore::WORKER_MASK = 0x1;
	RsCore::WORKER_MASK = 0x2;
	SsCore::WORKER_MASK = 0x0; // TODO DONT ENABLE.

	// Test resource naming.
	BcRegex Regex( "\\$\\((.*?):(.*?)\\.(.*?)\\)" );
	BcRegexMatch Match( 6 );
	Regex.match( "$(CsResource:Package.Name)", Match );

	BcName Names[] =
	{
		"Test",
		"Test1",
		"Test_1",
		"Test_2",
		"Test_2_3",
		"Test_2_c",
		"Test_2_c_2",
	};
	
	for( BcU32 Idx = 0; Idx < Match.noofMatches(); ++Idx )
	{
		std::string match;
		Match.getMatch( Idx, match );
	}

	// Disable render thread for debugging.
	if( SysArgs_.find( "-norenderthread " ) != std::string::npos )
	{
		RsCore::WORKER_MASK = 0x0;
	}

	// Disable sound thread for debugging.
	if( SysArgs_.find( "-nosoundthread ") != std::string::npos )
	{
		SsCore::WORKER_MASK = 0x0;
	}

	// Disable file thread for debugging.
	if( SysArgs_.find( "-nofilethread " ) != std::string::npos )
	{
		FsCore::WORKER_MASK = 0x0;
	}

	// System worker mask.
	SysKernel::SYSTEM_WORKER_MASK = FsCore::WORKER_MASK | RsCore::WORKER_MASK | SsCore::WORKER_MASK;

	// Strip the renderer from the user mask.
	// - Renderer is a bit of a special case. NEVER allow anything to run on that thread since it should always be high CPU.
	SysKernel::USER_WORKER_MASK = SysKernel::USER_WORKER_MASK & ~RsCore::WORKER_MASK;

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
	BcPrintf( " - SysKernel::SYSTEM_WORKER_MASK: 0x%x\n", SysKernel::SYSTEM_WORKER_MASK );
	BcPrintf( " - SysKernel::USER_WORKER_MASK: 0x%x\n", SysKernel::USER_WORKER_MASK );
	BcPrintf( " - FsCore::WORKER_MASK: 0x%x\n", FsCore::WORKER_MASK );
	BcPrintf( " - RsCore::WORKER_MASK: 0x%x\n", RsCore::WORKER_MASK );
	BcPrintf( " - SsCore::WORKER_MASK: 0x%x\n", SsCore::WORKER_MASK );

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

	// Setup callback for post CsCore pre close for resource unregistration
	SysSystemEvent::Delegate OnCsCorePreClose = SysSystemEvent::Delegate::bind< onCsCorePreClose >();
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_PRE_CLOSE, OnCsCorePreClose );
}
