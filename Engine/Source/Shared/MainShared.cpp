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
#include "System/Scene/Animation/ScnAnimationPose.h"

#include "System/Scene/Physics/ScnPhysicsCollisionShape.h"
#include "System/Scene/Physics/ScnPhysicsBoxCollisionShape.h"
#include "System/Scene/Physics/ScnPhysicsRigidBodyComponent.h"
#include "System/Scene/Physics/ScnPhysicsWorldComponent.h"

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

	// Register system.
	SysKernel::StaticRegisterClass();
	SysSystem::StaticRegisterClass();

	// Register content.
	CsResource::StaticRegisterClass();
	CsPackage::StaticRegisterClass();
	CsDependency::StaticRegisterClass();
	CsPackageDependencies::StaticRegisterClass();

	// Register file.
	FsTimestamp::StaticRegisterClass();
	FsStats::StaticRegisterClass();

	// Register debug.
	// Register network.
	// Register os
	// Register renderer
	RsColour::StaticRegisterClass();

	// Register sound

	// Register scene resources.
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
	ScnLightComponent::StaticRegisterClass();
	ScnModelComponent::StaticRegisterClass();
	ScnSoundListenerComponent::StaticRegisterClass();
	ScnSoundEmitterComponent::StaticRegisterClass();
	ScnCanvasComponent::StaticRegisterClass();

	ScnAnimation::StaticRegisterClass();
	ScnAnimationComponent::StaticRegisterClass();
	ScnAnimationTreeNode::StaticRegisterClass();
	ScnAnimationTreeBlendNode::StaticRegisterClass();
	ScnAnimationTreeTrackNode::StaticRegisterClass();
	ScnAnimationPose::StaticRegisterClass();

	ScnPhysicsCollisionShape::StaticRegisterClass();
	ScnPhysicsBoxCollisionShape::StaticRegisterClass();
	ScnPhysicsRigidBodyComponent::StaticRegisterClass();
	ScnPhysicsWorldComponent::StaticRegisterClass();

	// Scene shader stuff.
	ScnShaderViewUniformBlockData::StaticRegisterClass();
	ScnShaderLightUniformBlockData::StaticRegisterClass();
	ScnShaderObjectUniformBlockData::StaticRegisterClass();
	ScnShaderBoneUniformBlockData::StaticRegisterClass();
	ScnShaderAlphaTestUniformBlockData::StaticRegisterClass();
	
	// Register game resources before the view.
	PsyGameRegisterResources();

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
	FsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 1 );
	RsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 2 );
	SsCore::JOB_QUEUE_ID = BcErrorCode;

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
