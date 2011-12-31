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

#ifndef __GaGameComponent_H__
#define __GaGameComponent_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaMainGameState;
typedef CsResourceRef< class GaGameComponent > GaGameComponentRef;

////////////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaGameComponent:
	public ScnComponent
{
public:
	enum 
	{
		EMOTE_EATING,
		EMOTE_SCARED,
		EMOTE_RUNAWAY
	};

public:
	DECLARE_RESOURCE( ScnComponent, GaGameComponent );

	virtual void initialise();
	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasComponentRef Canvas );
	virtual BcBool isAlive(){ return BcTrue; };

	BcForceInline void setParent( GaMainGameState* pParent )
	{
		pParent_ = pParent;
	}
	
	BcForceInline GaMainGameState* pParent()
	{
		return pParent_;
	}
	
	BcForceInline const BcVec2d& getPosition() const
	{
		return Position_;
	}
	
	BcForceInline void setProjection( const BcMat4d& Projection )
	{
		Projection_ = Projection;
	}

	BcBool doEmote( BcU32 Idx, const BcVec3d& Position );

protected:
	GaMainGameState* pParent_;
	BcMat4d Projection_;
	BcVec2d Position_;

	struct TEmote
	{
		BcU32 Idx_;
		BcReal Timeleft_;
		BcVec3d Position_;
	};

	typedef std::vector< TEmote > TEmoteList;
	TEmoteList EmoteList_;

	ScnMaterialComponentRef EmoteMaterialComponent_;

	BcU32 LastEmote_;
	BcReal EmoteTimer_;

	BcMutex EmoteLock_;
};

#endif
