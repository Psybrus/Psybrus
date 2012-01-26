/**************************************************************************
*
* File:		GaMonsterComponent.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#ifndef __GaMonsterComponent_H__
#define __GaMonsterComponent_H__

#include "Psybrus.h"

#include "GaPlayerComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaMainGameState;
typedef CsResourceRef< class GaMonsterComponent > GaMonsterComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaMonsterComponent
class GaMonsterComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaMonsterComponent );

	virtual void initialise();
	virtual void update( BcReal Tick );
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );
	
private:
	ScnEntityRef			PlayerEntity_;		// Cache this so we can quickly access info on it.
	GaPlayerComponentRef	PlayerComponent_;

	ScnSoundRef				FootStepSound_;
	ScnSoundRef				MonsterAlertSound_;
	ScnSoundRef				MonsterRoarSound_[4];
	ScnSoundRef				DeadSound_;

	ScnSoundEmitterRef		LocalEmitter_;
	
	BcReal					FootStepTimer_;
	BcReal					IdleTimer_;

	BcReal					KillTimer_;
	
	BcReal					TrackSpeed_;
	BcReal					MoveSpeed_;
	BcReal					MoveCircle_;
	BcVec3d					TargetPosition_;	// Where we need to tend towards.

	enum MonsterState
	{
		MS_IDLE = 0,			// wandering round aimlessly (circle player).
		MS_HUNT,				// figure of 8 the player (intent to intercept, stay behind player)
		MS_KILL,				// run at player and kill them! ()
		MS_WIN,
	};

	MonsterState			MonsterState_;

};

#endif
