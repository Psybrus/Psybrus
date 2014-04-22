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

#include "System/Scene/Animation/ScnAnimation.h"
#include "System/Scene/Animation/ScnAnimationComponent.h"
#include "System/Scene/Animation/ScnAnimationTreeNode.h"
#include "System/Scene/Animation/ScnAnimationTreeBlendNode.h"
#include "System/Scene/Animation/ScnAnimationTreeTrackNode.h"

#include "System/Scene/ScnComponent.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/ScnDebugRenderComponent.h"
#include "System/Scene/ScnTexture.h"
#include "System/Scene/ScnTextureAtlas.h"
#include "System/Scene/ScnRenderTarget.h"
#include "System/Scene/ScnLightComponent.h"
#include "System/Scene/ScnMaterial.h"
#include "System/Scene/ScnModel.h"
#include "System/Scene/ScnParticleSystemComponent.h"
#include "System/Scene/ScnCanvasComponent.h"
#include "System/Scene/ScnShader.h"
#include "System/Scene/ScnFont.h"
#include "System/Scene/ScnSound.h"
#include "System/Scene/ScnSoundEmitter.h"
#include "System/Scene/ScnSoundListenerComponent.h"
#include "System/Scene/ScnViewComponent.h"

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
	// Register math libraries.
	MaVec2d::StaticRegisterClass();
	MaVec3d::StaticRegisterClass();
	MaVec4d::StaticRegisterClass();
	MaAABB::StaticRegisterClass();
	MaMat3d::StaticRegisterClass();
	MaMat4d::StaticRegisterClass();
	MaPlane::StaticRegisterClass();

	// Register content.
	CsResource::StaticRegisterClass();
	CsPackage::StaticRegisterClass();

	// Register debug.
	// Register file.
	// Register network.
	// Register os
	// Register renderer
	RsColour::StaticRegisterClass();

	// Register sound

	// Register scene resources.
	ScnAnimation::StaticRegisterClass();
	ScnShader::StaticRegisterClass();
	ScnTexture::StaticRegisterClass();
	ScnTextureAtlas::StaticRegisterClass();
	ScnRenderTarget::StaticRegisterClass();
	ScnMaterial::StaticRegisterClass();
	ScnFont::StaticRegisterClass();
	ScnModel::StaticRegisterClass();
	ScnSound::StaticRegisterClass();

	// Register scene components.
	ScnViewComponent::StaticRegisterClass();
	ScnComponent::StaticRegisterClass();
	ScnRenderableComponent::StaticRegisterClass();
	ScnSpatialComponent::StaticRegisterClass();
	ScnEntity::StaticRegisterClass();
	ScnDebugRenderComponent::StaticRegisterClass();
	ScnMaterialComponent::StaticRegisterClass();
	ScnFontComponent::StaticRegisterClass();
	ScnParticleSystemComponent::StaticRegisterClass();
	ScnAnimationComponent::StaticRegisterClass();
	ScnLightComponent::StaticRegisterClass();
	ScnModelComponent::StaticRegisterClass();
	ScnSoundListenerComponent::StaticRegisterClass();
	ScnSoundEmitterComponent::StaticRegisterClass();
	ScnCanvasComponent::StaticRegisterClass();

	// Register game resources before the view.
	PsyGameRegisterResources();

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

	// Subscribe to quit.
	OsEventCore::Delegate OnQuitDelegate = OsEventCore::Delegate::bind< onQuit >();
	OsCore::pImpl()->subscribe( osEVT_CORE_QUIT, OnQuitDelegate );

}
