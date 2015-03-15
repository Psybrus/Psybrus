/**************************************************************************
*
* File:		ScnPhysicsMesh.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnPhysicsMesh_H__
#define __ScnPhysicsMesh_H__

#include "System/Content/CsResource.h"

#include "System/Scene/Physics/ScnPhysicsFileData.h"
#include "System/SysFence.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsMeshRef
typedef ReObjectRef< class ScnPhysicsMesh > ScnPhysicsMeshRef;

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsMesh
class ScnPhysicsMesh:
	public CsResource
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsMesh, CsResource );
	
	ScnPhysicsMesh();
	virtual ~ScnPhysicsMesh();
	
	virtual void create();
	virtual void destroy();

	class btCollisionShape* createCollisionShape();

protected:
	void fileReady();
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData );
	
protected:
	ScnPhysicsMeshHeader Header_;
	const ScnPhysicsTriangle* Triangles_;
	const ScnPhysicsVertex* Vertices_;

	class btStridingMeshInterface* MeshInterface_;
	class btOptimizedBvh* OptimizedBvh_;

	SysFence BuildingBvhFence_;
};


#endif


