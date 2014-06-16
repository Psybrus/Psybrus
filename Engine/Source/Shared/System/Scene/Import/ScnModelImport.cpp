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

#include "Import/Mdl/Mdl.h"

#include "assimp/config.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/mesh.h"
#include "assimp/postprocess.h"

#define ENABLE_ASSIMP_IMPORTER			( 0 )

#ifdef PSY_SERVER

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

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelImport::ScnModelImport()
{
}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnModelImport::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	pImporter_ = &Importer;
	Source_ = Object[ "source" ].asString();
	pImporter_->addDependency( Source_.c_str() );

#if ENABLE_ASSIMP_IMPORTER
	auto PropertyStore = aiCreatePropertyStore();
	aiSetImportPropertyInteger( PropertyStore, AI_CONFIG_PP_SBBC_MAX_BONES, ScnShaderBoneUniformBlockData::MAX_BONES );
	aiSetImportPropertyInteger( PropertyStore, AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4 );
	aiSetImportPropertyInteger( PropertyStore, AI_CONFIG_IMPORT_MD5_NO_ANIM_AUTOLOAD, true );

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
		BcU32 NodeIndex = 0;
		BcU32 PrimitiveIndex = 0;
		
		recursiveSerialiseNodes( Scene_->mRootNode, 
								 BcErrorCode, 
								 NodeIndex, 
								 PrimitiveIndex );

		aiReleaseImport( Scene_ );
		Scene_ = nullptr;

		// Setup header.
		ScnModelHeader Header = 
		{
			NodeIndex,
			PrimitiveIndex
		};
		
		HeaderStream_ << Header;
		
				// Calculate world transforms.
		calculateNodeWorldTransforms();

		// Serialise node data.
		for( const auto& NodeTransformData : NodeTransformData_ )
		{
			//
			NodeTransformDataStream_ << NodeTransformData;
		}

		for( auto NodePropertyData : NodePropertyData_ )
		{
			// Add name to the importer.
			NodePropertyData.Name_ = pImporter_->addString( (*NodePropertyData.Name_).c_str() );

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
		Importer.addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize() );
		Importer.addChunk( BcHash( "nodetransformdata" ), NodeTransformDataStream_.pData(), NodeTransformDataStream_.dataSize() );
		Importer.addChunk( BcHash( "nodepropertydata" ), NodePropertyDataStream_.pData(), NodePropertyDataStream_.dataSize() );
		Importer.addChunk( BcHash( "vertexdata" ), VertexDataStream_.pData(), VertexDataStream_.dataSize() );
		Importer.addChunk( BcHash( "indexdata" ), IndexDataStream_.pData(), IndexDataStream_.dataSize() );
		Importer.addChunk( BcHash( "vertexelements" ), VertexElementStream_.pData(), VertexElementStream_.dataSize() );
		Importer.addChunk( BcHash( "meshdata" ), MeshDataStream_.pData(), MeshDataStream_.dataSize() );
		
		//
		return BcTrue;
	}
#endif // ENABLE_ASSIMP_IMPORTER

	// Fall back to old method.
	MdlNode* pNode = MdlLoader::loadModel( Source_.c_str() );
	
	if( pNode != NULL )
	{
		BcU32 NodeIndex = 0;
		BcU32 PrimitiveIndex = 0;
		
		recursiveSerialiseNodes( pNode, 
								 BcErrorCode, 
								 NodeIndex, 
								 PrimitiveIndex );

		// Delete root node.
		delete pNode;
		pNode = NULL;
		
		// Setup header.
		ScnModelHeader Header = 
		{
			NodeIndex,
			PrimitiveIndex
		};
		
		HeaderStream_ << Header;

		// Calculate world transforms.
		calculateNodeWorldTransforms();

		// Serialise node data.
		for( const auto& NodeTransformData : NodeTransformData_ )
		{
			//
			NodeTransformDataStream_ << NodeTransformData;
		}

		for( auto NodePropertyData : NodePropertyData_ )
		{
			// Add name to the importer.
			NodePropertyData.Name_ = pImporter_->addString( (*NodePropertyData.Name_).c_str() );

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
		Importer.addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize() );
		Importer.addChunk( BcHash( "nodetransformdata" ), NodeTransformDataStream_.pData(), NodeTransformDataStream_.dataSize() );
		Importer.addChunk( BcHash( "nodepropertydata" ), NodePropertyDataStream_.pData(), NodePropertyDataStream_.dataSize() );
		Importer.addChunk( BcHash( "vertexdata" ), VertexDataStream_.pData(), VertexDataStream_.dataSize() );
		Importer.addChunk( BcHash( "indexdata" ), IndexDataStream_.pData(), IndexDataStream_.dataSize() );
		Importer.addChunk( BcHash( "vertexelements" ), VertexElementStream_.pData(), VertexElementStream_.dataSize() );
		Importer.addChunk( BcHash( "meshdata" ), MeshDataStream_.pData(), MeshDataStream_.dataSize() );
		
		//
		return BcTrue;
	}
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// calculateNodeWorldTransforms
void ScnModelImport::calculateNodeWorldTransforms()
{
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
}

