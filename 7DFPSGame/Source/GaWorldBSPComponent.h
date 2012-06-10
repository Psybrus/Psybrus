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

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef CsResourceRef< class GaWorldBSPComponent > GaWorldBSPComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaWorldBSPPoint
struct GaWorldBSPPoint
{
	BcVec2d Position_;
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
class GaWorldBSPComponent: public ScnRenderableComponent
{
public:
	DECLARE_RESOURCE( ScnRenderableComponent, GaWorldBSPComponent );

public:
	virtual void						initialise( const Json::Value& Object );
	virtual void						create();
	virtual void						destroy();
	virtual BcBool						isReady();
	virtual void						update( BcReal Tick );
	virtual void						render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort );
	virtual void						onAttach( ScnEntityWeakRef Parent );
	virtual void						onDetach( ScnEntityWeakRef Parent );

	eEvtReturn							onKeyboardEvent( EvtID ID, const OsEventInputKeyboard& Event );
	eEvtReturn							onMouseEvent( EvtID ID, const OsEventInputMouse& Event );

	BcU32								nearestPoint( BcVec2d Position, BcReal Radius );
	BcU32								addPoint( BcVec2d Position );
	BcBool								addEdge( BcU32 IdxA, BcU32 IdxB );

	void								removePoint( BcU32 Idx );
	void								removeEdge( BcU32 Idx );

	void								invertEdge( BcU32 Idx );

	BcU32								nearestEdge( const BcVec2d& Position, BcReal Radius );
	BcVec2d								nearestPositionOnEdge( const BcVec2d& Position, BcU32 Idx );
	
	void								saveJson();
	void								loadJson();
	void								buildBSP();

	BcBool								checkPointFront( const BcVec3d& Point, BcReal Radius, BcBSPInfo* pData = NULL, BcBSPNode* pNode = NULL );
	BcBool								checkPointBack( const BcVec3d& Point, BcReal Radius, BcBSPInfo* pData = NULL, BcBSPNode* pNode = NULL );
	BcBool								lineIntersection( const BcVec3d& A, const BcVec3d& B, BcBSPPointInfo* pPointInfo, BcBSPNode* pNode = NULL );


public:
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
	ScnMaterialComponentRef				Material_;
	ScnMaterialComponentRef				MaterialWorld_;
	BcU32								WorldTransformParam_;
	BcMat4d								Projection_;

	std::vector< GaWorldBSPPoint >		Points_;
	std::vector< GaWorldBSPEdge >		Edges_;

	BcU32								LastPointIdx_;

	BcVec2d								MousePosition_;
	BcVec2d								MousePointPosition_;

	BcU32								NearestPoint_;
	BcU32								NearestEdge_;

	BcBSPTree*							pBSPTree_;

	GaWorldBSPVertex*					pVertexArray_;
	RsVertexBuffer*						pVertexBuffer_;
	RsPrimitive*						pPrimitive_;
};

#endif
