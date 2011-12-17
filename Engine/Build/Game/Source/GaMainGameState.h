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

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaEntity;

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
	
	virtual void render( RsFrame* pFrame );

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
	}

	template< class _Ty >
	_Ty* getEntity( BcU32 Index )
	{
		BcU32 NoofEntities = 0;
		for( BcU32 Idx = 0; Idx < Entities_.size(); ++Idx )
		{
			if( dynamic_cast< _Ty* >( Entities_[ Idx ] ) )
			{
				if( NoofEntities ++ == Idx )
				{
					return dynamic_cast< _Ty* >( Entities_[ Idx ] );
				}
			}
		}

		return NULL;
	}

	void spawnEntity( GaEntity* pEntity );
	void killEntity( GaEntity* pEntity );


private:
	typedef std::vector< GaEntity* > TEntityList;
	typedef TEntityList::iterator TEntityListIterator;

	TEntityList Entities_;

	TEntityList SpawnEntities_;
	TEntityList KillEntities_;

	BcMat4d Projection_;

};

#endif
