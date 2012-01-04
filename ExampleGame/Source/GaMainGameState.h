/**************************************************************************
*
* File:		GaMainGameState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Main game state.
*		
*
*
* 
**************************************************************************/

#ifndef __GAMAINGAMESTATE_H__
#define __GAMAINGAMESTATE_H__

#include "GaBaseGameState.h"
#include "GaGameComponent.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaLayeredSpriteComponent;

////////////////////////////////////////////////////////////////////////////////
// GaMainGameState
class GaMainGameState: 
	public GaBaseGameState
{
public:
	GaMainGameState();
	virtual ~GaMainGameState();

	virtual void enterOnce();
	virtual eSysStateReturn enter();
	virtual void preMain();
	virtual eSysStateReturn main();
	virtual eSysStateReturn leave();
	virtual void leaveOnce();
	
	virtual void render();

	void doWin();
	void doLose();

	template< class _Ty >
	BcU32 noofEntitiesOfType()
	{
		BcU32 NoofEntities = 0;
		for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
		{
			if( dynamic_cast< _Ty* >( Entities_[ Idx ] ) )
			{
				++NoofEntities;
			}
		}

		return NoofEntities;
	}

	template< class _Ty >
	_Ty* getEntity( BcU32 Index )
	{
		BcU32 NoofEntities = 0;
		for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
		{
			if( CsResourceRef< _Ty >( Entities_[ Idx ] ).isValid() )
			{
				if( NoofEntities++ == Index )
				{
					return CsResourceRef< _Ty >( Entities_[ Idx ] );
				}
			}
		}

		return NULL;
	}

	template< class _Ty >
	_Ty* getNearestEntity( const BcVec2d& Position )
	{
		CsResourceRef< _Ty > pNearestEntity = NULL;
		BcReal NearestDistanceSquared = 1e16f;
		for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
		{
			if( Entities_[ Idx ]->isAlive() && CsResourceRef< _Ty >( Entities_[ Idx ] ).isValid() )
			{
				BcReal DistanceSquared = ( Entities_[ Idx ]->getPosition() - Position ).magnitudeSquared();

				if( DistanceSquared < NearestDistanceSquared )
				{
					NearestDistanceSquared = DistanceSquared;
					pNearestEntity = CsResourceRef< _Ty >( Entities_[ Idx ] );
				}
			}
		}
		return pNearestEntity;
	}

	template< class _Ty >
	_Ty* getFarthestEntity( const BcVec2d& Position )
	{
		CsResourceRef< _Ty > pFarthestEntity = NULL;
		BcReal FarthestDistanceSquared = 0.0f;
		for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
		{
			if( Entities_[ Idx ]->isAlive() && CsResourceRef< _Ty >( Entities_[ Idx ] ).isValid() )
			{
				BcReal DistanceSquared = ( Entities_[ Idx ]->getPosition() - Position ).magnitudeSquared();

				if( DistanceSquared > FarthestDistanceSquared )
				{
					FarthestDistanceSquared = DistanceSquared;
					pFarthestEntity = CsResourceRef< _Ty >( Entities_[ Idx ] );
				}
			}
		}
		return pFarthestEntity;
	}

	void setMaterialComponentParams( ScnMaterialComponentRef MaterialComponentRef, const BcMat4d& WorldView );

	void getWorldPosition( const BcVec2d& ScreenPosition, BcVec3d& Near, BcVec3d& Far );

	// TODO: Move to data.
	void spawnPlayerEntity();
	void spawnSwarmEntity( BcU32 Level );
	void spawnFoodEntity( const BcVec2d& Position );

	void killEntity( GaGameComponentRef Component );
	void spawnKill();

private:
	typedef std::vector< GaGameComponentRef > TEntityList;
	typedef TEntityList::iterator TEntityListIterator;

	TEntityList Entities_;

	TEntityList SpawnEntities_;
	TEntityList KillEntities_;

	BcVec2d WorldHalfSize_;
	BcMat4d Projection_;
	BcMat4d WorldView_;

	BcReal FoodHealth_;

	RsContext* pContext_;

	ScnMaterialComponentRef BackgroundMaterialComponent_;
	ScnMaterialComponentRef ForegroundMaterialComponent_;
	ScnMaterialComponentRef BarMaterialComponent_;
};

#endif
