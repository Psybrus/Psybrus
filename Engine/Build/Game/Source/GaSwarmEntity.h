/**************************************************************************
*
* File:		GaSwarmEntity.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Swarm.
*		
*
*
* 
**************************************************************************/

#ifndef __GASWAWM_H__
#define __GASWAWM_H__

#include "Psybrus.h"

#include "GaEntity.h"

#include "GaPhysicsBody.h"

#include "GaBunnyRenderer.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class GaFoodEntity;

////////////////////////////////////////////////////////////////////////////////
// GaSwarmEntity
class GaSwarmEntity:
	public GaEntity
{
public:
	GaSwarmEntity( BcU32 Level );
	virtual ~GaSwarmEntity();
	
	GaPhysicsBody* findNearestBody( GaPhysicsBody* pSource );
	BcVec2d averageVelocity() const;
	BcVec2d averagePosition() const;
	
	virtual BcBool isAlive();

	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	
	virtual void update( BcReal Tick );

	virtual void render( ScnCanvasRef Canvas );
	
public:
	struct TAnimationLogic
	{
		TAnimationLogic():
			StartPosition_( 0.0f, 0.0f ),
			EndPosition_( 0.0f, 0.0f ),
			AnimationPosition_( 1.0f ),
			AnimationSpeed_( 1.0f ),
			NotNeeded_( BcFalse )
		{
		};

		BcVec2d StartPosition_;
		BcVec2d EndPosition_;
		BcReal AnimationPosition_;
		BcReal AnimationSpeed_;
		BcBool NotNeeded_;
		GaBunnyRenderer BunnyRenderer_;
		ScnSoundEmitterRef Emitter_;
	};
	
	void updateAnimation( BcU32 Idx, BcReal Tick  );
	BcVec3d animationPosition( BcU32 Idx ) const;
	BcBool shouldStartMoveAnimation( BcU32 Idx );
	void startMoveAnimation( BcU32 Idx, const BcVec2d& Start, const BcVec2d& End );
	
	void updateBody_Threaded( BcReal Tick, GaFoodEntity* pFoodEntity, GaPhysicsBody* pBody, TAnimationLogic* pAnimationLogic );

private:
	typedef std::vector< GaPhysicsBody* > TBodyList;
	typedef std::vector< TAnimationLogic* > TAnimationLogicList;
	
	TBodyList Bodies_;
	TAnimationLogicList AnimationLogicList_;
	
	GaFoodEntity* pTargetFoodEntity_;
};

#endif
