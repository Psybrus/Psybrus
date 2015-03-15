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
	};
	
	ReRegisterClass< ScnPhysicsMeshImport, Super >( Fields );

	{
		ReEnumConstant* EnumConstants[] = 
		{
			new ReEnumConstant( "BVH", (BcU32)ScnPhysicsMeshShapeType::BVH ),
			new ReEnumConstant( "GIMPACT", (BcU32)ScnPhysicsMeshShapeType::GIMPACT ),
			new ReEnumConstant( "STATIC", (BcU32)ScnPhysicsMeshShapeType::STATIC ),
			new ReEnumConstant( "DYNAMIC", (BcU32)ScnPhysicsMeshShapeType::DYNAMIC ),

		};
		ReRegisterEnum< ScnPhysicsMeshShapeType >( EnumConstants );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnPhysicsMeshImport::ScnPhysicsMeshImport():
	ShapeType_( ScnPhysicsMeshShapeType::DYNAMIC )
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
			0, 
			Scene_->mNumMaterials, 
			MaAABB() 
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
				OutTriangle.Indices_[ 0 ] = InFace.mIndices[ 0 ] + Header.NoofVertices_;
				OutTriangle.Indices_[ 1 ] = InFace.mIndices[ 1 ] + Header.NoofVertices_;
				OutTriangle.Indices_[ 2 ] = InFace.mIndices[ 2 ] + Header.NoofVertices_;
				OutTriangle.Material_ = Mesh->mMaterialIndex;

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

				VertexStream_ << OutVertex;
			}

			// Advance header stuff for each mesh.
			Header.NoofTriangles_ += Mesh->mNumFaces;
			Header.NoofVertices_ += Mesh->mNumVertices;
		}
		HeaderStream_ << Header;

		BcAssert( HeaderStream_.dataSize() > 0 );
		BcAssert( TriangleStream_.dataSize() > 0 );
		BcAssert( VertexStream_.dataSize() > 0 );
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize(), 16, csPCF_IN_PLACE );
		CsResourceImporter::addChunk( BcHash( "triangles" ), TriangleStream_.pData(), TriangleStream_.dataSize(), 16 );
		CsResourceImporter::addChunk( BcHash( "vertices" ), VertexStream_.pData(), VertexStream_.dataSize(), 16 );
		return BcTrue;
	}
#endif // PSY_IMPORT_PIPELINE
	return BcFalse;

}
