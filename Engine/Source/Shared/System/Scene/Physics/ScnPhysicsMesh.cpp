/**************************************************************************
*
* File:		ScnPhysicsMesh.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/
#include "System/Scene/Physics/ScnPhysicsMesh.h"
#include "System/Scene/Physics/ScnPhysics.h"

#include "System/SysKernel.h"

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Physics/ScnPhysicsMeshImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnPhysicsMesh );

void ScnPhysicsMesh::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Header_", &ScnPhysicsMesh::Header_, bcRFF_CHUNK_DATA ),
	};
		
	auto& Class = ReRegisterClass< ScnPhysicsMesh, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnPhysicsMeshImport::StaticGetClass(), 1 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsMesh::ScnPhysicsMesh():
	Header_(),
	Triangles_( nullptr ),
	Vertices_( nullptr ),
	MeshInterface_( nullptr ),
	OptimizedBvh_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnPhysicsMesh::~ScnPhysicsMesh()
{
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnPhysicsMesh::create()
{
	// Create mesh interface.
	MeshInterface_ = new btTriangleIndexVertexArray();

	// Add the mesh parts.
	BcU32 TriangleOffset = 0;
	BcU32 VertexOffset = 0;
	for( BcU32 Idx = 0; Idx < Header_.NoofMeshParts_; ++Idx )
	{
		const auto & MeshPart = MeshParts_[ Idx ];
		btIndexedMesh IndexedMesh;
		IndexedMesh.m_numTriangles = MeshPart.NoofTriangles_;
		IndexedMesh.m_triangleIndexBase = reinterpret_cast< const unsigned char* >( Triangles_ + TriangleOffset );
		IndexedMesh.m_triangleIndexStride = sizeof( ScnPhysicsTriangle );
		IndexedMesh.m_numVertices = MeshPart.NoofVertices_;
		IndexedMesh.m_vertexBase = reinterpret_cast< const unsigned char* >( Vertices_ + VertexOffset );
		IndexedMesh.m_vertexStride = sizeof( ScnPhysicsVertex );
		IndexedMesh.m_indexType = PHY_INTEGER;
		IndexedMesh.m_vertexType = PHY_FLOAT;

		MeshInterface_->addIndexedMesh( IndexedMesh );

		TriangleOffset += MeshPart.NoofTriangles_;
		VertexOffset += MeshPart.NoofVertices_;
	}

	// Construct optimized bvh for static mesh.
	if( Header_.ShapeType_ == ScnPhysicsMeshShapeType::BVH )
	{
		// Create optimised bvh.
		OptimizedBvh_ = new btOptimizedBvh();

		// Build optimised bvh on a worker, mark as ready immediately
		// to maximise concurrency, just wait on access.
		BuildingBvhFence_.increment();
		SysKernel::pImpl()->pushFunctionJob(
			BcErrorCode,
			[ this ]
			{
				OptimizedBvh_->build( 
					MeshInterface_, true, 
					ScnPhysicsToBullet( Header_.AABB_.min() ), ScnPhysicsToBullet( Header_.AABB_.max() ) );
				BuildingBvhFence_.decrement();
			} );
	}

	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnPhysicsMesh::destroy()
{
	BuildingBvhFence_.wait();

	delete MeshInterface_;
	delete OptimizedBvh_;
}

//////////////////////////////////////////////////////////////////////////
// createCollisionShape
btCollisionShape* ScnPhysicsMesh::createCollisionShape()
{
	//
	btCollisionShape* CollisionShape = nullptr;

	switch( Header_.ShapeType_ )
	{
	case ScnPhysicsMeshShapeType::BVH:
		{
			// Wait for optimised bvh to complete building.
			BuildingBvhFence_.wait();
			BcAssertMsg( OptimizedBvh_ != nullptr, "Optimized bvh has not been created." );
			auto MeshShape = new btBvhTriangleMeshShape( MeshInterface_, true, false );
			MeshShape->setOptimizedBvh( OptimizedBvh_ );
			CollisionShape = MeshShape;
		}
		break;

	case ScnPhysicsMeshShapeType::GIMPACT:
		{
			auto MeshShape = new btGImpactMeshShape( MeshInterface_ );
			MeshShape->updateBound();
			CollisionShape = MeshShape;
		}
		break;

	case ScnPhysicsMeshShapeType::CONVEX_DECOMPOSITION:
		{
			auto CompoundShape = new btCompoundShape();
			auto& IndexedMeshArray = MeshInterface_->getIndexedMeshArray();
			for( BcU32 Idx = 0; Idx < IndexedMeshArray.size(); ++Idx )
			{
				const auto& IndexedMesh = IndexedMeshArray[ Idx ];
				BcAssert( IndexedMesh.m_numTriangles >= 3 );
				BcAssert( IndexedMesh.m_numVertices == ( IndexedMesh.m_numTriangles * 3 ) );
				const btScalar* Points = reinterpret_cast< const btScalar * >( IndexedMesh.m_vertexBase );
				auto ConvexHullShape = new btConvexHullShape( 
					Points, IndexedMesh.m_numVertices, sizeof( ScnPhysicsVertex ) );

				btTransform Trans;
				Trans.setIdentity();
				CompoundShape->addChildShape( Trans, ConvexHullShape );	
			}
			CompoundShape->recalculateLocalAabb();
			CollisionShape = CompoundShape;
		}
		break;
	}

	BcAssert( CollisionShape != nullptr );
	return CollisionShape;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnPhysicsMesh::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0, &Header_ );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnPhysicsMesh::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{
		requestChunk( 1 );
		requestChunk( 2 );
		requestChunk( 3 );
	}
	else if( ChunkID == BcHash( "meshparts" ) )
	{
		MeshParts_ = reinterpret_cast< const ScnPhysicsMeshPart* >( pData );
	}
	else if( ChunkID == BcHash( "triangles" ) )
	{
		Triangles_ = reinterpret_cast< const ScnPhysicsTriangle* >( pData );
	}
	else if( ChunkID == BcHash( "vertices" ) )
	{
		Vertices_ = reinterpret_cast< const ScnPhysicsVertex* >( pData );
		markCreate();
	}
}
