/**************************************************************************
*
* File:		GaTopState.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Game top state.
*		
*
*
* 
**************************************************************************/

#include "GaTopState.h"

#include "GaWorldComponent.h"
#include "GaPlayerComponent.h"
#include "GaMonsterComponent.h"
#include "GaTitleComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
GaTopState::GaTopState()
{
	name( "GaTopState" );
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
GaTopState::~GaTopState()
{

}

////////////////////////////////////////////////////////////////////////////////
// enterOnce
void GaTopState::enterOnce()
{
	Time_ = 0.0f;
	PrevTime_ = 0.0f;
	BestTime_ = 0.0f;

	CsCore::pImpl()->requestResource( "world", WorldModel_ );
	CsCore::pImpl()->requestResource( "monster", MonsterModel_ );
	CsCore::pImpl()->requestResource( "world", WorldRigidBody_ );
	CsCore::pImpl()->requestResource( "player", PlayerRigidBody_ );
	CsCore::pImpl()->requestResource( "waterdrop0", WaterDropSound_ );
	CsCore::pImpl()->requestResource( "footstepslow", FootStepSlowSound_ );
	CsCore::pImpl()->requestResource( "footstepfast", FootStepFastSound_ );
	CsCore::pImpl()->requestResource( "ambience0", Ambience0Sound_ );
	CsCore::pImpl()->requestResource( "ambience1", Ambience1Sound_ );
	CsCore::pImpl()->requestResource( "jump", JumpSound_ );
	CsCore::pImpl()->requestResource( "land", LandSound_ );
	CsCore::pImpl()->requestResource( "footstep", FootStepSound_ );
	CsCore::pImpl()->requestResource( "monsteralert0", MonsterAlertSound_ );
	CsCore::pImpl()->requestResource( "monsterroar0", MonsterRoarSound_[0] );
	CsCore::pImpl()->requestResource( "monsterroar1", MonsterRoarSound_[1] );
	CsCore::pImpl()->requestResource( "monsterroar2", MonsterRoarSound_[2] );
	CsCore::pImpl()->requestResource( "monsterroar3", MonsterRoarSound_[3] );
	CsCore::pImpl()->requestResource( "dead", DeadSound_ );

	CsCore::pImpl()->requestResource( "title", TitleMaterial_ );

	CsCore::pImpl()->requestResource( "noise0", NoiseTexture_[0] );
	CsCore::pImpl()->requestResource( "noise1", NoiseTexture_[1] );
	CsCore::pImpl()->requestResource( "noise2", NoiseTexture_[2] );
	CsCore::pImpl()->requestResource( "noise3", NoiseTexture_[3] );

	BcFile Scores;
	if( Scores.open( "highscore.dat" ) )
	{
		Scores.read( &PrevTime_, sizeof( PrevTime_ ) );
		Scores.read( &BestTime_, sizeof( BestTime_ ) );
		Scores.close();
	}
}

////////////////////////////////////////////////////////////////////////////////
// enter
eSysStateReturn GaTopState::enter()
{
	BcBool Ready = BcTrue;
	
	Ready &= WorldModel_.isReady();
	Ready &= MonsterModel_.isReady();
	Ready &= WorldRigidBody_.isReady();
	Ready &= PlayerRigidBody_.isReady();
	Ready &= WaterDropSound_.isReady();
	Ready &= FootStepSlowSound_.isReady();
	Ready &= FootStepFastSound_.isReady();
	Ready &= Ambience0Sound_.isReady();
	Ready &= Ambience1Sound_.isReady();
	Ready &= JumpSound_.isReady();
	Ready &= LandSound_.isReady();
	Ready &= FootStepSound_.isReady();
	Ready &= MonsterAlertSound_.isReady();
	Ready &= MonsterRoarSound_[0].isReady();
	Ready &= MonsterRoarSound_[1].isReady();
	Ready &= MonsterRoarSound_[2].isReady();
	Ready &= MonsterRoarSound_[3].isReady();
	Ready &= DeadSound_.isReady();
	Ready &= TitleMaterial_.isReady();

	Ready &= NoiseTexture_[0].isReady();
	Ready &= NoiseTexture_[1].isReady();
	Ready &= NoiseTexture_[2].isReady();
	Ready &= NoiseTexture_[3].isReady();

	Ready &= ScnMaterial::Default.isReady();
	Ready &= ScnModel::Default.isReady();
	Ready &= ScnRigidBody::Default.isReady();
	
	return Ready ? sysSR_FINISHED : sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preMain
void GaTopState::preMain()
{
	startTitle();
}

////////////////////////////////////////////////////////////////////////////////
// main
eSysStateReturn GaTopState::main()
{
	switch( GameStage_ )
	{
	case GS_IDLE:
		{
			Time_ += SysKernel::pImpl()->getFrameTime();
		}
		break;

	case GS_START_TITLE:
		{
			// Play death listener.
			SsCore::pImpl()->play( DeadSound_->getSample() );
			SsCore::pImpl()->setListener( BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, 0.0f, 1.0f ), BcVec3d( 0.0f, 1.0f, 0.0f ) );

			// Create entities.
			if( CsCore::pImpl()->createResource( "TitleEntity_0", TitleEntity_ ) )
			{
				ScnMaterialComponentRef MaterialComponent;
				ScnCanvasComponentRef CanvasComponent;
				GaTitleComponentRef TitleComponent;
				ScnViewComponentRef ViewComponent;

				CsCore::pImpl()->createResource( BcName::INVALID, MaterialComponent, TitleMaterial_, BcErrorCode );
				CsCore::pImpl()->createResource( BcName::INVALID, CanvasComponent, 8192, MaterialComponent );
				CsCore::pImpl()->createResource( BcName::INVALID, TitleComponent, TitleMaterial_, PrevTime_, BestTime_ );
				CsCore::pImpl()->createResource( BcName::INVALID, ViewComponent, 0.0f, 0.0f, 1.0f, 1.0f, 0.1f, 1024.0f, BcPIDIV4, 0.0f );

				TitleEntity_->attach( MaterialComponent );
				TitleEntity_->attach( CanvasComponent );
				TitleEntity_->attach( TitleComponent );
				TitleEntity_->attach( ViewComponent );
				ScnCore::pImpl()->addEntity( TitleEntity_ );
			}
			GameStage_ = GS_IDLE;
		}
		break;

	case GS_START_GAME:
		{
			// Create world entity.
			if( CsCore::pImpl()->createResource( "WorldEntity_0", WorldEntity_ ) )
			{
				BcMat4d RotationMatrix;
				RotationMatrix.rotation( BcVec3d( BcPIDIV2, 0.0f, 0.0f ) );
				BcQuat Rotation;
				Rotation.fromMatrix4d( RotationMatrix );

				WorldEntity_->setPosition( BcVec3d( 0.0f, 0.0f, 0.0f ) );
				WorldEntity_->setRotation( Rotation );

				ScnModelComponentRef ModelComponent;
				ScnRigidBodyWorldComponentRef RigidBodyWorldComponent;
				ScnRigidBodyComponentRef RigidBodyComponent;
				GaWorldComponentRef WorldComponent;
				CsCore::pImpl()->createResource( BcName::INVALID, ModelComponent, WorldModel_ );
				CsCore::pImpl()->createResource( BcName::INVALID, RigidBodyWorldComponent, ScnRigidBodyWorldRef( NULL ) );
				CsCore::pImpl()->createResource( BcName::INVALID, RigidBodyComponent, WorldRigidBody_ );
				CsCore::pImpl()->createResource( BcName::INVALID, WorldComponent );
				WorldEntity_->attach( ModelComponent );
				WorldEntity_->attach( RigidBodyWorldComponent );
				WorldEntity_->attach( RigidBodyComponent );
				WorldEntity_->attach( WorldComponent );
				ScnCore::pImpl()->addEntity( WorldEntity_ );
			}
			
			// Create player entity.
			if( CsCore::pImpl()->createResource( "PlayerEntity_0", PlayerEntity_ ) )
			{
				// 
				BcMat4d LookAt;
				LookAt.lookAt( BcVec3d( 0.0f, 0.0f, 0.0f ), BcVec3d( 0.0f, 2.0f, 1.0f ), BcVec3d( 0.0f, 1.0f, 0.0f ) );
				LookAt.inverse();
				PlayerEntity_->setMatrix( LookAt );
				
				//
				GaPlayerComponentRef PlayerComponent;
				CsCore::pImpl()->createResource( BcName::INVALID, PlayerComponent );
				PlayerEntity_->attach( PlayerComponent );

				//
				ScnViewComponentRef ViewComponent;
				CsCore::pImpl()->createResource( BcName::INVALID, ViewComponent, 0.0f, 0.0f, 1.0f, 1.0f, 0.1f, 1024.0f, BcPIDIV4, 0.0f );
				PlayerEntity_->attach( ViewComponent );

				ScnCore::pImpl()->addEntity( PlayerEntity_ );
 			}

			// Create monster entity.
			if( CsCore::pImpl()->createResource( "MonsterEntity_0", MonsterEntity_ ) )
			{
				// 
				BcMat4d LookAt;
				LookAt.lookAt( BcVec3d( 64.0f, 0.0f, 64.0f ), BcVec3d( 0.0f, 2.0f, 1.0f ), BcVec3d( 0.0f, 1.0f, 0.0f ) );
				LookAt.inverse();
				MonsterEntity_->setMatrix( LookAt );

				//
				GaMonsterComponentRef MonsterComponent;
				ScnModelComponentRef ModelComponent;
				CsCore::pImpl()->createResource( BcName::INVALID, MonsterComponent );
				//CsCore::pImpl()->createResource( BcName::INVALID, ModelComponent, MonsterModel_ );
				MonsterEntity_->attach( MonsterComponent );
				//MonsterEntity_->attach( ModelComponent );

				ScnCore::pImpl()->addEntity( MonsterEntity_ );
 			}
			GameStage_ = GS_IDLE;
			Time_ = 0.0f;
		}
		break;
	}
	
	return sysSR_CONTINUE;
}

////////////////////////////////////////////////////////////////////////////////
// preLeave
void GaTopState::preLeave()
{
	ScnCore::pImpl()->removeAllEntities();
}

////////////////////////////////////////////////////////////////////////////////
// leave
eSysStateReturn GaTopState::leave()
{
	return sysSR_FINISHED;
}

////////////////////////////////////////////////////////////////////////////////
// leaveOnce
void GaTopState::leaveOnce()
{
	
}

////////////////////////////////////////////////////////////////////////////////
// startTitle
void GaTopState::startTitle()
{
	PrevTime_ = Time_;
	BestTime_ = BcMax( BestTime_, PrevTime_ );

	BcFile Scores;
	if( Scores.open( "highscore.dat", bcFM_WRITE ) )
	{
		Scores.write( &PrevTime_, sizeof( PrevTime_ ) );
		Scores.write( &BestTime_, sizeof( BestTime_ ) );
		Scores.close();
	}


	ScnCore::pImpl()->removeAllEntities();
	PlayerEntity_ = NULL;
	WorldEntity_ = NULL;
	MonsterEntity_ = NULL;
	TitleEntity_ = NULL;

	GameStage_ = GS_START_TITLE;
}

////////////////////////////////////////////////////////////////////////////////
// startGame
void GaTopState::startGame()
{
	Time_ = 0.0f;

	ScnCore::pImpl()->removeAllEntities();
	PlayerEntity_ = NULL;
	WorldEntity_ = NULL;
	MonsterEntity_ = NULL;
	TitleEntity_ = NULL;

	GameStage_ = GS_START_GAME;

	BestTime_ = BcMax( BestTime_, Time_ );
	Time_ = 0.0f;
}
