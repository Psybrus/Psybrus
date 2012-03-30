/**************************************************************************
*
* File:		GaTitleComponent.h
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Title component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaTitleComponent_H__
#define __GaTitleComponent_H__

#include "Psybrus.h"

//////////////////////////////////////////////////////////////////////////
// GaGameComponentRef
typedef CsResourceRef< class GaTitleComponent > GaTitleComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaTitleComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaTitleComponent );

	void								initialise();
	void								destroy();

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	eEvtReturn							onKeyEvent( EvtID ID, const OsEventInputKeyboard& Event );

private:
	ScnCanvasComponentRef				Canvas_;
	ScnMaterialComponentRef				Material_;
	ScnMaterialComponentRef				FontMaterial_;
	ScnFontComponentRef					Font_;

	class GaMatchmakingState*			pState_;
};

#endif
