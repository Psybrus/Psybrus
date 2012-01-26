/**************************************************************************
*
* File:		GaSwarmComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#ifndef __GaPlayerComponent_H__
#define __GaPlayerComponent_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaMainGameState;
typedef CsResourceRef< class GaPlayerComponent > GaPlayerComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaPlayerComponent
class GaPlayerComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaPlayerComponent );

	virtual void initialise();
	virtual void update( BcReal Tick );
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	eEvtReturn onKeyDown( EvtID ID, const OsEventInputKeyboard& Event );
	eEvtReturn onKeyUp( EvtID ID, const OsEventInputKeyboard& Event );

	void shakeCamera( BcReal Depth );

private:
	class btPairCachingGhostObject*	pGhostObject_;
	class btCharacterControllerInterface* pCharacterController_;
	class btConvexShape* pShape_;

	BcReal Rotation_;

	BcBool MoveU_;
	BcBool MoveD_;
	BcBool MoveL_;
	BcBool MoveR_;
	BcBool Running_;
	
	ScnMaterialComponentRef WorldMaterialComponent_;
	ScnMaterialComponentRef MonsterMaterialComponent_;
	BcU32 InverseDrawDistanceParam_;
	BcU32 NoiseTextureParam_;
	
	BcReal MinDrawDistance_;
	BcReal MaxDrawDistance_;
	BcReal TargetDrawDistance_;
	BcReal ActualDrawDistance_;
	BcReal AdjustSpeed_;
	
	BcReal WaterDropTimer_;

	ScnSoundRef WaterDropSound_;
	ScnSoundRef FootStepSlowSound_;
	ScnSoundRef FootStepFastSound_;
	ScnSoundRef JumpSound_;
	ScnSoundRef LandSound_;

	ScnTextureRef NoiseTexture_[4];

	BcReal NoiseTextureTimer_;
	BcU32 CurrNoiseTexture_;

	ScnSoundRef Ambience0Sound_;
	ScnSoundRef Ambience1Sound_;

	ScnSoundEmitterRef LocalEmitter_;
	ScnSoundEmitterRef Ambience0Emitter_;
	ScnSoundEmitterRef Ambience1Emitter_;

	SsEnvironment Environment_;

	BcReal MoveTimer_;
	BcReal TargetMoveSpeed_;


	BcReal AmbienceVolume_;

	BcBool WasOnGround_;

	BcReal CameraShakeDepth_;
	BcReal CameraShakeTimer_;

private:

};

#endif
