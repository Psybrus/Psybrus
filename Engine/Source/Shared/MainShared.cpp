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
BcU32 GResolutionWidth = 1024 / 2;
BcU32 GResolutionHeight = 768 / 2;
#else
BcU32 GResolutionWidth = 1024;
BcU32 GResolutionHeight = 768;
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
// MainShared
void MainShared()
{
	// Parse command line params for disabling systems.
	if( SysArgs_.find( "-noremote" ) != std::string::npos )
	{
		GPsySetupParams.Flags_ &= ~psySF_REMOTE;
	}

	if( SysArgs_.find( "-norender" ) != std::string::npos )
	{
		GPsySetupParams.Flags_ &= ~psySF_RENDER;
	}

	if( SysArgs_.find( "-nosound" ) != std::string::npos )
	{
		GPsySetupParams.Flags_ &= ~psySF_SOUND;
	}

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

	SysKernel::pImpl()->startSystem( "FsCore" );
	SysKernel::pImpl()->startSystem( "CsCore" );

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
}
