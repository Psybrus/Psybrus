/**************************************************************************
*
* File:		ScnModelImport.cpp
* Author:	Neil Richardson 
* Ver/Date: 06/01/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnModelImport.h"
#include "ScnMaterialImport.h"
#include "ScnTextureImport.h"
#include "Reflection/ReReflection.h"

#include "Base/BcMath.h"

#include <memory>
#include <regex>

#if PSY_IMPORT_PIPELINE

#include "assimp/config.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

// TODO: Remove dup from ScnPhysicsMeshImport.
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

	/**
	 * Fill all elements less than zero with specific value.
	 */
	void FillAllElementsLessThanZero( BcF32 Value, BcF32* pElements, BcU32 NoofElements )
	{
		for( BcU32 Idx = 0; Idx < NoofElements; ++Idx )
		{
			if( pElements[ Idx ] < 0.0f )
			{
				pElements[ Idx ] = Value;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
static BcU32 gVertexDataSize[] = 
{
	4,					// RsVertexDataType::FLOAT32 = 0,
	2,					// RsVertexDataType::FLOAT16,
	4,					// RsVertexDataType::FIXED,
	1,					// RsVertexDataType::BYTE,
	1,					// RsVertexDataType::BYTE_NORM,
	1,					// RsVertexDataType::UBYTE,
	1,					// RsVertexDataType::UBYTE_NORM,
	2,					// RsVertexDataType::SHORT,
	2,					// RsVertexDataType::SHORT_NORM,
	2,					// RsVertexDataType::USHORT,
	2,					// RsVertexDataType::USHORT_NORM,
	4,					// RsVertexDataType::INT,
	4,					// RsVertexDataType::INT_NORM,
	4,					// RsVertexDataType::UINT,
	4					// RsVertexDataType::UINT_NORM,
};

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// ScnModelMaterialDesc
ScnModelMaterialDesc::ScnModelMaterialDesc( ScnModelMaterialDesc&& Other )
{
	using std::swap;
	swap( Regex_, Other.Regex_ );
	swap( TemplateMaterial_, Other.TemplateMaterial_ );
	swap( Material_, Other.Material_ );
}

ScnModelMaterialDesc::~ScnModelMaterialDesc()
{ 
	delete TemplateMaterial_;
	TemplateMaterial_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_BASIC( ScnModelVertexFormat );

void ScnModelVertexFormat::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Position_", &ScnModelVertexFormat::Position_, bcRFF_IMPORTER ),
		new ReField( "Normal_", &ScnModelVertexFormat::Normal_, bcRFF_IMPORTER ),
		new ReField( "Binormal_", &ScnModelVertexFormat::Binormal_, bcRFF_IMPORTER ),
		new ReField( "Tangent_", &ScnModelVertexFormat::Tangent_, bcRFF_IMPORTER ),
		new ReField( "Colour_", &ScnModelVertexFormat::Colour_, bcRFF_IMPORTER ),
		new ReField( "BlendIndices_", &ScnModelVertexFormat::BlendIndices_, bcRFF_IMPORTER ),
		new ReField( "BlendWeights_", &ScnModelVertexFormat::BlendWeights_, bcRFF_IMPORTER ),
		new ReField( "TexCoord0_", &ScnModelVertexFormat::TexCoord0_, bcRFF_IMPORTER ),
		new ReField( "TexCoord1_", &ScnModelVertexFormat::TexCoord1_, bcRFF_IMPORTER ),
		new ReField( "TexCoord2_", &ScnModelVertexFormat::TexCoord2_, bcRFF_IMPORTER ),
		new ReField( "TexCoord3_", &ScnModelVertexFormat::TexCoord3_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnModelVertexFormat >( Fields );
}


REFLECTION_DEFINE_BASIC( ScnModelMaterialDesc );

void ScnModelMaterialDesc::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Regex_", &ScnModelMaterialDesc::Regex_, bcRFF_IMPORTER ),
		new ReField( "TemplateMaterial_", &ScnModelMaterialDesc::TemplateMaterial_, bcRFF_IMPORTER ),
		new ReField( "Material_", &ScnModelMaterialDesc::Material_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnModelMaterialDesc >( Fields );
}


REFLECTION_DEFINE_DERIVED( ScnModelImport )

void ScnModelImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnModelImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Materials_", &ScnModelImport::Materials_, bcRFF_IMPORTER ),
		new ReField( "FlattenHierarchy_", &ScnModelImport::FlattenHierarchy_, bcRFF_IMPORTER ),
		new ReField( "VertexFormat_", &ScnModelImport::VertexFormat_, bcRFF_IMPORTER ),
		new ReField( "DefaultTextures_", &ScnModelImport::DefaultTextures_, bcRFF_IMPORTER ),
	};
	
	ReRegisterClass< ScnModelImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelImport::ScnModelImport():
	HeaderStream_(),
	NodeTransformDataStream_( BcFalse, 65536, 65536 ),
	NodePropertyDataStream_( BcFalse, 65536, 65536 ),
	VertexDataStream_( BcFalse, 1024 * 1024, 1024 * 1024 ),
	IndexDataStream_( BcFalse, 1024 * 1024, 1024 * 1024 ),
	VertexElementStream_( BcFalse, 65536, 65536 ),
	MeshDataStream_( BcFalse, 65536, 65536 )
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelImport::ScnModelImport( ReNoInit )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnModelImport::~ScnModelImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnModelImport::import()
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

	// Resolve source path.
	auto ResolvedSource = CsPaths::resolveContent( Source_.c_str() );

	CsResourceImporter::addDependency( ResolvedSource.c_str() );

	auto PropertyStore = aiCreatePropertyStore();
	aiSetImportPropertyInteger( PropertyStore, AI_CONFIG_PP_SBBC_MAX_BONES, ScnShaderBoneUniformBlockData::MAX_BONES );
	aiSetImportPropertyInteger( PropertyStore, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4 );
	aiSetImportPropertyInteger( PropertyStore, AI_CONFIG_IMPORT_MD5_NO_ANIM_AUTOLOAD, true );

	aiLogStream AssimpLogger =
	{
		AssimpLogStream, (char*)this
	};
	aiAttachLogStream( &AssimpLogger );

	// TODO: Intercept file io to track dependencies.
	int Flags = aiProcessPreset_TargetRealtime_MaxQuality | 
			aiProcess_SplitByBoneCount |
			aiProcess_LimitBoneWeights |
			aiProcess_ConvertToLeftHanded;
	if( FlattenHierarchy_ )
	{
		Flags |= aiProcess_OptimizeGraph;
	}

	Scene_ = aiImportFileExWithProperties( 
		ResolvedSource.c_str(), 
		Flags,
		nullptr, 
		PropertyStore );

	aiReleasePropertyStore( PropertyStore );

	if( Scene_ != nullptr )
	{
		PSY_LOG( "Found %u materials:\n", Scene_->mNumMaterials );
		for( int Idx = 0; Idx < (int)Scene_->mNumMaterials; ++Idx )
		{
			PSY_LOG( " - %s\n", AssimpGetMaterialName( Scene_->mMaterials[ Idx ] ).c_str() );
		}

		size_t NodeIndex = 0;
		size_t PrimitiveIndex = 0;
		
		recursiveSerialiseNodes( Scene_->mRootNode, 
								 (size_t)-1, 
								 NodeIndex, 
								 PrimitiveIndex );

		aiReleaseImport( Scene_ );
		Scene_ = nullptr;

		// Setup header.
		ScnModelHeader Header = 
		{
			static_cast< BcU32 >( NodeIndex ),
			static_cast< BcU32 >( PrimitiveIndex )
		};
		
		HeaderStream_ << Header;
		
		// Calculate world transforms.
		calculateNodeWorldTransforms();

		// Serialise node data.
		for( const auto& NodeTransformData : NodeTransformData_ )
		{
			NodeTransformDataStream_ << NodeTransformData;
		}

		for( auto NodePropertyData : NodePropertyData_ )
		{
			// Add name to the importer.
			NodePropertyData.Name_ = CsResourceImporter::addString( (*NodePropertyData.Name_).c_str() );

			//
			NodePropertyDataStream_ << NodePropertyData;
		}

		// Serialise vertex elements.
		for( const auto& VertexDecl : VertexDeclarations_ )
		{
			for( const auto& VertexElement : VertexDecl.Elements_ )
			{
				VertexElementStream_ << VertexElement;
			}
		}

		// Serialise mesh data.
		for( const auto& MeshData : MeshData_ )
		{
			MeshDataStream_ << MeshData;
		}

		// Write to file.
		CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize() );
		CsResourceImporter::addChunk( BcHash( "nodetransformdata" ), NodeTransformDataStream_.pData(), NodeTransformDataStream_.dataSize() );
		CsResourceImporter::addChunk( BcHash( "nodepropertydata" ), NodePropertyDataStream_.pData(), NodePropertyDataStream_.dataSize() );
		CsResourceImporter::addChunk( BcHash( "vertexdata" ), VertexDataStream_.pData(), VertexDataStream_.dataSize() );
		CsResourceImporter::addChunk( BcHash( "indexdata" ), IndexDataStream_.pData(), IndexDataStream_.dataSize() );
		CsResourceImporter::addChunk( BcHash( "vertexelements" ), VertexElementStream_.pData(), VertexElementStream_.dataSize() );
		CsResourceImporter::addChunk( BcHash( "meshdata" ), MeshDataStream_.pData(), MeshDataStream_.dataSize() );
		
		//
		return BcTrue;
	}
	else
	{

	}
	aiDetachLogStream( &AssimpLogger );

#endif // PSY_IMPORT_PIPELINE
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// calculateNodeWorldTransforms
void ScnModelImport::calculateNodeWorldTransforms()
{
#if PSY_IMPORT_PIPELINE
	BcAssert( NodeTransformData_.size() == NodePropertyData_.size() );

	for( BcU32 Idx = 0; Idx < NodeTransformData_.size(); ++Idx )
	{
		auto& NodeTransformData( NodeTransformData_[ Idx ] );
		const auto& NodePropertyData( NodePropertyData_[ Idx ] );

		// If we've got a parent, we need to use it's world transform.
		if( NodePropertyData.ParentIndex_ != BcErrorCode )
		{
			BcAssert( NodePropertyData.ParentIndex_ < Idx );
			const auto& ParentNodeTransformData( NodeTransformData_[ NodePropertyData.ParentIndex_ ] );

			NodeTransformData.WorldTransform_ = 
				NodeTransformData.LocalTransform_ *
				ParentNodeTransformData.WorldTransform_;
		}
		else
		{
			NodeTransformData.WorldTransform_ = 
				NodeTransformData.LocalTransform_;
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// recursiveSerialiseNodes
void ScnModelImport::recursiveSerialiseNodes( 
	struct aiNode* Node,
	size_t ParentIndex,
	size_t& NodeIndex,
	size_t& PrimitiveIndex )
{
#if PSY_IMPORT_PIPELINE
	// Setup structs.
	ScnModelNodeTransformData NodeTransformData =
	{
		MaMat4d( Node->mTransformation[0] ).transposed(),
		MaMat4d(),	// todo: absolute
	};
	
	ScnModelNodePropertyData NodePropertyData = 
	{
		static_cast< BcU32 >( ParentIndex ),
		BcName::StripInvalidChars( Node->mName.C_Str() ).c_str(),
		BcFalse, // todo: is bone
	};
	
	// Put in lists.
	NodeTransformData_.push_back( NodeTransformData );
	NodePropertyData_.push_back( NodePropertyData );
	
	// Update parent & node index.
	ParentIndex = NodeIndex++;

	// Setup primitive data.
	if( Node->mNumMeshes > 0 )
	{
		for( size_t Idx = 0; Idx < Node->mNumMeshes; ++Idx )
		{
			serialiseMesh( 
				Scene_->mMeshes[ Node->mMeshes[ Idx ] ],
				ParentIndex, 
				NodeIndex,
				PrimitiveIndex );
		}
	}
		
	// Recurse into children.
	for( size_t Idx = 0; Idx < Node->mNumChildren; ++Idx )
	{
		recursiveSerialiseNodes( 
			Node->mChildren[ Idx ],
			ParentIndex,
			NodeIndex,
			PrimitiveIndex );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// serialiseMesh
void ScnModelImport::serialiseMesh( 
	struct aiMesh* Mesh,
	size_t ParentIndex,
	size_t& NodeIndex,
	size_t& PrimitiveIndex )
{
#if PSY_IMPORT_PIPELINE
	if( Mesh->HasPositions() && Mesh->HasFaces() )
	{
		ScnShaderPermutationFlags ShaderPermutation = ScnShaderPermutationFlags::MESH_STATIC_3D;

		// Calculate number of primitives.
		BcAssert( BcBitsSet( Mesh->mPrimitiveTypes ) == 1 );

		// Vertex format.
		RsVertexDeclarationDesc VertexDeclarationDesc = RsVertexDeclarationDesc();
		VertexDeclarationDesc.addElement( RsVertexElement( 
			0, VertexDeclarationDesc.getMinimumStride(), 
			4, VertexFormat_.Position_, RsVertexUsage::POSITION, 0 ) );

		if( Mesh->HasNormals() )
		{
			VertexDeclarationDesc.addElement( RsVertexElement(
				0, VertexDeclarationDesc.getMinimumStride(), 
				4, VertexFormat_.Normal_, RsVertexUsage::NORMAL, 0 ) );
		}

		if( Mesh->HasTangentsAndBitangents() )
		{
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(), 
				4, VertexFormat_.Tangent_, RsVertexUsage::TANGENT, 0 ) );
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(), 
				4, VertexFormat_.Binormal_, RsVertexUsage::BINORMAL, 0 ) );
		}

		if( Mesh->HasTextureCoords( 0 ) )
		{
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(), 
				2, VertexFormat_.TexCoord0_, RsVertexUsage::TEXCOORD, 0 ) );
		}

		// Always export channel 0 to save on shader permutations.
		// Should optimise out later.
		//if( Mesh->HasVertexColors( 0 ) )
		{
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(), 4, VertexFormat_.Colour_, RsVertexUsage::COLOUR, 0 ) );
		}
		
		// Add bones to vertex declaration if they exist.
		if( Mesh->HasBones() )
		{
			ShaderPermutation = ScnShaderPermutationFlags::MESH_SKINNED_3D;

			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(),
				4, VertexFormat_.BlendIndices_, RsVertexUsage::BLENDINDICES, 0 ) );
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(),
				4, VertexFormat_.BlendWeights_, RsVertexUsage::BLENDWEIGHTS, 0 ) );
		}

		VertexDeclarations_.push_back( VertexDeclarationDesc );


		// NOTE: This next section needs to be picky to be optimal. Optimise later :)
		ScnModelMeshData MeshData = 
		{
			static_cast< BcU32 >( ParentIndex ),
			BcFalse,
			RsTopologyType::TRIANGLE_LIST,	
			ShaderPermutation,
			0,
			BcErrorCode,
			0, // padding0
			0, // padding1
			MaAABB(),
			Mesh->mNumVertices,
			static_cast< BcU32 >( VertexDeclarationDesc.Elements_.size() ),
			static_cast< BcU32 >( VertexDeclarationDesc.getMinimumStride() ),
			0
		};

		// Grab primitive type.
		switch( Mesh->mPrimitiveTypes )
		{
		case aiPrimitiveType_POINT:
			MeshData.NoofIndices_ = Mesh->mNumFaces;
			MeshData.Type_ = RsTopologyType::POINTS;
			break;
		case aiPrimitiveType_LINE:
			MeshData.NoofIndices_ = Mesh->mNumFaces * 2;
			MeshData.Type_ = RsTopologyType::LINE_LIST;
			break;
		case aiPrimitiveType_TRIANGLE:
			MeshData.NoofIndices_ = Mesh->mNumFaces * 3;
			MeshData.Type_ = RsTopologyType::TRIANGLE_LIST;
			break;
		default:
			BcBreakpoint;
		}
		
		MeshData.NoofVertexElements_ = static_cast< BcU32 >( VertexDeclarationDesc.Elements_.size() );
		MeshData.IsSkinned_ = Mesh->HasBones();

		// Calculate stride.
		MeshData.VertexStride_ = VertexDeclarationDesc.getMinimumStride();

		if( Mesh->HasBones() )
		{
			// Setup bone palette + bind poses for primitive.
			BcMemSet( MeshData.BonePalette_, 0xff, sizeof( MeshData.BonePalette_ ) );
			BcAssert( Mesh->mNumBones <= SCN_MODEL_BONE_PALETTE_SIZE );
			for( BcU32 BoneIdx = 0; BoneIdx < Mesh->mNumBones; ++BoneIdx )
			{
				const auto* Bone = Mesh->mBones[ BoneIdx ];
				size_t NodeBaseIndex = 0;
				MeshData.BonePalette_[ BoneIdx ] = static_cast< BcU32 >( findNodeIndex( Bone->mName.C_Str(), Scene_->mRootNode, NodeBaseIndex ) );
				MeshData.BoneInverseBindpose_[ BoneIdx ] = MaMat4d( Bone->mOffsetMatrix[ 0 ] ).transposed();
			}
		}	

		// Export vertices.
		serialiseVertices( Mesh, 
			&VertexDeclarationDesc.Elements_[ 0 ], 
			VertexDeclarationDesc.Elements_.size(), 
			MeshData.AABB_ );
		
		// Grab material name.
		std::string MaterialName;
		aiMaterial* Material = Scene_->mMaterials[ Mesh->mMaterialIndex ];
		
		// Import material.
		MeshData.MaterialRef_ = findMaterialMatch( Material );	
		MeshData_.push_back( MeshData );

		// Export indices.
		BcU32 TotalIndices = 0;
		for( BcU32 FaceIdx = 0; FaceIdx < Mesh->mNumFaces; ++FaceIdx )
		{
			const auto& Face = Mesh->mFaces[ FaceIdx ];
			for( BcU32 IndexIdx = 0; IndexIdx < Face.mNumIndices; ++ IndexIdx )
			{
				BcU32 Index = Face.mIndices[ IndexIdx ];
				BcAssert( Index < 0x10000 );
				IndexDataStream_ << BcU16( Index );
				++TotalIndices;
			}
		}

		// Verify indices.
		BcAssert( TotalIndices == MeshData.NoofIndices_ );
		
		// Update primitive index.
		++PrimitiveIndex;
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// serialiseVertices
void ScnModelImport::serialiseVertices( 
	struct aiMesh* Mesh,
	RsVertexElement* pVertexElements,
	size_t NoofVertexElements,
	MaAABB& AABB )
{
#if PSY_IMPORT_PIPELINE
	AABB.empty();

	// Build blend weights and indices.
	std::vector< MaVec4d > BlendWeights;
	std::vector< MaVec4d > BlendIndices;

	if( Mesh->HasBones() )
	{
		// Clear off to less than zero to signify empty.
		BlendWeights.resize( Mesh->mNumVertices, MaVec4d( -1.0f, -1.0f, -1.0f, -1.0f ) );
		BlendIndices.resize( Mesh->mNumVertices, MaVec4d( -1.0f, -1.0f, -1.0f, -1.0f ) );

		// Populate the weights and indices.
		for( BcU32 BoneIdx = 0; BoneIdx < Mesh->mNumBones; ++BoneIdx )
		{
			auto* Bone = Mesh->mBones[ BoneIdx ];

			for( BcU32 WeightIdx = 0; WeightIdx < Bone->mNumWeights; ++WeightIdx )
			{
				const auto& WeightVertex = Bone->mWeights[ WeightIdx ];
				
				MaVec4d& BlendWeight = BlendWeights[ WeightVertex.mVertexId ];
				MaVec4d& BlendIndex = BlendIndices[ WeightVertex.mVertexId ];

				BcU32 BlendWeightElementIdx = FillNextElementLessThanZero( 
					WeightVertex.mWeight, reinterpret_cast< BcF32* >( &BlendWeight ), 4 );
				BcU32 BlendIndexElementIdx = FillNextElementLessThanZero( 
					static_cast< BcF32 >( BoneIdx ), reinterpret_cast< BcF32* >( &BlendIndex ), 4 );
				BcAssert( BlendWeightElementIdx == BlendIndexElementIdx );
				BcUnusedVar( BlendWeightElementIdx );
				BcUnusedVar( BlendIndexElementIdx );
			}
		}

		// Fill the rest of the weights and indices with valid, but empty values.
		for( BcU32 VertIdx = 0; VertIdx < Mesh->mNumVertices; ++VertIdx )
		{
			MaVec4d& BlendWeight = BlendWeights[ VertIdx ];
			MaVec4d& BlendIndex = BlendIndices[ VertIdx ];

			FillAllElementsLessThanZero( 0.0f, reinterpret_cast< BcF32* >( &BlendWeight ), 4 );
			FillAllElementsLessThanZero( 0.0f, reinterpret_cast< BcF32* >( &BlendIndex ), 4 );
		}
	}

	// Calculate output vertex size.
	// TODO: Stride.
	BcU32 Stride = 0;
	for( BcU32 ElementIdx = 0; ElementIdx < NoofVertexElements; ++ElementIdx )
	{
		const auto VertexElement( pVertexElements[ ElementIdx ] );
		BcU32 Size = VertexElement.Components_ * gVertexDataSize[(BcU32)VertexElement.DataType_];
		Stride = std::max( Stride, VertexElement.Offset_ + Size );
	}

	std::vector< BcU8 > VertexData( Stride, 0 );

	for( BcU32 VertexIdx = 0; VertexIdx < Mesh->mNumVertices; ++VertexIdx )
	{
		aiVector3D Position = 
			Mesh->mVertices != nullptr ? Mesh->mVertices[ VertexIdx ] : 
			aiVector3D( 0.0f, 0.0f, 0.0f );

		// Expand AABB.
		AABB.expandBy( MaVec3d( Position.x, Position.y, Position.z ) );

		for( BcU32 ElementIdx = 0; ElementIdx < NoofVertexElements; ++ElementIdx )
		{
			const auto VertexElement( pVertexElements[ ElementIdx ] );

			switch( VertexElement.Usage_ )
			{
			case RsVertexUsage::POSITION:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcF32 Input[] = { Position.x, Position.y, Position.z, 1.0f };
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, 4, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;
			case RsVertexUsage::NORMAL:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcF32 Input[] = { 0.0f, 0.0f, 0.0f, 0.0f };
					if( Mesh->mNormals != nullptr )
					{
						Input[ 0 ] = Mesh->mNormals[ VertexIdx ].x;
						Input[ 1 ] = Mesh->mNormals[ VertexIdx ].y;
						Input[ 2 ] = Mesh->mNormals[ VertexIdx ].z;
					}
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, 4, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;
			case RsVertexUsage::TANGENT:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcF32 Input[] = { 0.0f, 0.0f, 0.0f, 0.0f };
					if( Mesh->mTangents != nullptr )
					{
						Input[ 0 ] = Mesh->mTangents[ VertexIdx ].x;
						Input[ 1 ] = Mesh->mTangents[ VertexIdx ].y;
						Input[ 2 ] = Mesh->mTangents[ VertexIdx ].z;
					}
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, 4, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;
			case RsVertexUsage::BINORMAL:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcF32 Input[] = { 0.0f, 0.0f, 0.0f, 0.0f };
					if( Mesh->mBitangents != nullptr )
					{
						Input[ 0 ] = Mesh->mBitangents[ VertexIdx ].x;
						Input[ 1 ] = Mesh->mBitangents[ VertexIdx ].y;
						Input[ 2 ] = Mesh->mBitangents[ VertexIdx ].z;
					}
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, 4, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;
			case RsVertexUsage::TEXCOORD:
				{
					BcAssert( VertexElement.UsageIdx_ < AI_MAX_NUMBER_OF_TEXTURECOORDS );
					BcAssert( VertexElement.Components_ == Mesh->mNumUVComponents[ VertexElement.UsageIdx_ ] );
					BcF32 Input[] = { 0.0f, 0.0f, 0.0f, 0.0f };
					if( Mesh->mTextureCoords[ VertexElement.UsageIdx_ ] != nullptr )
					{
						Input[ 0 ] = Mesh->mTextureCoords[ VertexElement.UsageIdx_ ][ VertexIdx ].x;
						Input[ 1 ] = Mesh->mTextureCoords[ VertexElement.UsageIdx_ ][ VertexIdx ].y;
						Input[ 2 ] = Mesh->mTextureCoords[ VertexElement.UsageIdx_ ][ VertexIdx ].z;
					}
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, VertexElement.Components_, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;
			case RsVertexUsage::COLOUR:
				{
					BcAssert( VertexElement.UsageIdx_ < AI_MAX_NUMBER_OF_COLOR_SETS );
					BcAssert( VertexElement.Components_ == 4 );
					BcF32 Input[] = { 1.0f, 1.0f, 1.0f, 1.0f };
					if( Mesh->mColors[ VertexElement.UsageIdx_ ] != nullptr )
					{
						Input[ 0 ] = Mesh->mColors[ VertexElement.UsageIdx_ ][ VertexIdx ].r;
						Input[ 1 ] = Mesh->mColors[ VertexElement.UsageIdx_ ][ VertexIdx ].g;
						Input[ 2 ] = Mesh->mColors[ VertexElement.UsageIdx_ ][ VertexIdx ].b;
						Input[ 3 ] = Mesh->mColors[ VertexElement.UsageIdx_ ][ VertexIdx ].a;
					}
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, VertexElement.Components_, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;
			case RsVertexUsage::BLENDINDICES:
				{
					BcAssert( VertexElement.Components_ == 4 );

					BcF32 Input[] = { 
						BlendIndices[ VertexIdx ].x(),
						BlendIndices[ VertexIdx ].y(),
						BlendIndices[ VertexIdx ].z(),
						BlendIndices[ VertexIdx ].w()
					};
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, 4, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;
			case RsVertexUsage::BLENDWEIGHTS:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcF32 Input[] = { 
						BlendWeights[ VertexIdx ].x(),
						BlendWeights[ VertexIdx ].y(),
						BlendWeights[ VertexIdx ].z(),
						BlendWeights[ VertexIdx ].w()
					};
					const BcF32 TotalWeight = Input[0] + Input[1] + Input[2] + Input[3];
					const BcF32 Epsilon = 0.5f;
					// TODO: Make error.
					BcAssertMsg( TotalWeight > Epsilon, 
						"Total weight too low to safely renormalise: %f\n", TotalWeight );
					Input[0] /= TotalWeight;
					Input[1] /= TotalWeight;
					Input[2] /= TotalWeight;
					Input[3] /= TotalWeight;
					BcU32 OutSize = 0;
					RsFloatToVertexDataType( 
						Input, 4, VertexElement.DataType_, 
						&VertexData[ VertexElement.Offset_ ], OutSize );
				}
				break;

			default:
				break;
			};
		}

		VertexDataStream_.push( VertexData.data(), VertexData.size() );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// findNodeIndex
size_t ScnModelImport::findNodeIndex( 
	std::string Name, 
	aiNode* RootSearchNode, 
	size_t& BaseIndex ) const
{
#if PSY_IMPORT_PIPELINE
	if( Name == RootSearchNode->mName.C_Str() )
	{
		return BaseIndex;
	}

	size_t FoundIndex = (size_t)-1;
	for( size_t Idx = 0; Idx < RootSearchNode->mNumChildren; ++Idx )
	{
		auto ChildSearchNode = RootSearchNode->mChildren[ Idx ];
		++BaseIndex;
		FoundIndex = findNodeIndex( Name, ChildSearchNode, BaseIndex );

		if( FoundIndex != -1 )
		{
			return FoundIndex;
		}
	}
	
	return FoundIndex;
#else
	return (size_t)-1;
#endif
}

//////////////////////////////////////////////////////////////////////////
// findMaterialMatch
CsCrossRefId ScnModelImport::findMaterialMatch( aiMaterial* Material )
{
	CsCrossRefId RetVal = CSCROSSREFID_INVALID;
#if PSY_IMPORT_PIPELINE

	// Grab material name.
	auto MaterialName = AssimpGetMaterialName( Material );

	// Setup material refs if there are matches.
	for( auto MaterialEntryIt = Materials_.rbegin(); MaterialEntryIt != Materials_.rend(); ++MaterialEntryIt )
	{
		auto& MaterialEntry = *MaterialEntryIt;

		if( std::regex_match( MaterialName, std::regex( MaterialEntry.Regex_ ) ) )
		{
			auto FoundMaterial = AddedMaterials_.find( MaterialName );
			if( FoundMaterial == AddedMaterials_.end() )
			{
				if( MaterialEntry.Material_ == CSCROSSREFID_INVALID )
				{
					// Create new material importer based on template.
					ScnMaterialImport* MaterialImport(
						ReConstructObject< ScnMaterialImport >( 
							this->getResourceName() + "/" + MaterialName, this, MaterialEntry.TemplateMaterial_ ) );

					// Attempt to find textures.
					auto DiffuseRef = addTexture( Material, MaterialImport, "aDiffuseTex", aiTextureType_DIFFUSE, 0 );
					auto SpecularRef = addTexture( Material, MaterialImport, "aSpecularTex", aiTextureType_SPECULAR, 0 );
					auto MetallicRef = addTexture( Material, MaterialImport, "aMetallicTex", aiTextureType_AMBIENT, 0 );
					auto EmissiveRef = addTexture( Material, MaterialImport, "aEmissiveTex", aiTextureType_EMISSIVE, 0 );
					auto HeightRef = addTexture( Material, MaterialImport, "aHeightTex", aiTextureType_HEIGHT, 0 );
					auto NormalRef = addTexture( Material, MaterialImport, "aNormalTex", aiTextureType_NORMALS, 0 );
					auto RoughnessRef = addTexture( Material, MaterialImport, "aRoughnessTex", aiTextureType_SHININESS, 0 );
					auto OpacityRef = addTexture( Material, MaterialImport, "aOpacityTex", aiTextureType_OPACITY, 0 );
					auto DisplacementRef = addTexture( Material, MaterialImport, "aDisplacementTex", aiTextureType_DISPLACEMENT, 0 );
					auto LightmapRef = addTexture( Material, MaterialImport, "aLightmapTex", aiTextureType_LIGHTMAP, 0 );
					auto ReflectionRef = addTexture( Material, MaterialImport, "aReflectionTex", aiTextureType_REFLECTION, 0 );

					RetVal = addImport( CsResourceImporterUPtr( MaterialImport ) );
					AddedMaterials_[ MaterialName ] = RetVal;
				}
				else
				{
					RetVal = MaterialEntry.Material_;
					AddedMaterials_[ MaterialName ] = RetVal;
				}
			}
			else
			{
				RetVal = FoundMaterial->second;
			}
		}
	}

	// Can't find match? Throw exception.
	if( RetVal == CSCROSSREFID_INVALID )
	{
		auto ErrorString = std::string( "Unable to find match for \"" ) + MaterialName + std::string( "\"" );
		
		CsResourceImporter::addMessage( CsMessageCategory::ERROR, ErrorString );
	}
#endif
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// addTexture
CsCrossRefId ScnModelImport::addTexture( aiMaterial* Material, ScnMaterialImport* MaterialImport, std::string Name, BcU32 Type, BcU32 Idx )
{
	CsCrossRefId TextureRef = CSCROSSREFID_INVALID;
#if PSY_IMPORT_PIPELINE
	aiString AiName;
	aiString Path;
	aiTextureMapping TextureMapping = aiTextureMapping_UV;
	unsigned int UVIndex = 0;
	float Blend = 0.0f;
	aiTextureOp TextureOp = aiTextureOp_Multiply;
	aiTextureMapMode TextureMapMode = aiTextureMapMode_Wrap;
	if( Material->GetTexture( (aiTextureType)Type, Idx, &Path,
			&TextureMapping, &UVIndex, &Blend, &TextureOp, &TextureMapMode ) == aiReturn_SUCCESS )
	{
		BcPath TexturePath = BcPath( Source_ ).getParent();
		std::string FixedPath = Path.C_Str();
		std::replace( FixedPath.begin(), FixedPath.end(), '\\', '/');
		TexturePath.join( FixedPath.c_str() );

		auto ResolvedTexturePath = CsPaths::resolveContent( TexturePath.c_str() );
		if( BcFileSystemExists( ResolvedTexturePath.c_str() ) )
		{
			RsSamplerStateDesc SamplerState;

			switch( TextureMapMode )
			{
			case aiTextureMapMode_Wrap:
				SamplerState.AddressU_ = RsTextureSamplingMode::WRAP;
				break;
			case aiTextureMapMode_Clamp:
				SamplerState.AddressU_ = RsTextureSamplingMode::CLAMP;
				break;
			case aiTextureMapMode_Decal:
				SamplerState.AddressU_ = RsTextureSamplingMode::DECAL;
				break;
			case aiTextureMapMode_Mirror:
				SamplerState.AddressU_ = RsTextureSamplingMode::MIRROR;
				break;
			default: 
				BcBreakpoint;
			}
			SamplerState.AddressV_ = SamplerState.AddressU_;
			SamplerState.AddressW_ = SamplerState.AddressU_;

			SamplerState.MinFilter_ = RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR;
			SamplerState.MagFilter_ = RsTextureFilteringMode::LINEAR;

			// Pass up unresolved texture path, it will resolve later.
			auto TextureImporter = new ScnTextureImport(
					Path.C_Str(), "ScnTexture",
					TexturePath.c_str(), RsResourceFormat::UNKNOWN );

			// Setup some default texture formats.
			switch( Type )
			{
			case aiTextureType_NORMALS:
				// TODO: BC5
				TextureImporter->setFormat( RsResourceFormat::R8G8B8A8_UINT );
				break;
			case aiTextureType_AMBIENT: // METALLIC
				// TODO: BC4.
				TextureImporter->setFormat( RsResourceFormat::R8G8B8A8_UINT );
				break;
			case aiTextureType_SHININESS: // ROUGHNESS
				// TODO: BC4.
				TextureImporter->setFormat( RsResourceFormat::R8G8B8A8_UINT );
				break;
			default:
				break;
			}


			TextureRef = addImport( CsResourceImporterUPtr( TextureImporter ) );
			MaterialImport->addTexture( Name, TextureRef, SamplerState );
		}
		else
		{
			addMessage( CsMessageCategory::WARNING, "Unable to find texture " + *TexturePath );
		}
	}

	// Patch in default texture if we need to.
	if( TextureRef == CSCROSSREFID_INVALID )
	{
		auto It = DefaultTextures_.find( Name );
		if( It != DefaultTextures_.end() )
		{
			// TODO: Implement DefaultSamplerStates.
			RsSamplerStateDesc Sampler;
			Sampler.MinFilter_ = RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR;
			Sampler.MagFilter_ = RsTextureFilteringMode::LINEAR;

			TextureRef = It->second;
			MaterialImport->addTexture( Name, TextureRef, Sampler );
		}
	}
#endif
	return TextureRef;
}
