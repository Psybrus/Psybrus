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

#include <memory>

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
		
		// Write to file.
		Importer.addChunk( BcHash( "header" ), HeaderStream_.pData(), HeaderStream_.dataSize() );
		Importer.addChunk( BcHash( "nodetransformdata" ), NodeTransformDataStream_.pData(), NodeTransformDataStream_.dataSize() );
		Importer.addChunk( BcHash( "nodepropertydata" ), NodePropertyDataStream_.pData(), NodePropertyDataStream_.dataSize() );
		Importer.addChunk( BcHash( "vertexdata" ), VertexDataStream_.pData(), VertexDataStream_.dataSize() );
		Importer.addChunk( BcHash( "indexdata" ), IndexDataStream_.pData(), IndexDataStream_.dataSize() );
		Importer.addChunk( BcHash( "vertexelements" ), VertexElementStream_.pData(), VertexElementStream_.dataSize() );
		Importer.addChunk( BcHash( "primitivedata" ), PrimitiveDataStream_.pData(), PrimitiveDataStream_.dataSize() );
		
		//
		return BcTrue;
	}
	return BcFalse;
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
		pNode->absoluteTransform(),
		pNode->inverseBindpose()
	};
	
	ScnModelNodePropertyData NodePropertyData = 
	{
		ParentIndex,
		pImporter_->addString( BcName::StripInvalidChars( pNode->name() ).c_str() ),
		pNode->type() == eNT_JOINT
	};
	
	// Serialise.
	NodeTransformDataStream_ << NodeTransformData;
	NodePropertyDataStream_ << NodePropertyData;
	
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
		ScnModelPrimitiveData PrimitiveData = 
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
		auto CurrentPosition = VertexElementStream_.dataSize();
		VertexElementStream_ << RsVertexElement( 0, 0,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 );
		VertexElementStream_ <<	RsVertexElement( 0, 12,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::NORMAL,			0 );
		VertexElementStream_ <<	RsVertexElement( 0, 24,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::TANGENT,			0 );
		VertexElementStream_ << RsVertexElement( 0, 36,			2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 );
		VertexElementStream_ << RsVertexElement( 0, 44,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 );

		// Export vertices.
		serialiseVertices( pMesh, (RsVertexElement*)(&(VertexElementStream_.pData()[ CurrentPosition ])), 5, PrimitiveData.AABB_ );
		
		// Grab material name.
		MdlMaterial Material = pMesh->material( 0 );
			
		// Always setup default material.
		if( Material.Name_.length() == 0 )
		{
			Material.Name_ = "$(ScnMaterial:default.default)";
		}
		else
		{
			// Add the cross package reference.
			Material.Name_ = std::string("$(ScnMaterial:") + Material.Name_ + std::string(")");
		}

		// Import material.
		// TODO: Pass through parameters from the model into import?
		PrimitiveData.MaterialRef_ = pImporter_->addPackageCrossRef( Material.Name_.c_str() );
		PrimitiveDataStream_ << PrimitiveData;
					

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
		ScnModelPrimitiveData PrimitiveData = 
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
		auto CurrentPosition = VertexElementStream_.dataSize();
		VertexElementStream_ << RsVertexElement( 0, 0,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::POSITION,		0 );
		VertexElementStream_ <<	RsVertexElement( 0, 12,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::NORMAL,			0 );
		VertexElementStream_ <<	RsVertexElement( 0, 24,			3,		RsVertexDataType::FLOAT32,		RsVertexUsage::TANGENT,			0 );
		VertexElementStream_ << RsVertexElement( 0, 36,			2,		RsVertexDataType::FLOAT32,		RsVertexUsage::TEXCOORD,		0 );
		VertexElementStream_ << RsVertexElement( 0, 44,			4,		RsVertexDataType::FLOAT32,		RsVertexUsage::BLENDINDICES,	0 );
		VertexElementStream_ << RsVertexElement( 0, 60,			4,		RsVertexDataType::FLOAT32,		RsVertexUsage::BLENDWEIGHTS,	0 );
		VertexElementStream_ << RsVertexElement( 0, 76,			4,		RsVertexDataType::UBYTE_NORM,	RsVertexUsage::COLOUR,			0 );

		// Export vertices.
		serialiseVertices( pSkin, (RsVertexElement*)(&(VertexElementStream_.pData()[ CurrentPosition ])), 7, PrimitiveData.AABB_ );

		// Setup bone palette for primitive.
		const MdlBonePalette& BonePalette( pSkin->bonePalette() );
		BcMemSet( PrimitiveData.BonePalette_, 0xff, sizeof( PrimitiveData.BonePalette_ ) );
		BcAssert( BonePalette.BonePalette_.size() <= SCN_MODEL_BONE_PALETTE_SIZE );
		for( BcU32 BoneIdx = 0; BoneIdx < BonePalette.BonePalette_.size(); ++BoneIdx )
		{
			if( BoneIdx < BonePalette.BonePalette_.size() )
			{
				PrimitiveData.BonePalette_[ BoneIdx ] = BonePalette.BonePalette_[ BoneIdx ] + NodeIndex;
			}
		}

		// Grab material name.
		MdlMaterial Material = pSkin->material( 0 );
			
		// Always setup default material.
		if( Material.Name_.length() == 0 )
		{
			Material.Name_ = "$(ScnMaterial:default.default)";
		}
		else
		{
			// Add the cross package reference.
			Material.Name_ = std::string("$(ScnMaterial:") + Material.Name_ + std::string(")");
		}

		// Import material.
		// TODO: Pass through parameters from the model into import?
		PrimitiveData.MaterialRef_ = pImporter_->addPackageCrossRef( Material.Name_.c_str() );
		PrimitiveDataStream_ << PrimitiveData;
							
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

#endif
