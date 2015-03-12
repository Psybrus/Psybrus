/**************************************************************************
*
* File:		ScnPhysicsFileData.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNPHYSICSFILEDATA__
#define __SCNPHYSICSFILEDATA__

#include "Base/BcTypes.h"
#include "Math/MaVec3d.h"
#include "Math/MaAABB.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsCollisionShapeHeader
struct ScnPhysicsMeshHeader
{
	BcU32 NoofTriangles_;
	BcU32 NoofVertices_;
	MaAABB AABB_;
};

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsTriangle
struct ScnPhysicsTriangle
{
	BcU32 Indices_[ 3 ];
};

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsVertex
struct ScnPhysicsVertex
{
	MaVec4d Position_;
};

#endif
