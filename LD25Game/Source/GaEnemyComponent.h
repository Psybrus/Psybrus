/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Enemy component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaEnemyComponent_H__
#define __GaEnemyComponent_H__

#include "Psybrus.h"

#include "GaEvents.h"

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
typedef CsResourceRef< class GaEnemyComponent > GaEnemyComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaEnemyComponent
class GaEnemyComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaEnemyComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
	void								particleHit( BcVec3d Position );

	eEvtReturn							onPlayerShoot( EvtID ID, const GaEventShoot& Event );
	eEvtReturn							onPlayerPosition( EvtID ID, const GaEventPosition& Event );

private:
	ScnMaterialComponentRef				DefaultMaterial_;
	ScnParticleSystemComponentRef		GameParticles_;
	ScnCanvasComponentRef				Canvas_;

	BcVec3d								Position_;
	BcVec3d								TargetPosition_;

	BcReal								MaxHealth_;
	BcReal								Health_;
	BcReal								EventHealthAmount_;
	BcReal								NextEventHealth_;

	enum GameState
	{
		GS_EVADING = 0,
		GS_TURNING,
		GS_FACING,
		GS_TURNING_BACK,
		GS_ATTACKING
	};

	GameState							GameState_;

	BcReal								TurnTimer_;

	BcReal								ProjectileTime_;
	BcReal								ProjectileTimer_;

	BcVec3d								Rotation_;
};

#endif

