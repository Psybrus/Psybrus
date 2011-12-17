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
class GaPlayerEntity;
class GaSwarmEntity;

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

private:
	GaPlayerEntity* pPlayer_;
	GaSwarmEntity* pSwarm_;

	BcMat4d Projection_;

};

#endif
