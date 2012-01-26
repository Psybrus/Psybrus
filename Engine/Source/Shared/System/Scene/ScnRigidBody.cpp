/**************************************************************************
*
* File:		ScnRigidBody.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnRigidBody.h"
#include "ScnEntity.h"

#include "ScnRigidBodyWorld.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#include "Mdl.h"
#endif

// Bullet includes.
#include "btBulletDynamicsCommon.h"

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnRigidBody::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& Shape = Object[ "shape" ];
	const Json::Value& Body = Object[ "body" ];
	const std::string& ShapeType = Shape[ "type" ].asString();
	THeader Header;
	Header.Extents_.x( (BcReal)Shape[ "extentx" ].asDouble() );
	Header.Extents_.y( (BcReal)Shape[ "extenty" ].asDouble() );
	Header.Extents_.z( (BcReal)Shape[ "extentz" ].asDouble() );
	Header.NoofVertices_ = 0;
	Header.NoofIndices_ = 0;

	BcStream HeaderStream;
	BcStream VertexStream;
	BcStream IndexStream;

	// Load in body.
	if( ShapeType == "mesh" )
	{
		Header.Type_ = ST_MESH;

		// Load and pack mesh data.
		const std::string& FileName = Shape[ "source" ].asString();
		MdlNode* pNode = MdlLoader::loadModel( FileName.c_str() );

		if( pNode == NULL )
		{
			return BcFalse;
		}

		DependancyList.push_back( CsDependancy( FileName ) );

		// Grab mesh.
		MdlMesh* pMesh = pNode->pMeshObject();

		if( pMesh != NULL )
		{
			Header.NoofVertices_ = pMesh->nVertices();
			for( BcU32 Idx = 0; Idx < pMesh->nVertices(); ++Idx )
			{
				VertexStream << (BcF32)pMesh->vertex( Idx ).Position_.x();
				VertexStream << (BcF32)pMesh->vertex( Idx ).Position_.y();
				VertexStream << (BcF32)pMesh->vertex( Idx ).Position_.z();
			}

			Header.NoofIndices_ = pMesh->nIndices();
			for( BcU32 Idx = 0; Idx < pMesh->nIndices(); ++Idx )
			{
				IndexStream << (int)pMesh->index( Idx ).iVertex_;
			}

			delete pNode;
		}	
		else
		{
			delete pNode;
			return BcFalse;
		}
	}
	else if ( ShapeType == "sphere" )
	{
		Header.Type_ = ST_SPHERE;
	}
	else if ( ShapeType == "box" )
	{
		Header.Type_ = ST_BOX;
	}
	else if ( ShapeType == "capsule" )
	{
		Header.Type_ = ST_CAPSULE;
	}
	else
	{
		return BcFalse;
	}
	
	// Body.
	Header.Mass_ = (BcReal)Body[ "mass" ].asDouble();
	Header.E_ = (BcReal)Body[ "e" ].asDouble();
	Header.F_ = (BcReal)Body[ "f" ].asDouble();

	// Write header.
	HeaderStream << Header;
	
	//
	pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
	if( Header.Type_ == ST_MESH )
	{
		pFile_->addChunk( BcHash( "vertices" ), VertexStream.pData(), VertexStream.dataSize() );
		pFile_->addChunk( BcHash( "indices" ), IndexStream.pData(), IndexStream.dataSize() );
	}

	//
	return BcTrue;
}

#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnRigidBody );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnRigidBody::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnRigidBody" )
		.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRigidBody::initialise()
{
	// NULL internals.
	pHeader_ = NULL;
	pVertexBufferData_ = NULL;
	pIndexBufferData_ = NULL;
	pTriangleIndexVertexArray_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRigidBody::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnRigidBody::destroy()
{
	delete pTriangleIndexVertexArray_;
	pTriangleIndexVertexArray_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnRigidBody::isReady()
{
	return pHeader_ != NULL && ( pHeader_->Type_ == ST_MESH ? ( pVertexBufferData_ != NULL && pIndexBufferData_ != NULL ) : BcTrue );
}

//////////////////////////////////////////////////////////////////////////
// setup
void ScnRigidBody::setup()
{
	if( pHeader_->Type_ == ST_MESH )
	{
		int NumTriangles = pHeader_->NoofIndices_ / 3;
		int IndexStride = 3 * sizeof( int );
		int VertexStride = 3 * sizeof( BcF32 );
		pTriangleIndexVertexArray_ = new btTriangleIndexVertexArray( NumTriangles, pIndexBufferData_, IndexStride, pHeader_->NoofVertices_, pVertexBufferData_, VertexStride );
	}
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnRigidBody::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnRigidBody::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;

		if( pHeader_->Type_ == ST_MESH )
		{
			getChunk( ++ChunkIdx );
		}
		else
		{
			setup();
		}
	}
	else if( pChunk->ID_ == BcHash( "vertices" ) )
	{
		pVertexBufferData_ = (BcF32*)pData;
		getChunk( ++ChunkIdx );
	}
	else if( pChunk->ID_ == BcHash( "indices" ) )
	{
		pIndexBufferData_ = (int*)pData;
		
		setup();
	}
}


//////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( ScnRigidBodyComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnRigidBodyComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	PropertyTable.beginCatagory( "ScnRigidBodyComponent" )
		//.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnRigidBodyComponent::initialise( ScnRigidBodyRef Parent )
{
	pCollisionShape_ = NULL;
	pRigidBody_ = NULL;

	// Cache parent.
	Parent_ = Parent;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnRigidBodyComponent::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnRigidBodyComponent::destroy()
{
	delete pCollisionShape_;
	delete pRigidBody_;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnRigidBodyComponent::isReady()
{
	return Parent_->isReady();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnRigidBodyComponent::update( BcReal Tick )
{
	Super::update( Tick );

	// Set transform from rigid body.
	const btTransform& PhysicsTransform = pRigidBody_->getCenterOfMassTransform();
	const btQuaternion& Rotation = PhysicsTransform.getRotation();
	const btVector3& Origin = PhysicsTransform.getOrigin();
	
	getParentEntity()->setRotation( BcQuat( Rotation.x(), Rotation.y(), Rotation.z(), Rotation.w() ) );
	getParentEntity()->setPosition( BcVec3d( Origin.x(), Origin.y(), Origin.z() ) );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnRigidBodyComponent::onAttach( ScnEntityWeakRef Parent )
{
	//
	Super::onAttach( Parent );

	// Create collision shape.
	ScnRigidBody::THeader* pHeader = Parent_->pHeader_;
	BcVec3d InternalExtents = pHeader->Extents_;
	switch( pHeader->Type_ )
	{
	case ScnRigidBody::ST_MESH:
		{
			pCollisionShape_ = new btBvhTriangleMeshShape( Parent_->pTriangleIndexVertexArray_, false, true );

			// Moving not supported for meshes!
			pHeader->Mass_ = 0.0f;
		}
		break;

	case ScnRigidBody::ST_SPHERE:
		{
			btScalar Radius = InternalExtents.x();
			pCollisionShape_ = new btSphereShape( Radius );
		}
		break;

	case ScnRigidBody::ST_BOX:
		{
			btVector3 Extent = btVector3( InternalExtents.x(), InternalExtents.y(), InternalExtents.z() );
			pCollisionShape_ = new btBoxShape( Extent );
		}
		break;

	case ScnRigidBody::ST_CAPSULE:
		{
			pCollisionShape_ = new btCapsuleShape( InternalExtents.x(), InternalExtents.y() );
		}
		break;

	default:
		{
		}
		break;
	}

	// Calculate local inertia.
	BcBool IsDynamic = (pHeader->Mass_ != 0.f);
	btVector3 LocalInertia(0,0,0);
	if (IsDynamic)
	{
		pCollisionShape_->calculateLocalInertia( pHeader->Mass_, LocalInertia );
	}

	// Create rigid body.
	const ScnTransform& Transform = getParentEntity()->getTransform();
	const BcQuat& Rotation = Transform.getRotation();
	const BcVec3d& Translation = Transform.getTranslation();
	btTransform StartTransform( btQuaternion( Rotation.x(), Rotation.y(), Rotation.z(), Rotation.w() ),
								btVector3( Translation.x(), Translation.y(), Translation.z() ) );
	btRigidBody::btRigidBodyConstructionInfo ConstructionInfo(
			pHeader->Mass_,
			NULL,
			pCollisionShape_,
			LocalInertia );
	ConstructionInfo.m_startWorldTransform = StartTransform;
	ConstructionInfo.m_friction = pHeader->F_;
	ConstructionInfo.m_restitution = pHeader->E_;
	pRigidBody_ = new btRigidBody( ConstructionInfo );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnRigidBodyComponent::onDetach( ScnEntityWeakRef Parent )
{
	//
	Super::onDetach( Parent );
	
	//
	delete pCollisionShape_;
	delete pRigidBody_;
	pCollisionShape_ = NULL;
	pRigidBody_ = NULL;
}
