/**************************************************************************
*
* File:		GaCameraComponent.h
* Author:	Neil Richardson 
* Ver/Date:	15/12/12	
* Description:
*		Game state component. Logic.
*		
*
*
* 
**************************************************************************/

#ifndef __GAGAMESTATECOMPONENT_H__
#define __GAGAMESTATECOMPONENT_H__

#include "Psybrus.h"

#include "GaEvents.h"

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
typedef CsResourceRef< class GaGameStateComponent > GaGameStateComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaGameStateComponent
class GaGameStateComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaGameStateComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );
	
	eEvtReturn							onMouseDown( EvtID ID, const OsEventInputMouse& Event );
	eEvtReturn							onPlayerDie( EvtID ID, const EvtNullEvent& Event );
	eEvtReturn							onEnemyDie( EvtID ID, const EvtNullEvent& Event );

private:
	enum GameState
	{
		GS_SPLASH,
		GS_GAME,
		GS_WIN,
		GS_LOSE
	};

	GameState							GameState_;

	ScnCanvasComponentRef				Canvas_;
	ScnFontComponentRef					Font_;

	ScnEntityRef						PlayerEntity_;
	ScnEntityRef						EnemyEntity_;

};

#endif

