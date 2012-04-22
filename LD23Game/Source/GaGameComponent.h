/**************************************************************************
*
* File:		GaGameComponent.h
* Author:	Neil Richardson 
* Ver/Date:	21/04/12
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
#include "GaStrongForceComponent.h"

//////////////////////////////////////////////////////////////////////////
// Statics.
static const BcReal WORLD_SIZE = 32.0f;

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
	void								destroy();

	virtual void						update( BcReal Tick );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	//
private:
	struct TElement
	{
		BcName Type_;					//< What we are.
		BcName FuseType_;				//< What we fuse with.
		BcName ReplaceType_;			//< What do we become after fusion?
		BcName RespawnType_;			//< What do we become when we fly out of bounds?
		ScnEntityRef Entity_;
		GaElementComponentRef Element_;
		BcBool MarkedForFusion_;
		
		BcVec3d Position_;
		BcVec3d Velocity_;
	};

	void								spawnElement( const BcVec3d& Position, const BcVec3d& Velocity, const BcName& Type );
	
	void								updateSimulation( BcReal Tick );
	BcU32								findNearestOfType( const BcVec3d& Position, BcReal Radius, const BcName& Type, BcU32 Exclude = BcErrorCode );
	BcU32								findNearestNotOfType( const BcVec3d& Position, BcReal Radius, const BcName& Type, BcU32 Exclude = BcErrorCode );

	void								addElements();
	void								removeEntities();
	BcBool								inRemoveEntityList( ScnEntityRef Entity );

	BcReal&								getHeatMapValue( BcS32 X, BcS32 Y, BcU32 Buffer = 0 );
	void								updateHeatMapTexture();
	void								addHeatMapValue( const BcVec3d& Position, BcReal Value );
	BcReal								getHeatMapValue( const BcVec3d& Position );
	
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
	ScnParticleSystemComponentRef		ParticleSystem_;

	GaStrongForceComponentRef			StrongForce_;


	// Element logic.
	typedef std::vector< TElement >		TElementList;
	typedef TElementList::iterator		TElementListIterator;

	TElementList						ElementList_;
	TElementList						AddElementList_;
	
	ScnEntityList						RemoveEntityList_;

	BcReal								SpawnTimer_;
	BcU32								MaxElements_;

	// Heat map.
	ScnTextureRef						HeatMapTexture_;
	ScnModelComponentRef				HeatMapModel_;
	ScnMaterialComponentRef				HeatMapMaterial_;
	BcReal*								pHeatMap_;
	BcReal*								pHeatMapBuffer_;
	BcU32								HeatMapWidth_;
	BcU32								HeatMapHeight_;

	// Sun
	ScnModelComponentRef				SunModel_;
	ScnMaterialComponentRef				SunMaterial_;

};

#endif
