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

#include "Base/BcMath.h"

#include <memory>
#include <regex>

#if PSY_IMPORT_PIPELINE

#include "assimp/config.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

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
			BcPrintf( "ASSIMP: %s", Message );
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
// Reflection
REFLECTION_DEFINE_DERIVED( ScnModelImport )
	
void ScnModelImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnModelImport::Source_, bcRFF_IMPORTER ),
		new ReField( "Materials_", &ScnModelImport::Materials_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnModelImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelImport::ScnModelImport()
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
BcBool ScnModelImport::import( const Json::Value& )
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

	// Failed? Try to use assimp.
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
	Scene_ = aiImportFileExWithProperties( 
		Source_.c_str(), 
		aiProcessPreset_TargetRealtime_MaxQuality | 
			aiProcess_SplitByBoneCount |
			aiProcess_LimitBoneWeights,
		nullptr, 
		PropertyStore );

	aiReleasePropertyStore( PropertyStore );

	if( Scene_ != nullptr )
	{
		BcPrintf( "Found %u materials:\n", Scene_->mNumMaterials );
		for( int Idx = 0; Idx < Scene_->mNumMaterials; ++Idx )
		{
			BcPrintf( " - %s\n", AssimpGetMaterialName( Scene_->mMaterials[ Idx ] ).c_str() );
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
			4, RsVertexDataType::FLOAT32, RsVertexUsage::POSITION, 0 ) );

		if( Mesh->HasNormals() )
		{
			VertexDeclarationDesc.addElement( RsVertexElement(
				0, VertexDeclarationDesc.getMinimumStride(), 
				4, RsVertexDataType::FLOAT32, RsVertexUsage::NORMAL, 0 ) );
		}

		if( Mesh->HasTangentsAndBitangents() )
		{
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(), 
				4, RsVertexDataType::FLOAT32, RsVertexUsage::TANGENT, 0 ) );
		}

		if( Mesh->HasTextureCoords( 0 ) )
		{
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(), 
				2, RsVertexDataType::FLOAT32, RsVertexUsage::TEXCOORD, 0 ) );
		}

		// Always export channel 0 to save on shader permutations.
		// Should optimise out later.
		//if( Mesh->HasVertexColors( 0 ) )
		{
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(), 4, RsVertexDataType::UBYTE_NORM, RsVertexUsage::COLOUR, 0 ) );
		}
		
		// Add bones to vertex declaration if they exist.
		if( Mesh->HasBones() )
		{
			ShaderPermutation = ScnShaderPermutationFlags::MESH_SKINNED_3D;

			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(),
				4, RsVertexDataType::FLOAT32, RsVertexUsage::BLENDINDICES, 0 ) );
			VertexDeclarationDesc.addElement( RsVertexElement( 
				0, VertexDeclarationDesc.getMinimumStride(),
				4, RsVertexDataType::FLOAT32, RsVertexUsage::BLENDWEIGHTS, 0 ) );
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

		MaterialName = AssimpGetMaterialName( Material );
		
		// Import material.
		MeshData.MaterialRef_ = findMaterialMatch( MaterialName.c_str() );	
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
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = Position.x;
						*OutVal++ = Position.y;
						*OutVal++ = Position.z;
						*OutVal++ = 1.0f;
					}
				}
				break;
			case RsVertexUsage::NORMAL:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );

					aiVector3D Normal = 
						Mesh->mNormals != nullptr ? 
						Mesh->mNormals[ VertexIdx ] : 
						aiVector3D( 0.0f, 0.0f, 0.0f );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = Normal.x;
						*OutVal++ = Normal.y;
						*OutVal++ = Normal.z;
						*OutVal++ = 1.0f;
					}
				}
				break;
			case RsVertexUsage::TANGENT:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );

					aiVector3D Tangent = 
						Mesh->mTangents != nullptr ? 
						Mesh->mTangents[ VertexIdx ] : 
						aiVector3D( 0.0f, 0.0f, 0.0f );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = Tangent.x;
						*OutVal++ = Tangent.y;
						*OutVal++ = Tangent.z;
						*OutVal++ = 1.0f;
					}
				}
				break;
			case RsVertexUsage::BINORMAL:
				{
					BcAssert( VertexElement.Components_ == 3 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );

					aiVector3D Bitangent = 
						Mesh->mBitangents != nullptr ? 
						Mesh->mBitangents[ VertexIdx ] : 
						aiVector3D( 0.0f, 0.0f, 0.0f );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = Bitangent.x;
						*OutVal++ = Bitangent.y;
						*OutVal++ = Bitangent.z;
						*OutVal++ = 1.0f;
					}
				}
				break;
			case RsVertexUsage::TEXCOORD:
				{
					BcAssert( VertexElement.UsageIdx_ < AI_MAX_NUMBER_OF_TEXTURECOORDS );
					BcAssert( VertexElement.Components_ == Mesh->mNumUVComponents[ VertexElement.UsageIdx_ ] );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );

					aiVector3D Texcoord = 
						Mesh->mTextureCoords[ VertexElement.UsageIdx_ ] != nullptr ? 
						Mesh->mTextureCoords[ VertexElement.UsageIdx_ ][ VertexIdx ] : 
						aiVector3D( 0.0f, 0.0f, 0.0f );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						for( BcU32 ComponentIdx = 0; ComponentIdx < VertexElement.Components_; ++ComponentIdx )
						{
							if( ComponentIdx == 1 )
							{
								*OutVal++ = 1.0f - Texcoord[ ComponentIdx ];
							}
							else
							{
								*OutVal++ = Texcoord[ ComponentIdx ];
							}
						}
					}
				}
				break;
			case RsVertexUsage::COLOUR:
				{
					BcAssert( VertexElement.UsageIdx_ < AI_MAX_NUMBER_OF_COLOR_SETS );
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::UBYTE_NORM );
					aiColor4D Colour = aiColor4D( 1.0f, 1.0f, 1.0f, 1.0f );
				
					if( Mesh->HasVertexColors( VertexElement.UsageIdx_ ) )
					{
						Colour = Mesh->mColors[ VertexElement.UsageIdx_ ][ VertexIdx ];
					}

					{
						BcU32* OutVal = reinterpret_cast< BcU32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = RsColour( Colour.r, Colour.g, Colour.b, Colour.a ).asABGR();
					}
				}
				break;
			case RsVertexUsage::BLENDINDICES:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						MaVec4d BlendIndicesVec = BlendIndices[ VertexIdx ];

						*OutVal++ = (BcF32)BlendIndicesVec.x();
						*OutVal++ = (BcF32)BlendIndicesVec.y();
						*OutVal++ = (BcF32)BlendIndicesVec.z();
						*OutVal++ = (BcF32)BlendIndicesVec.w();
					}
				}
				break;
			case RsVertexUsage::BLENDWEIGHTS:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						MaVec4d BlendWeightsVec = BlendWeights[ VertexIdx ];
						const BcF32 TotalWeight = 
							BlendWeightsVec.x() + BlendWeightsVec.y() +
							BlendWeightsVec.z() + BlendWeightsVec.w();
						const BcF32 Epsilon = 0.5f;
						// TODO: Make error.
						BcAssertMsg( TotalWeight > Epsilon, 
							"Total weight too low to safely renormalise: %f\n", TotalWeight );

						BlendWeightsVec /= TotalWeight;
						const BcF32 TotalWeightRecalc = 
							BlendWeightsVec.x() + BlendWeightsVec.y() +
							BlendWeightsVec.z() + BlendWeightsVec.w();

						*OutVal++ = (BcF32)BlendWeightsVec.x();
						*OutVal++ = (BcF32)BlendWeightsVec.y();
						*OutVal++ = (BcF32)BlendWeightsVec.z();
						*OutVal++ = (BcF32)BlendWeightsVec.w();
					}
				}
				break;

			default:
				break;
			};
		}

		VertexDataStream_.push( &VertexData[0], VertexData.size() );
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
CsCrossRefId ScnModelImport::findMaterialMatch( const std::string& MaterialName )
{
	CsCrossRefId RetVal = CSCROSSREFID_INVALID;

	for( const auto& MaterialEntry : Materials_ )
	{
		if( std::regex_match( MaterialName, std::regex( MaterialEntry.first ) ) )
		{
			RetVal = MaterialEntry.second;
		}
	}

	// Can't find match? Throw exception.
	if( RetVal == CSCROSSREFID_INVALID )
	{
		auto ErrorString = std::string( "Unable to find match for \"" ) + MaterialName + std::string( "\"" );
		
		CsResourceImporter::addMessage( CsMessageCategory::ERROR, ErrorString );
	}

	return RetVal;
}