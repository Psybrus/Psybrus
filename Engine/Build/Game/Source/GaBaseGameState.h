/**************************************************************************
*
* File:		GaBaseGameState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Base game state.
*		
*
*
* 
**************************************************************************/

#ifndef __GABASEGAMESTATE_H__
#define __GABASEGAMESTATE_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// GaBaseGameState
class GaBaseGameState: 
	public SysState
{
public:
	GaBaseGameState();
	virtual ~GaBaseGameState();

	/**
	 * Perform rendering for state.
	 */
	virtual void render( RsFrame* pFrame );

private:

};

#endif
