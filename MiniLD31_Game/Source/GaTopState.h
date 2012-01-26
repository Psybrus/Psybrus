/**************************************************************************
*
* File:		GaTopState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Game top state.
*		
*
*
* 
**************************************************************************/

#ifndef __GATOPSTATE_H__
#define __GATOPSTATE_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// GaTopState
class GaTopState: 
	public SysState,
	public BcGlobal< GaTopState >
{
public:
	GaTopState();
	virtual ~GaTopState();

	virtual void					enterOnce();
	virtual eSysStateReturn			enter();
	virtual void					preMain();
	virtual eSysStateReturn			main();
	virtual void					preLeave();
	virtual eSysStateReturn			leave();
	virtual void					leaveOnce();
	
	void							startTitle();
	void							startGame();

private:
	ScnModelRef						WorldModel_;
	ScnModelRef						MonsterModel_;
	ScnRigidBodyRef					WorldRigidBody_;
	ScnRigidBodyRef					PlayerRigidBody_;

	ScnSoundRef						WaterDropSound_;
	ScnSoundRef						FootStepSlowSound_;
	ScnSoundRef						FootStepFastSound_;
	ScnSoundRef						Ambience0Sound_;
	ScnSoundRef						Ambience1Sound_;
	ScnSoundRef						JumpSound_;
	ScnSoundRef						LandSound_;
	ScnSoundRef						FootStepSound_;
	ScnSoundRef						MonsterAlertSound_;
	ScnSoundRef						MonsterRoarSound_[4];
	ScnSoundRef						DeadSound_;

	ScnMaterialRef					TitleMaterial_;

	ScnTextureRef					NoiseTexture_[4];

	// Entities.
	ScnEntityRef					WorldEntity_;
	ScnEntityRef					PlayerEntity_;
	ScnEntityRef					MonsterEntity_;

	ScnEntityRef					TitleEntity_;

	enum GameStage
	{
		GS_IDLE = 0,
		GS_START_TITLE,
		GS_START_GAME
	};
	GameStage						GameStage_;

public:
	BcReal							Time_;
	BcReal							PrevTime_;
	BcReal							BestTime_;
};

#endif
