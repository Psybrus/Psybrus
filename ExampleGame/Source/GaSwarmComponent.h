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

#ifndef __GASWAWM_H__
#define __GASWAWM_H__

#include "Psybrus.h"

#include "GaGameComponent.h"

#include "GaPhysicsBody.h"

#include "GaLayeredSpriteComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class GaFoodComponent;

////////////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResourceRef< class GaSwarmComponent > GaSwarmComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaSwarmComponent
class GaSwarmComponent:
	public GaGameComponent
{
public:
	DECLARE_RESOURCE( GaGameComponent, GaSwarmComponent );

	virtual void initialise( BcU32 Level );
	virtual void destroy();
	
	GaPhysicsBody* findNearestBody( GaPhysicsBody* pSource );
	BcVec2d averageVelocity() const;
	BcVec2d averagePosition() const;
	
	virtual BcBool isAlive();

	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	
	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasComponentRef Canvas );

	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

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
		GaLayeredSpriteComponentRef LayeredSpriteComponent_;
		ScnSoundEmitterRef Emitter_;
	};
	
	void updateAnimation( BcU32 Idx, BcReal Tick  );
	BcVec3d animationPosition( BcU32 Idx ) const;
	BcBool shouldStartMoveAnimation( BcU32 Idx );
	void startMoveAnimation( BcU32 Idx, const BcVec2d& Start, const BcVec2d& End );
	
	void updateBody_Threaded( BcReal Tick, GaFoodComponent* pFoodEntity, GaPhysicsBody* pBody, TAnimationLogic* pAnimationLogic );

private:
	typedef std::vector< GaPhysicsBody* > TBodyList;
	typedef std::vector< TAnimationLogic* > TAnimationLogicList;
	
	TBodyList Bodies_;
	TAnimationLogicList AnimationLogicList_;
	
	GaFoodComponent* pTargetFoodEntity_;
};

#endif
