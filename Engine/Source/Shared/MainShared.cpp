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

	// Reflection serialisation JSON unit test.
	extern void BcReflectionSerialisationJson_UnitTest();
	BcReflectionSerialisationJson_UnitTest();
}

//////////////////////////////////////////////////////////////////////////
// onCsCoreOpened
eEvtReturn onCsCoreOpened( EvtID ID, const SysSystemEvent& Event )
{
	// Register scene resources.
	CsCore::pImpl()->registerResource< ScnAnimation >();
	CsCore::pImpl()->registerResource< ScnShader >();
	CsCore::pImpl()->registerResource< ScnTexture >();
	CsCore::pImpl()->registerResource< ScnTextureAtlas >();
	CsCore::pImpl()->registerResource< ScnRenderTarget >();
	CsCore::pImpl()->registerResource< ScnMaterial >();
	CsCore::pImpl()->registerResource< ScnFont >();
	CsCore::pImpl()->registerResource< ScnModel >();
	CsCore::pImpl()->registerResource< ScnSound >();

	// Register scene components.
	CsCore::pImpl()->registerResource< ScnComponent >();
	CsCore::pImpl()->registerResource< ScnRenderableComponent >();
	CsCore::pImpl()->registerResource< ScnSpatialComponent >();
	CsCore::pImpl()->registerResource< ScnEntity >();
	CsCore::pImpl()->registerResource< ScnDebugRenderComponent >();
	CsCore::pImpl()->registerResource< ScnMaterialComponent >();
	CsCore::pImpl()->registerResource< ScnFontComponent >();
	CsCore::pImpl()->registerResource< ScnParticleSystemComponent >();
	CsCore::pImpl()->registerResource< ScnAnimationComponent >();
	CsCore::pImpl()->registerResource< ScnLightComponent >();
	CsCore::pImpl()->registerResource< ScnModelComponent >();
	CsCore::pImpl()->registerResource< ScnSoundListenerComponent >();
	CsCore::pImpl()->registerResource< ScnSoundEmitterComponent >();
	CsCore::pImpl()->registerResource< ScnCanvasComponent >();

	// Register game resources before the view.
	PsyGameRegisterResources();

	// View is the last thing we want to update.
	CsCore::pImpl()->registerResource< ScnViewComponent >();

	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// onCsCorePreClose
eEvtReturn onCsCorePreClose( EvtID ID, const SysSystemEvent& Event )
{
	// Unregister scene resources.
	CsCore::pImpl()->unregisterResource< ScnComponent >();
	CsCore::pImpl()->unregisterResource< ScnRenderableComponent >();
	CsCore::pImpl()->unregisterResource< ScnSpatialComponent >();
	CsCore::pImpl()->unregisterResource< ScnEntity >();
	CsCore::pImpl()->unregisterResource< ScnDebugRenderComponent >();

	CsCore::pImpl()->unregisterResource< ScnShader >();
	CsCore::pImpl()->unregisterResource< ScnTexture >();
	CsCore::pImpl()->unregisterResource< ScnTextureAtlas >();
	CsCore::pImpl()->unregisterResource< ScnRenderTarget >();

	CsCore::pImpl()->unregisterResource< ScnMaterial >();
	CsCore::pImpl()->unregisterResource< ScnMaterialComponent >();

	CsCore::pImpl()->unregisterResource< ScnFont >();
	CsCore::pImpl()->unregisterResource< ScnFontComponent >();

	CsCore::pImpl()->unregisterResource< ScnAnimation >();
	CsCore::pImpl()->unregisterResource< ScnAnimationComponent >();

	CsCore::pImpl()->unregisterResource< ScnLightComponent >();

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
// OnQuit
eEvtReturn onQuit( EvtID ID, const OsEventCore& Event )
{
	SysKernel::pImpl()->stop();

	exit(0);
	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// MainShared
void MainShared()
{
	// Setup system threads.
	RsCore::WORKER_MASK = 0x1;
	FsCore::WORKER_MASK = 0x2;
	SsCore::WORKER_MASK = 0x0; // TODO DONT ENABLE.

	BcRegex Regex( "/(.*?)/" );
	BcRegexMatch Match;
	Regex.match( "/Content/Type/Package/Name/", Match );

	// Allocate a huge chunk of memory up front to prewarm.
	BcU32 MemSize = 1024 * 1024 * 128;
	BcU8* Prewarm = (BcU8*)BcMemAlign( MemSize, 4096 );
	for( BcU32 Idx = 0; Idx < MemSize / 4096; ++Idx )
	{
		Prewarm[ Idx * 4096 ]++;
	}
	BcMemFree( Prewarm );
	
	std::string String;
	for( BcU32 Idx = 0; Idx < Match.noofMatches(); ++Idx )
	{
		Match.getMatch( Idx, String );

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

	// Subscribe to quit.
	OsEventCore::Delegate OnQuitDelegate = OsEventCore::Delegate::bind< onQuit >();
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT, OnQuitDelegate );

}
