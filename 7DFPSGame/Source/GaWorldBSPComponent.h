/**************************************************************************
*
* File:		GaWorldBSPComponent.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		World BSP
*		
*
*
* 
**************************************************************************/

#ifndef __GAWORLDBSPCOMPONENT_H__
#define __GAWORLDBSPCOMPONENT_H__

#include "Psybrus.h"

#include "GaWorldInfoComponent.h"

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResourceRef< class GaWorldBSPComponent > GaWorldBSPComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaWorldBSPPoint
struct GaWorldBSPPoint
{
	BcVec2d Position_;
};

////////////////////////////////////////////////////////////////////////////////
// Events
#define GA_EVENTGROUP_CORE		EVT_MAKE_ID( 'G', 'a', 0 )

enum GaEvents
{
	// All core events.
	gaEVT_CORE_FIRST			= GA_EVENTGROUP_CORE,
	gaEVT_CORE_RESET,
};

////////////////////////////////////////////////////////////////////////////////
// GaWorldResetEvent
struct GaWorldResetEvent: EvtEvent< GaWorldResetEvent >
{
	BcVec2d Position_;
	BcBool HasWeapon_;
};

//////////////////////////////////////////////////////////////////////////
// GaWorldBSPVertex
struct GaWorldBSPVertex
{
	BcF32 X_, Y_, Z_;
	BcF32 U_, V_, W_;
	BcU32 RGBA_;
};

//////////////////////////////////////////////////////////////////////////
// GaWorldBSPEdge
struct GaWorldBSPEdge
{
	GaWorldBSPEdge():
		A_( BcErrorCode ),
		B_( BcErrorCode )
	{
	}

	GaWorldBSPEdge( BcU32 A, BcU32 B ):
		A_( A ),
		B_( B )
	{
	}
	
	BcU32 A_;
	BcU32 B_;
};

//////////////////////////////////////////////////////////////////////////
// GaWorldBSPComponent
class GaWorldBSPComponent: public ScnComponent
{
public:
	DECLARE_RESOURCE( ScnComponent, GaWorldBSPComponent );

public:
	virtual void						initialise( const Json::Value& Object );
	virtual BcBool						isReady();
	virtual void						update( BcF32 Tick );
	virtual void						render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	eEvtReturn							onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event );
	eEvtReturn							onMouseEvent( EvtID ID, const OsEventInputMouse& Event );

	BcU32								nearestPoint( BcVec2d Position, BcF32 Radius );
	BcU32								addPoint( BcVec2d Position );
	BcBool								addEdge( BcU32 IdxA, BcU32 IdxB );
	BcBool								addEnemy( const BcVec2d& Position );

	void								removePoint( BcU32 Idx );
	void								removeEdge( BcU32 Idx );
	void								removeEnemy( BcU32 Idx );

	void								invertEdge( BcU32 Idx );

	BcU32								nearestEdge( const BcVec2d& Position, BcF32 Radius );
	BcVec2d								nearestPositionOnEdge( const BcVec2d& Position, BcU32 Idx );
	BcU32								nearestEnemy( BcVec2d Position, BcF32 Radius );

	BcBool								killEnemy( const BcVec3d& Position, BcF32 Radius );
	void								killPlayer();

	BcBool								canSeePlayer(const BcVec3d& From );

	void								saveJson();
	void								loadJson();
	void								createEntities();
	void								destroyEntities();

	void								buildBSP();

	BcBool								checkPointFront( const BcVec3d& Point, BcF32 Radius, BcBSPInfo* pData = NULL, BcBSPNode* pNode = NULL );
	BcBool								checkPointBack( const BcVec3d& Point, BcF32 Radius, BcBSPInfo* pData = NULL, BcBSPNode* pNode = NULL );
	BcBool								lineIntersection( const BcVec3d& A, const BcVec3d& B, BcBSPPointInfo* pPointInfo, BcBSPNode* pNode = NULL );

	void								clearMessages();
	void								addMessage( const BcChar* pMessage );
	
public:
	BcBool								IsEditor_;
	BcBool								InEditorMode_;

	enum EditorState
	{
		ES_IDLE = 0,
		ES_ADD_POINTS,

		//
		ES_MAX
	};

	EditorState							EditorState_;
	ScnCanvasComponentRef				Canvas_;
	GaWorldInfoComponentRef				WorldInfo_;
	ScnMaterialComponentRef				Material_;
	ScnMaterialComponentRef				MaterialWorld_;
	ScnFontComponentRef					Font_;
	BcU32								FontScaleParam_;
	
	BcMat4d								Projection_;

	std::vector< GaWorldBSPPoint >		Points_;
	std::vector< GaWorldBSPEdge >		Edges_;
	std::vector< BcVec2d >				Enemies_;
	std::vector< ScnEntityRef >			EnemyEntities_;
	ScnEntityRef						PlayerEntity_;

	BcVec2d								StartPosition_;
	BcVec2d								QuitPosition_;

	BcU32								LastPointIdx_;

	BcVec2d								MousePosition_;
	BcVec2d								MousePointPosition_;

	BcU32								NearestPoint_;
	BcU32								NearestEdge_;
	BcU32								NearestEnemy_;

	BcBSPTree*							pBSPTree_;

	GaWorldBSPVertex*					pVertexArray_;
	RsVertexBuffer*						pVertexBuffer_;
	RsPrimitive*						pPrimitive_;

	BcU32								CurrentLevel_;
	BcU32								TotalLevels_;

	BcF32								TextTimer_;
	std::vector< std::string >			TextList_;
};

#endif
