/**************************************************************************
*
* File:		GaGameComponent.h
* Author:	Neil Richardson 
* Ver/Date:	29/12/11	
* Description:
*		Example user component.
*		
*
*
* 
**************************************************************************/

#ifndef __GaGameComponent_H__
#define __GaGameComponent_H__

#include "Psybrus.h"

#include "GaGameSimulator.h"

//////////////////////////////////////////////////////////////////////////
// GaGameComponentRef
typedef CsResourceRef< class GaGameComponent > GaGameComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaGameComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaGameComponent );

	void								initialise();
	void								destroy();

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	eEvtReturn							onKeyEvent( EvtID ID, const OsEventInputKeyboard& Event );
	eEvtReturn							onMouseEvent( EvtID ID, const OsEventInputMouse& Event );

private:
	ScnCanvasComponentRef				CanvasComponent_;

	ScnMaterialComponentRef				SpriteSheetMaterial_;
	ScnMaterialComponentRef				HUDMaterial_;

	GaGameSimulator*					pSimulator_;

	// 
	BcBool								MouseDown_;
	BcBool								BoxSelection_;
	BcFixedVec2d						CursorPosition_;
	BcFixedVec2d						GameCursorPosition_;
	BcFixedVec2d						StartGameCursorPosition_;
	BcFixedVec2d						EndGameCursorPosition_;

	BcBool								CtrlDown_;
	BcBool								AttackMove_;
	
	// Unit selection.
	GaGameUnitIDList					UnitSelection_;
	std::vector< GaGameUnitIDList >		ControlGroups_;

};

#endif

