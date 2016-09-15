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

	/**
	 * Create physics mesh with data.
	 * Data pass in here will be owned by this, and freed upon destruction.
	 */
	ScnPhysicsMesh( const ScnPhysicsMeshHeader& Header, 
		const ScnPhysicsMeshPart* MeshParts,
		const ScnPhysicsTriangle* Triangles, 
		const ScnPhysicsVertex* Vertices );
	virtual ~ScnPhysicsMesh();
	
	void create() override;
	void destroy() override;

	class btCollisionShape* createCollisionShape();

protected:
	void fileReady() override;
	void fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData ) override;
	
protected:
	ScnPhysicsMeshHeader Header_;
	const ScnPhysicsMeshPart* MeshParts_;
	const ScnPhysicsTriangle* Triangles_;
	const ScnPhysicsVertex* Vertices_;

	BcBool OwnsData_;

	class btTriangleIndexVertexArray* MeshInterface_;
	class btOptimizedBvh* OptimizedBvh_;

	SysFence BuildingBvhFence_;
};


#endif


