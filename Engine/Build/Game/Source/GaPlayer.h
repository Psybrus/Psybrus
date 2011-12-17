/**************************************************************************
*
* File:		GaPlayer.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Player.
*		
*
*
* 
**************************************************************************/

#ifndef __GAPLAYER_H__
#define __GAPLAYER_H__

#include "Psybrus.h"

////////////////////////////////////////////////////////////////////////////////
// GaPlayer
class GaPlayer
{
public:
	GaPlayer();
	virtual ~GaPlayer();

	eEvtReturn onMouseMove( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn onMouseUp( EvtID ID, const OsEventInputMouse& Event );

private:

};

#endif
