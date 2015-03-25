/**************************************************************************
*
* File:		ScnPhysicsMeshImport.cpp
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Physics/ScnPhysicsMeshImport.h"

#if PSY_IMPORT_PIPELINE

#include "assimp/config.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

#include "hacdCircularList.h"
#include "hacdVector.h"
#include "hacdICHull.h"
#include "hacdGraph.h"
#include "hacdHACD.h"

// TODO: Remove dup from ScnModelImport.
namespace
{
	/**
	 * Assimp logging function.
	 */
	void AssimpLogStream( const char* Message, char* User )
	{
		if( BcStrStr( Message, "Error" ) != nullptr ||
			BcStrStr( Message, "Warning" ) != nullptr ) 
		{
			PSY_LOG( "ASSIMP: %s", Message );
		}
	}

	/**
	 * Determine material name.
	 */
	std::string AssimpGetMaterialName( aiMaterial* Material )
	{
		aiString AiName( "default" );
		// Try material name.
		if( Material->Get( AI_MATKEY_NAME, AiName ) == aiReturn_SUCCESS )
		{
		}
		// Try diffuse texture.
		else if( Material->Get( AI_MATKEY_TEXTURE( aiTextureType_DIFFUSE, 0 ), AiName ) == aiReturn_SUCCESS )
		{
		}
		return AiName.C_Str();
	}

	/**
	 * Fill next element that is less than zero.
	 * Will check elements until first one less than 0.0 is found and overwrite it.
	 */
	BcU32 FillNextElementLessThanZero( BcF32 Value, BcF32* pElements, BcU32 NoofElements )
	{
		for( BcU32 Idx = 0; Idx < NoofElements; ++Idx )
		{
			if( pElements[ Idx ] < 0.0f )
			{
				pElements[ Idx ] = Value;
				return Idx;
			}
		}

		return BcErrorCode;
	}
}

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnPhysicsMeshImport )
	
void ScnPhysicsMeshImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "ShapeType_", &ScnPhysicsMeshImport::ShapeType_, bcRFF_IMPORTER ),
		new ReField( "Source_", &ScnPhysicsMeshImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Materials_", &ScnPhysicsMeshImport::Materials_, bcRFF_IMPORTER ),
		new ReField( "MinClusters_", &ScnPhysicsMeshImport::MinClusters_, bcRFF_IMPORTER ),
		new ReField( "MaxVerticesPerCluster_", &ScnPhysicsMeshImport::MaxVerticesPerCluster_, bcRFF_IMPORTER ),
		new ReField( "Concavity_", &ScnPhysicsMeshImport::Concavity_, bcRFF_IMPORTER ),
		new ReField( "AddExtraDistPoints_", &ScnPhysicsMeshImport::AddExtraDistPoints_, bcRFF_IMPORTER ),
		new ReField( "AddNeighbourDistPoints_", &ScnPhysicsMeshImport::AddNeighbourDistPoints_, bcRFF_IMPORTER ),
		new ReField( "AddFacePoints_", &ScnPhysicsMeshImport::AddFacePoints_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnPhysicsMeshImport, Super >( Fields );

	{
		ReEnumConstant* EnumConstants[] = 
		{
			new ReEnumConstant( "BVH", (BcU32)ScnPhysicsMeshShapeType::BVH ),
			new ReEnumConstant( "GIMPACT", (BcU32)ScnPhysicsMeshShapeType::GIMPACT ),
			new ReEnumConstant( "CONVEX_DECOMPOSITION", (BcU32)ScnPhysicsMeshShapeType::CONVEX_DECOMPOSITION ),
			new ReEnumConstant( "STATIC", (BcU32)ScnPhysicsMeshShapeType::STATIC ),
			new ReEnumConstant( "DYNAMIC", (BcU32)ScnPhysicsMeshShapeType::DYNAMIC ),

		};
		ReRegisterEnum< ScnPhysicsMeshShapeType >( EnumConstants );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsMeshImport::ScnPhysicsMeshImport():
	ShapeType_( ScnPhysicsMeshShapeType::DYNAMIC ),
	Source_(),
	Materials_(),
	MinClusters_( 2 ),
	MaxVerticesPerCluster_( 64 ),
	Concavity_( 100.0f ),
	AddExtraDistPoints_( BcFalse ),
	AddNeighbourDistPoints_( BcFalse ),
	AddFacePoints_( BcFalse )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
//virtual
ScnPhysicsMeshImport::~ScnPhysicsMeshImport()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcBool ScnPhysicsMeshImport::import( const Json::Value& )
{
#if PSY_IMPORT_PIPELINE
	BcBool CanImport = BcTrue;

	if( Source_.empty() )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "Missing 'source' field." );
		CanImport = BcFalse;
	}

	if( Materials_.empty() )
	{
		CsResourceImporter::addMessage( CsMessageCategory::CRITICAL, "Missing 'materials' list." );
		CanImport = BcFalse;
	}

	// If we hit any problems, bail out.
	if( CanImport == BcFalse )
	{
		return BcFalse;
	}

	CsResourceImporter::addDependency( Source_.c_str() );

	auto PropertyStore = aiCreatePropertyStore();
	aiSetImportPropertyInteger( PropertyStore, AI_CONFIG_PP_RVC_FLAGS, 
		aiComponent_TANGENTS_AND_BITANGENTS | 
		aiComponent_NORMALS | 
		aiComponent_COLORS |
		aiComponent_TEXCOORDS |
		aiComponent_BONEWEIGHTS |
		aiComponent_ANIMATIONS |
		aiComponent_TEXTURES |
		aiComponent_LIGHTS |
		aiComponent_CAMERAS );

	aiLogStream AssimpLogger =
	{
		AssimpLogStream, (char*)this
	};
	aiAttachLogStream( &AssimpLogger );

	// TODO: Intercept file io to track dependencies.
	Scene_ = aiImportFileExWithProperties( 
		Source_.c_str(), 
			aiProcess_RemoveComponent |
			aiProcess_Triangulate | 
			aiProcess_FindDegenerates |
			aiProcess_FindInvalidData |
			aiProcess_JoinIdenticalVertices |
			aiProcess_ImproveCacheLocality |
			aiProcess_LimitBoneWeights |
			aiProcess_PreTransformVertices,
		nullptr, 
		PropertyStore );

	aiReleasePropertyStore( PropertyStore );

	if( Scene_ != nullptr )
	{
		if( Scene_->mRootNode == nullptr )
		{
			CsResourceImporter::addMessage( CsMessageCategory::ERROR, "No root node in model." );
			return BcFalse;
		}

		if( Scene_->mRootNode->mNumChildren > 0 )
		{
			CsResourceImporter::addMessage( CsMessageCategory::ERROR, "Model is not completely flattened." );
			return BcFalse;
		}

		if( Scene_->mRootNode->mNumMeshes == 0 )
		{
			CsResourceImporter::addMessage( CsMessageCategory::ERROR, "No meshes in model." );
			return BcFalse;
		}

		ScnPhysicsMeshHeader Header = 
		{
			ShapeType_,
			0, 
			Scene_->mNumMaterials, 
			MaAABB() 
		};

		std::vector< HACD::Vec3< HACD::Real > > HACDPoints;
		std::vector< HACD::Vec3< long > > HACDTriangles;


		ScnPhysicsMeshPart MeshPart
		{
			0, 0
		};

		auto RootNode = Scene_->mRootNode;
		for( size_t MeshIdx = 0; MeshIdx < RootNode->mNumMeshes; ++MeshIdx )
		{
			auto Mesh = Scene_->mMeshes[ RootNode->mMeshes[ MeshIdx ] ];

			ScnPhysicsTriangle OutTriangle;
			ScnPhysicsVertex OutVertex;

			// Import faces.
			for( size_t FaceIdx = 0; FaceIdx < Mesh->mNumFaces; ++FaceIdx )
			{
				const auto& InFace = Mesh->mFaces[ FaceIdx ];
				BcAssert( InFace.mNumIndices == 3 );
				OutTriangle.Indices_[ 0 ] = InFace.mIndices[ 0 ] + MeshPart.NoofVertices_;
				OutTriangle.Indices_[ 1 ] = InFace.mIndices[ 1 ] + MeshPart.NoofVertices_;
				OutTriangle.Indices_[ 2 ] = InFace.mIndices[ 2 ] + MeshPart.NoofVertices_;
				OutTriangle.Material_ = Mesh->mMaterialIndex;

				HACDTriangles.push_back( 
					HACD::Vec3< long >( 
						OutTriangle.Indices_[ 0 ],
						OutTriangle.Indices_[ 1 ],
						OutTriangle.Indices_[ 2 ] ) );

				TriangleStream_ << OutTriangle;
			}

			// Import vertices.
			for( size_t VertexIdx = 0; VertexIdx < Mesh->mNumVertices; ++VertexIdx )
			{
				const auto& InVertex = Mesh->mVertices[ VertexIdx ];
				OutVertex.Position_.x( InVertex.x );
				OutVertex.Position_.y( InVertex.y );
				OutVertex.Position_.z( InVertex.z );
				OutVertex.Position_.w( 0.0f );

				Header.AABB_.expandBy( OutVertex.Position_.xyz() );

				HACDPoints.push_back( 
					HACD::Vec3< HACD::Real >( 
						OutVertex.Position_.x(),
						OutVertex.Position_.y(),
						OutVertex.Position_.z() ) );

				VertexStream_ << OutVertex;
			}

			// Advance header stuff for each mesh.
			MeshPart.NoofTriangles_ += Mesh->mNumFaces;
			MeshPart.NoofVertices_ += Mesh->mNumVertices;
		}

		// Not convex decomposition, so only 1 mesh part.
		if( ShapeType_ != ScnPhysicsMeshShapeType::CONVEX_DECOMPOSITION )
		{
			MeshPartStream_ << MeshPart;

			Header.NoofMeshParts_ = 1;
		}
		else
		{
			HACD::HACD MyHACD;
			MyHACD.SetPoints( &HACDPoints[ 0 ] );
			MyHACD.SetNPoints( HACDPoints.size() );
			MyHACD.SetTriangles( &HACDTriangles[ 0 ] );
			MyHACD.SetNTriangles( HACDTriangles.size() );
			MyHACD.SetCompacityWeight( 0.1 );
			MyHACD.SetVolumeWeight( 0.0 );
			MyHACD.SetNClusters( MinClusters_ );
			MyHACD.SetNVerticesPerCH( MaxVerticesPerCluster_ );
			MyHACD.SetConcavity( Concavity_ );
			MyHACD.SetAddExtraDistPoints( AddExtraDistPoints_);
			MyHACD.SetAddNeighboursDistPoints( AddNeighbourDistPoints_ );
			MyHACD.SetAddFacesPoints( AddFacePoints_ );
	 
			PSY_LOG( "Computing convex hull with HACD..." );
			MyHACD.Compute();
			auto NoofClusters = MyHACD.GetNClusters();
			PSY_LOG( "...Done! %u clusters.", NoofClusters );

			ScnPhysicsTriangle OutTriangle;
			ScnPhysicsVertex OutVertex;

			VertexStream_.clear();
			TriangleStream_.clear();

			for (int c=0;c<NoofClusters;c++)
			{
				//generate convex result
				size_t nPoints = MyHACD.GetNPointsCH(c);
				size_t nTriangles = MyHACD.GetNTrianglesCH(c);

				PSY_LOG( "Cluster %u: %u points, %u tris", c, nPoints, nTriangles );

				HACD::Vec3< HACD::Real > * PointsCH = new HACD::Vec3< HACD::Real >[ nPoints ];
				HACD::Vec3< long > * TrianglesCH = new HACD::Vec3< long >[ nTriangles ];
				MyHACD.GetCH( c, PointsCH, TrianglesCH );

				// Write out sections as flat vertex arrays with sequential indices., still pack indexed
				// data for debugging the indexed meshes too.
				// points
				// triangles
				BcU32 NoofTriangles = 0;
				BcU32 NoofVertices = 0;
				for( BcU32 TriIdx = 0; TriIdx < nTriangles; ++TriIdx )
				{
					OutTriangle.Indices_[ 0 ] = ( ( TriIdx * 3 ) + 0 );
					OutTriangle.Indices_[ 1 ] = ( ( TriIdx * 3 ) + 1 );
					OutTriangle.Indices_[ 2 ] = ( ( TriIdx * 3 ) + 2 );
					OutTriangle.Material_ = 0;

					TriangleStream_ << OutTriangle;

					++NoofTriangles;

					auto AddVertex = [ & ]( HACD::Vec3< HACD::Real > Position )
					{
						OutVertex.Position_.x( Position.X() );
						OutVertex.Position_.y( Position.Y() );
						OutVertex.Position_.z( Position.Z() );
						OutVertex.Position_.w( 0.0f );
						Header.AABB_.expandBy( OutVertex.Position_.xyz() );
						VertexStream_ << OutVertex;
						++NoofVertices;
					};

					AddVertex( PointsCH[ TrianglesCH[ TriIdx ].X() ] );
					AddVertex( PointsCH[ TrianglesCH[ TriIdx ].Y() ] );
					AddVertex( PointsCH[ TrianglesCH[ TriIdx ].Z() ] );
				}

				delete [] PointsCH;
				delete [] TrianglesCH;

				//ConvexResult r(nPoints, vertices, nTriangles, triangles);
				//convexDecomposition.ConvexDecompResult(r);

				//
				MeshPart.NoofTriangles_ = NoofTriangles;
				MeshPart.NoofVertices_ = NoofVertices;
				MeshPartStream_ << MeshPart;

				Header.NoofMeshParts_ += 1;
			}

		}

		HeaderStream_ << Header;
		BcAssert( HeaderStream_.dataSize() > 0 );
		BcAssert( TriangleStream_.dataSize() > 0 );
		BcAssert( VertexStream_.dataSize() > 0 );
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize(), 16, csPCF_IN_PLACE );
		CsResourceImporter::addChunk( BcHash( "meshparts" ), MeshPartStream_.pData(), MeshPartStream_.dataSize(), 16 );
		CsResourceImporter::addChunk( BcHash( "triangles" ), TriangleStream_.pData(), TriangleStream_.dataSize(), 16 );
		CsResourceImporter::addChunk( BcHash( "vertices" ), VertexStream_.pData(), VertexStream_.dataSize(), 16 );

		delete Scene_;		
		return BcTrue;
	}
#endif // PSY_IMPORT_PIPELINE
	return BcFalse;

}