//////////////////////////////////////////////////////////////////////////
// recursiveSerialiseNodes
void ScnModelImport::recursiveSerialiseNodes( MdlNode* pNode,
									          BcU32 ParentIndex,
									          BcU32& NodeIndex,
									          BcU32& PrimitiveIndex )
{
	// Setup structs.
	ScnModelNodeTransformData NodeTransformData =
	{
		pNode->relativeTransform(),
		MaMat4d(),	// calculated later.
		pNode->inverseBindpose()
	};
	
	ScnModelNodePropertyData NodePropertyData = 
	{
		ParentIndex,
		BcName::StripInvalidChars( pNode->name() ).c_str(),
		pNode->type() == eNT_JOINT
	};

	// Put in lists.
	NodeTransformData_.push_back( NodeTransformData );
	NodePropertyData_.push_back( NodePropertyData );
		
	// Update parent & node index.
	ParentIndex = NodeIndex++;

	// Setup primitive data.
	if( pNode->pMeshObject() != NULL || pNode->pSkinObject() != NULL )
	{
		MdlMesh* pMesh = pNode->pMeshObject() ? pNode->pMeshObject() : pNode->pSkinObject();
		BcBool IsSkin = pNode->pSkinObject() != NULL;
		
		// Split up mesh by material.
		std::vector< MdlMesh >& SubMeshes = pMesh->splitByMaterial();

		// Export a primitive for each submesh.
		for( BcU32 SubMeshIdx = 0; SubMeshIdx < SubMeshes.size(); ++SubMeshIdx )
		{
			if( IsSkin )
			{
				serialiseSkin( &SubMeshes[ SubMeshIdx ], ParentIndex, NodeIndex, PrimitiveIndex );
			}
			else
			{
				serialiseMesh( &SubMeshes[ SubMeshIdx ], ParentIndex, NodeIndex, PrimitiveIndex );
			}
		}
	}
		
	// Recurse into children.
	MdlNode* pChild = pNode->pChild();
	
	while( pChild != NULL )
	{
		recursiveSerialiseNodes( pChild,
								 ParentIndex,
								 NodeIndex,
								 PrimitiveIndex );
		
		pChild = pChild->pNext();
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseMesh
void ScnModelImport::serialiseMesh( class MdlMesh* pMesh,
                                    BcU32 ParentIndex,
                                    BcU32& NodeIndex,
									BcU32& PrimitiveIndex )
{
	if( pMesh->nVertices() > 0 )
	{
		ScnShaderPermutationFlags ShaderPermutation = ScnShaderPermutationFlags::MESH_STATIC_3D;

		// NOTE: This next section needs to be picky to be optimal. Optimise later :)
		ScnModelMeshData MeshData = 
		{
			ParentIndex,
			BcFalse,
			RsTopologyType::TRIANGLE_LIST,	
			ShaderPermutation,
			pMesh->nIndices(),
			BcErrorCode,
			0, // padding0
			0, // padding1
			MaAABB(),
			pMesh->nVertices(),
			5,
			48,
			nullptr
		};

		// Vertex format.
		RsVertexDeclarationDesc VertexDeclarationDesc = RsVertexDeclarationDesc( 5 )
			.addElement( RsVertexElement( 0, 0,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
			.addElement( RsVertexElement( 0, 12,		3,		RsVertexDataType::FLOAT32,		RsVertexUsage::NORMAL,			0 ) )
			.addElement( RsVertexElement( 0, 24,		3,		RsVertexDataType::FLOAT32,		RsVertexUsage::TANGENT,			0 ) )
			.addElement( RsVertexElement( 0, 36,		2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 ) )
			.addElement( RsVertexElement( 0, 44,		4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 ) );
		VertexDeclarations_.push_back( VertexDeclarationDesc );

		// Export vertices.
		serialiseVertices( 
			pMesh, 
			&VertexDeclarationDesc.Elements_[ 0 ], 
			VertexDeclarationDesc.Elements_.size(), 
			MeshData.AABB_ );
		
		// Grab material name.
		MdlMaterial Material = pMesh->material( 0 );
			
		// Always setup default material.
		if( Material.Name_.length() == 0 )
		{
			Material.Name_ = "$(ScnMaterial:materials.default)";
		}
		else
		{
			// Add the cross package reference.
			Material.Name_ = std::string("$(ScnMaterial:") + Material.Name_ + std::string(")");
		}

		// Import material.
		// TODO: Pass through parameters from the model into import?
		MeshData.MaterialRef_ = pImporter_->addPackageCrossRef( Material.Name_.c_str() );

		MeshData_.push_back( MeshData );

		// Export indices.
		MdlIndex Index;
		for( BcU32 IndexIdx = 0; IndexIdx < pMesh->nIndices(); ++IndexIdx )
		{
			Index = pMesh->index( IndexIdx );
			BcAssert( Index.iVertex_ < 0x10000 );
			IndexDataStream_ << BcU16( Index.iVertex_ );
		}
		
		// Update primitive index.
		++PrimitiveIndex;
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseSkin
void ScnModelImport::serialiseSkin( class MdlMesh* pSkin,
                                    BcU32 ParentIndex,
                                    BcU32& NodeIndex,
									BcU32& PrimitiveIndex )
{
	BcU32 BonePaletteSize = SCN_MODEL_BONE_PALETTE_SIZE;
	if( pSkin->findBoneCount() > BonePaletteSize )
	{
		std::vector< MdlMesh >& SubMeshes = pSkin->splitIntoBonePalettes( BonePaletteSize );
		
		// Export a primitive for each submesh.
		for( BcU32 SubMeshIdx = 0; SubMeshIdx < SubMeshes.size(); ++SubMeshIdx )
		{
			serialiseSkin( &SubMeshes[ SubMeshIdx ], ParentIndex, NodeIndex, PrimitiveIndex );
		}
	}
	else if( pSkin->nVertices() > 0 )
	{
		ScnShaderPermutationFlags ShaderPermutation = ScnShaderPermutationFlags::MESH_SKINNED_3D;

		// NOTE: This next section needs to be picky to be optimal. Optimise later :)
		ScnModelMeshData MeshData = 
		{
			ParentIndex,
			BcTrue,
			RsTopologyType::TRIANGLE_LIST,	
			ShaderPermutation,
			pSkin->nIndices(),
			BcErrorCode,
			0, // padding0
			0, // padding1
			MaAABB(),
			pSkin->nVertices(),
			7,
			80,
			nullptr
		};

		// Vertex format.
		RsVertexDeclarationDesc VertexDeclarationDesc = RsVertexDeclarationDesc( 7 )
			.addElement( RsVertexElement( 0, 0,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
			.addElement( RsVertexElement( 0, 12,		3,		RsVertexDataType::FLOAT32,		RsVertexUsage::NORMAL,			0 ) )
			.addElement( RsVertexElement( 0, 24,		3,		RsVertexDataType::FLOAT32,		RsVertexUsage::TANGENT,			0 ) )
			.addElement( RsVertexElement( 0, 36,		2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 ) )
			.addElement( RsVertexElement( 0, 44,		4,		RsVertexDataType::FLOAT32,		RsVertexUsage::BLENDINDICES,	0 ) )
			.addElement( RsVertexElement( 0, 60,		4,		RsVertexDataType::FLOAT32,		RsVertexUsage::BLENDWEIGHTS,	0 ) )
			.addElement( RsVertexElement( 0, 76,		4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 ) );
		VertexDeclarations_.push_back( VertexDeclarationDesc );

		// Export vertices.
		serialiseVertices( pSkin, 
			&VertexDeclarationDesc.Elements_[ 0 ], 
			VertexDeclarationDesc.Elements_.size(), 
			MeshData.AABB_ );

		// Setup bone palette for primitive.
		const MdlBonePalette& BonePalette( pSkin->bonePalette() );
		BcMemSet( MeshData.BonePalette_, 0xff, sizeof( MeshData.BonePalette_ ) );
		BcAssert( BonePalette.BonePalette_.size() <= SCN_MODEL_BONE_PALETTE_SIZE );
		for( BcU32 BoneIdx = 0; BoneIdx < BonePalette.BonePalette_.size(); ++BoneIdx )
		{
			if( BoneIdx < BonePalette.BonePalette_.size() )
			{
				MeshData.BonePalette_[ BoneIdx ] = BonePalette.BonePalette_[ BoneIdx ] + NodeIndex;
			}
		}

		// Grab material name.
		MdlMaterial Material = pSkin->material( 0 );
			
		// Always setup default material.
		if( Material.Name_.length() == 0 )
		{
			Material.Name_ = "$(ScnMaterial:materials.default)";
		}
		else
		{
			// Add the cross package reference.
			Material.Name_ = std::string("$(ScnMaterial:") + Material.Name_ + std::string(")");
		}

		// Import material.
		// TODO: Pass through parameters from the model into import?
		MeshData.MaterialRef_ = pImporter_->addPackageCrossRef( Material.Name_.c_str() );

		MeshData_.push_back( MeshData );

		// Export indices.
		MdlIndex Index;
		for( BcU32 IndexIdx = 0; IndexIdx < pSkin->nIndices(); ++IndexIdx )
		{
			Index = pSkin->index( IndexIdx );
			BcAssert( Index.iVertex_ < 0x10000 );
			IndexDataStream_ << BcU16( Index.iVertex_ );
		}
		
		// Update primitive index.
		++PrimitiveIndex;
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseVertices
void ScnModelImport::serialiseVertices( class MdlMesh* pMesh,
                                        RsVertexElement* pVertexElements,
										BcU32 NoofVertexElements,
										MaAABB& AABB )
{
	AABB.empty();

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

	for( BcU32 VertexIdx = 0; VertexIdx < pMesh->nVertices(); ++VertexIdx )
	{
		const MdlVertex& Vertex = pMesh->vertex( VertexIdx );

		// Expand AABB.
		AABB.expandBy( Vertex.Position_ );

		for( BcU32 ElementIdx = 0; ElementIdx < NoofVertexElements; ++ElementIdx )
		{
			const auto VertexElement( pVertexElements[ ElementIdx ] );

			switch( VertexElement.Usage_ )
			{
			case RsVertexUsage::POSITION:
				BcAssert( VertexElement.Components_ == 3 );
				BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
				{
					BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
					*OutVal++ = Vertex.Position_.x();
					*OutVal++ = Vertex.Position_.y();
					*OutVal++ = Vertex.Position_.z();
				}
				break;
			case RsVertexUsage::NORMAL:
				BcAssert( VertexElement.Components_ == 3 );
				BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
				{
					BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
					*OutVal++ = Vertex.Normal_.x();
					*OutVal++ = Vertex.Normal_.y();
					*OutVal++ = Vertex.Normal_.z();
				}
				break;
			case RsVertexUsage::TANGENT:
				BcAssert( VertexElement.Components_ == 3 );
				BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
				{
					BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
					*OutVal++ = Vertex.Tangent_.x();
					*OutVal++ = Vertex.Tangent_.y();
					*OutVal++ = Vertex.Tangent_.z();
				}
				break;
			case RsVertexUsage::TEXCOORD:
				BcAssert( VertexElement.Components_ == 2 );
				BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
				{
					BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
					*OutVal++ = Vertex.UV_.x();
					*OutVal++ = Vertex.UV_.y();
				}
				break;
			case RsVertexUsage::BLENDINDICES:
				BcAssert( VertexElement.Components_ == 4 );
				BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
				{
					BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
					*OutVal++ = (BcF32)Vertex.iJoints_[0];
					*OutVal++ = (BcF32)Vertex.iJoints_[1];
					*OutVal++ = (BcF32)Vertex.iJoints_[2];
					*OutVal++ = (BcF32)Vertex.iJoints_[3];
				}
				break;
			case RsVertexUsage::BLENDWEIGHTS:
				BcAssert( VertexElement.Components_ == 4 );
				BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
				{
					BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
					*OutVal++ = (BcF32)Vertex.Weights_[0];
					*OutVal++ = (BcF32)Vertex.Weights_[1];
					*OutVal++ = (BcF32)Vertex.Weights_[2];
					*OutVal++ = (BcF32)Vertex.Weights_[3];
				}
				break;
			case RsVertexUsage::COLOUR:
				BcAssert( VertexElement.Components_ == 4 );
				BcAssert( VertexElement.DataType_ == RsVertexDataType::UBYTE_NORM );
				{
					BcU32* OutVal = reinterpret_cast< BcU32* >( &VertexData[ VertexElement.Offset_ ] );
					*OutVal++ = RsColour( Vertex.Colour_ ).asABGR();
				}
				break;

			default:
				break;
			};
		}

		VertexDataStream_.push( &VertexData[0], VertexData.size() );
	}
}

//////////////////////////////////////////////////////////////////////////
// recursiveSerialiseNodes
void ScnModelImport::recursiveSerialiseNodes( struct aiNode* Node,
									          BcU32 ParentIndex,
									          BcU32& NodeIndex,
									          BcU32& PrimitiveIndex )
{
	// Setup structs.
	ScnModelNodeTransformData NodeTransformData =
	{
		MaMat4d( Node->mTransformation[0] ),
		MaMat4d(),	// todo: absolute
		MaMat4d()	// todo: inv bind pose.
	};
	
	ScnModelNodePropertyData NodePropertyData = 
	{
		ParentIndex,
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
		for( BcU32 Idx = 0; Idx < Node->mNumMeshes; ++Idx )
		{
			serialiseMesh( 
				Scene_->mMeshes[ Node->mMeshes[ Idx ] ],
				ParentIndex, 
				NodeIndex,
				PrimitiveIndex );
		}
	}
		
	// Recurse into children.
	for( BcU32 Idx = 0; Idx < Node->mNumChildren; ++Idx )
	{
		recursiveSerialiseNodes( 
			Node->mChildren[ Idx ],
			ParentIndex,
			NodeIndex,
			PrimitiveIndex );
	}
}

//////////////////////////////////////////////////////////////////////////
// serialiseMesh
void ScnModelImport::serialiseMesh( struct aiMesh* Mesh,
                                    BcU32 ParentIndex,
                                    BcU32& NodeIndex,
									BcU32& PrimitiveIndex )
{
	if( Mesh->HasPositions() && Mesh->HasFaces() )
	{
		ScnShaderPermutationFlags ShaderPermutation = ScnShaderPermutationFlags::MESH_STATIC_3D;

		// Calculate number of primitives.
		BcAssert( BcBitsSet( Mesh->mPrimitiveTypes ) == 1 );
		
		// NOTE: This next section needs to be picky to be optimal. Optimise later :)
		ScnModelMeshData MeshData = 
		{
			ParentIndex,
			BcFalse,
			RsTopologyType::TRIANGLE_LIST,	
			ShaderPermutation,
			0,
			BcErrorCode,
			0, // padding0
			0, // padding1
			MaAABB(),
			Mesh->mNumVertices,
			5,
			48,
			nullptr
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

		// Vertex format.
		RsVertexDeclarationDesc VertexDeclarationDesc = RsVertexDeclarationDesc( 5 )
			.addElement( RsVertexElement( 0, 0,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 ) )
			.addElement( RsVertexElement( 0, 12,		3,		RsVertexDataType::FLOAT32,		RsVertexUsage::NORMAL,			0 ) )
			.addElement( RsVertexElement( 0, 24,		3,		RsVertexDataType::FLOAT32,		RsVertexUsage::TANGENT,			0 ) )
			.addElement( RsVertexElement( 0, 36,		2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 ) )
			.addElement( RsVertexElement( 0, 44,		4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 ) );
		VertexDeclarations_.push_back( VertexDeclarationDesc );

		// Export vertices.
		serialiseVertices( Mesh, 
			&VertexDeclarationDesc.Elements_[ 0 ], 
			VertexDeclarationDesc.Elements_.size(), 
			MeshData.AABB_ );
		
		// Grab material name.
		std::string Material;
		//Mesh->mMaterialIndex
		
		// Always setup default material.
		//if( Material.Name_.length() == 0 )
		{
			Material = "$(ScnMaterial:materials.default)";
		}
		//else
		//{
			// Add the cross package reference.
			//Material.Name_ = std::string("$(ScnMaterial:") + Material.Name_ + std::string(")");
		//}

		// Import material.
		// TODO: Pass through parameters from the model into import?
		MeshData.MaterialRef_ = pImporter_->addPackageCrossRef( Material.c_str() );
		
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
}

//////////////////////////////////////////////////////////////////////////
// serialiseVertices
void ScnModelImport::serialiseVertices( struct aiMesh* Mesh,
                                        RsVertexElement* pVertexElements,
										BcU32 NoofVertexElements,
										MaAABB& AABB )
{
	AABB.empty();

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
					BcAssert( VertexElement.Components_ == 3 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = Position.x;
						*OutVal++ = Position.y;
						*OutVal++ = Position.z;
					}
				}
				break;
			case RsVertexUsage::NORMAL:
				{
					BcAssert( VertexElement.Components_ == 3 );
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
					}
				}
				break;
			case RsVertexUsage::TANGENT:
				{
					BcAssert( VertexElement.Components_ == 3 );
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
							*OutVal++ = Texcoord[ ComponentIdx ];
						}
					}
				}
				break;
			case RsVertexUsage::COLOUR:
				{
					BcAssert( VertexElement.UsageIdx_ < AI_MAX_NUMBER_OF_COLOR_SETS );
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::UBYTE_NORM );
					aiColor4D Colour = 
						Mesh->mColors[ VertexElement.UsageIdx_ ] != nullptr ? 
						Mesh->mColors[ VertexElement.UsageIdx_ ][ VertexIdx ] : 
						aiColor4D( 1.0f, 1.0f, 1.0f, 1.0f );
					{
						BcU32* OutVal = reinterpret_cast< BcU32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = RsColour( Colour.r, Colour.g, Colour.b, Colour.a ).asABGR();
					}
				}
				break;
				/*
			case RsVertexUsage::BLENDINDICES:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = (BcF32)Vertex.iJoints_[0];
						*OutVal++ = (BcF32)Vertex.iJoints_[1];
						*OutVal++ = (BcF32)Vertex.iJoints_[2];
						*OutVal++ = (BcF32)Vertex.iJoints_[3];
					}
				}
				break;
			case RsVertexUsage::BLENDWEIGHTS:
				{
					BcAssert( VertexElement.Components_ == 4 );
					BcAssert( VertexElement.DataType_ == RsVertexDataType::FLOAT32 );
					{
						BcF32* OutVal = reinterpret_cast< BcF32* >( &VertexData[ VertexElement.Offset_ ] );
						*OutVal++ = (BcF32)Vertex.Weights_[0];
						*OutVal++ = (BcF32)Vertex.Weights_[1];
						*OutVal++ = (BcF32)Vertex.Weights_[2];
						*OutVal++ = (BcF32)Vertex.Weights_[3];
					}
				}
				break;
				*/

			default:
				break;
			};
		}

		VertexDataStream_.push( &VertexData[0], VertexData.size() );
	}
}

#endif
