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

#ifndef __GAGAMECOMPONENT_H__
#define __GAGAMECOMPONENT_H__

#include "Psybrus.h"
#include "GaElementComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef CsResourceRef< class GaGameComponent > GaGameComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaGameComponent
class GaGameComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaGameComponent );

	void								initialise( const Json::Value& Object );

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	//
private:
	struct TElement
	{
		BcName Type_;
		ScnEntityRef Entity_;
		GaElementComponentRef Element_;
		
		BcVec3d Position_;
		BcVec3d Velocity_;
	};

	void								spawnElement( const BcVec3d& Position, const BcName& Type );
	void								updateSimulation( BcReal Tick );
	BcU32								findNearestOfType( const BcVec3d& Position, const BcName& Type, BcU32 Exclude = BcErrorCode );
	
private:
	enum GameState
	{
		GS_INIT = 0,
		GS_UPDATE,
		GS_EXIT,
	};

	GameState							GameState_;

	// Components.
	ScnCanvasComponentRef				Canvas_;
	ScnFontComponentRef					Font_;

	// Element logic.
	std::vector< TElement >				ElementList_;
};

#endif
