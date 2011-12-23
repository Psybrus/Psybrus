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

#ifdef PSY_DEBUG
BcU32 GResolutionWidth = 1280 / 2;
BcU32 GResolutionHeight = 720 / 2;
#else
BcU32 GResolutionWidth = 1280;
BcU32 GResolutionHeight = 720;
#endif

//////////////////////////////////////////////////////////////////////////
// MainUnitTests
void MainUnitTests()
{
	// Types unit test.
	extern void BcTypes_UnitTest();
	BcTypes_UnitTest();
	
	// Atomic unit test.
	extern void BcAtomic_UnitTest();
	BcAtomic_UnitTest();
}

//////////////////////////////////////////////////////////////////////////
// onCsCoreOpened
eEvtReturn onCsCoreOpened( EvtID ID, const SysSystemEvent& Event )
{
#ifdef PSY_SERVER
	// Add overlay paths.
	CsCore::pImpl()->addImportOverlayPath( "EngineContent" );
	CsCore::pImpl()->addImportOverlayPath( "GameContent" );
#endif

	// Register scene resources.
	CsCore::pImpl()->registerResource< ScnRenderTarget >();

	CsCore::pImpl()->registerResource< ScnShader >();
	CsCore::pImpl()->registerResource< ScnTexture >();
	CsCore::pImpl()->registerResource< ScnTextureAtlas >();

	CsCore::pImpl()->registerResource< ScnMaterial >();
	CsCore::pImpl()->registerResource< ScnMaterialInstance >();

	CsCore::pImpl()->registerResource< ScnFont >();
	CsCore::pImpl()->registerResource< ScnFontInstance >();

	CsCore::pImpl()->registerResource< ScnModel >();
	CsCore::pImpl()->registerResource< ScnModelInstance >();

	CsCore::pImpl()->registerResource< ScnSound >();
	CsCore::pImpl()->registerResource< ScnSoundEmitter >();

	CsCore::pImpl()->registerResource< ScnComponent >();
	CsCore::pImpl()->registerResource< ScnEntity >();

	CsCore::pImpl()->registerResource< ScnCanvas >();
	CsCore::pImpl()->registerResource< ScnView >();

	return evtRET_REMOVE;
}

//////////////////////////////////////////////////////////////////////////
// MainShared
void MainShared()
{
	// Setup system threads.
	FsCore::WORKER_MASK = 0x1;
	RsCore::WORKER_MASK = 0x2;
	SsCore::WORKER_MASK = 0x4;

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
	// - Renderer is a bit of a special case. NEVER allow anything to run on that thread since it will always be high CPU.
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

	// Start file and content systems.
	SysKernel::pImpl()->startSystem( "FsCore" );
	SysKernel::pImpl()->startSystem( "CsCore" );

	// Start systems.
	if( GPsySetupParams.Flags_ & psySF_REMOTE )
	{
		SysKernel::pImpl()->startSystem( "RmCore" );
	}

	SysKernel::pImpl()->startSystem( "OsCore" );

	if( GPsySetupParams.Flags_ & psySF_RENDER )
	{
		SysKernel::pImpl()->startSystem( "RsCore" );
	}

	if( GPsySetupParams.Flags_ & psySF_SOUND )
	{
		SysKernel::pImpl()->startSystem( "SsCore" );
	}

	// Setup callback for post CsCore open for resource registration.
	SysSystemEvent::Delegate OnCsCoreOpened = SysSystemEvent::Delegate::bind< onCsCoreOpened >();
	CsCore::pImpl()->subscribe( sysEVT_SYSTEM_POST_OPEN, OnCsCoreOpened );
}
