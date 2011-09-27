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

#include "SysKernel.h"
#include "CsCore.h"

#include "ScnTexture.h"
#include "ScnTextureAtlas.h"
#include "ScnRenderTarget.h"
#include "ScnMaterial.h"
#include "ScnModel.h"
#include "ScnCanvas.h"
#include "ScnShader.h"
#include "ScnFont.h"
#include "ScnSound.h"
#include "ScnSoundEmitter.h"
#include "ScnSynthesizer.h"

#include "GaPackage.h"
#include "GaScript.h"

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
	// Start systems.
	SysKernel::pImpl()->startSystem( "OsCore" );
	SysKernel::pImpl()->startSystem( "RsCore" );
	SysKernel::pImpl()->startSystem( "SsCore" );
	SysKernel::pImpl()->startSystem( "FsCore" );
	SysKernel::pImpl()->startSystem( "CsCore" );
	SysKernel::pImpl()->startSystem( "GaCore" );
	
	// Register scene resources.
	CsCore::pImpl()->registerResource< ScnTexture >();
	CsCore::pImpl()->registerResource< ScnTextureAtlas >();
	CsCore::pImpl()->registerResource< ScnRenderTarget >();
	CsCore::pImpl()->registerResource< ScnMaterial >();
	CsCore::pImpl()->registerResource< ScnMaterialInstance >();
	CsCore::pImpl()->registerResource< ScnModel >();
	CsCore::pImpl()->registerResource< ScnModelInstance >();
	CsCore::pImpl()->registerResource< ScnCanvas >();
	CsCore::pImpl()->registerResource< ScnShader >();
	CsCore::pImpl()->registerResource< ScnFont >();
	CsCore::pImpl()->registerResource< ScnFontInstance >();
	CsCore::pImpl()->registerResource< ScnSound >();
	CsCore::pImpl()->registerResource< ScnSoundEmitter >();
	CsCore::pImpl()->registerResource< ScnSynthesizer >();

	// Register game resources.
	CsCore::pImpl()->registerResource< GaPackage >();
	CsCore::pImpl()->registerResource< GaScript >();
}

