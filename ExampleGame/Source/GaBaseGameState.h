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

	virtual void enterOnce();
	virtual eSysStateReturn enter();

	/**
	 * Perform rendering for state.
	 */
	virtual void				render();

protected:
	ScnEntityRef				CameraEntity_;
	ScnCanvasComponentRef		Canvas_;
	ScnMaterialComponentRef		DefaultMaterialComponent_;

private:

};

#endif
