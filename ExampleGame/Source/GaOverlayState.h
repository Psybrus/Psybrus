/**************************************************************************
*
* File:		GaOverlayState.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Overlay state.
*		
*
*
* 
**************************************************************************/

#ifndef __GAOVERLAYSTATE_H__
#define __GAOVERLAYSTATE_H__

#include "GaBaseGameState.h"

////////////////////////////////////////////////////////////////////////////////
// GaOverlayState
class GaOverlayState: 
	public GaBaseGameState
{
public:
	GaOverlayState( ScnMaterialRef Material, const std::string& Text, GaBaseGameState* pNextState );
	virtual ~GaOverlayState();

	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );

	virtual void enterOnce();
	virtual eSysStateReturn enter();
	virtual void preMain();
	virtual eSysStateReturn main();
	virtual eSysStateReturn leave();
	virtual void leaveOnce();
	
	virtual void render();

private:
	ScnMaterialRef FontMaterial_;
	ScnMaterialComponentRef MaterialComponent_;
	ScnFontComponentRef FontComponent_;

	BcU32 CurrOverlay_;

	std::string Text_;

	GaBaseGameState* pNextState_;
};

#endif
