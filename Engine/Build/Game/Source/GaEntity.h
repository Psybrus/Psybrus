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

#ifndef __GAENTITY_H__
#define __GAENTITY_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// Forward Declarations
class GaMainGameState;

////////////////////////////////////////////////////////////////////////////////
// GaEntity
class GaEntity
{
public:
	enum 
	{
		EMOTE_EATING,
		EMOTE_SCARED,
		EMOTE_RUNAWAY
	};

public:
	GaEntity();
	virtual ~GaEntity();

	virtual void update( BcReal Tick );
	virtual void render( ScnCanvasRef Canvas );
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

	ScnMaterialInstanceRef EmoteMaterialInstance_;

	BcU32 LastEmote_;
	BcReal EmoteTimer_;

};

#endif
