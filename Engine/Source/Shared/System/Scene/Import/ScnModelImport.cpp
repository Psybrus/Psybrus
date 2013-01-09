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

#ifdef PSY_SERVER

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
		pImporter_->addString( BcName::StripInvalidChars( pNode->name() ).c_str() )
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
		BcU32 VertexFormat = rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TANGENT_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_ABGR8;
		BcU32 ShaderPermutation = scnSPF_STATIC_3D;

		// NOTE: This next section needs to be picky to be optimal. Optimise later :)
		ScnModelPrimitiveData PrimitiveData = 
		{
			ParentIndex,
			rsPT_TRIANGLELIST,	
			VertexFormat,
			ShaderPermutation,
			pMesh->nVertices(),
			pMesh->nIndices(),
			BcErrorCode,
			0, // padding0
			0, // padding1
			BcAABB()
		};

		// Export vertices.
		serialiseVertices( pMesh, VertexFormat, PrimitiveData.AABB_ );
		
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
	BcU32 BonePaletteSize = 24;
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
		BcU32 VertexFormat = rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TANGENT_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_SKIN_INDICES | rsVDF_SKIN_WEIGHTS | rsVDF_COLOUR_ABGR8;
		BcU32 ShaderPermutation = scnSPF_SKINNED_3D;

		// NOTE: This next section needs to be picky to be optimal. Optimise later :)
		ScnModelPrimitiveData PrimitiveData = 
		{
			ParentIndex,
			rsPT_TRIANGLELIST,	
			VertexFormat,
			ShaderPermutation,
			pSkin->nVertices(),
			pSkin->nIndices(),
			BcErrorCode,
			0, // padding0
			0, // padding1
			BcAABB()
		};

		// Export vertices.
		serialiseVertices( pSkin, VertexFormat, PrimitiveData.AABB_ );

		// Setup bone palette for primitive.
		const MdlBonePalette& BonePalette( pSkin->bonePalette() );
		BcMemSet( PrimitiveData.BonePalette_, 0xff, sizeof( PrimitiveData.BonePalette_ ) );
		BcAssert( BonePalette.BonePalette_.size() <= 24 );
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
                                        BcU32 VertexFormat,
										BcAABB& AABB )
{
	AABB.empty();
	for( BcU32 VertexIdx = 0; VertexIdx < pMesh->nVertices(); ++VertexIdx )
	{
		const MdlVertex& Vertex = pMesh->vertex( VertexIdx );

		if( VertexFormat & rsVDF_POSITION_XYZ )
		{
			VertexDataStream_ << Vertex.Position_.x() << Vertex.Position_.y() << Vertex.Position_.z();
		}

		if( VertexFormat & rsVDF_NORMAL_XYZ )
		{
			VertexDataStream_ << Vertex.Normal_.x() << Vertex.Normal_.y() << Vertex.Normal_.z();
		}

		if( VertexFormat & rsVDF_TANGENT_XYZ )
		{
			VertexDataStream_ << Vertex.Tangent_.x() << Vertex.Tangent_.y() << Vertex.Tangent_.z();
		}

		if( VertexFormat & rsVDF_TEXCOORD_UV0 )
		{
			VertexDataStream_ << Vertex.UV_.x() << Vertex.UV_.y();
		}

		if( VertexFormat & rsVDF_SKIN_INDICES )
		{
			VertexDataStream_ << BcF32( Vertex.iJoints_[ 0 ] ) << BcF32( Vertex.iJoints_[ 1 ] ) << BcF32( Vertex.iJoints_[ 2 ] ) << BcF32( Vertex.iJoints_[ 3 ] );
		}

		if( VertexFormat & rsVDF_SKIN_WEIGHTS )
		{
			VertexDataStream_ << Vertex.Weights_[ 0 ] << Vertex.Weights_[ 1 ] << Vertex.Weights_[ 2 ] << Vertex.Weights_[ 3 ];
		}
					
		if( VertexFormat & rsVDF_COLOUR_ABGR8 )
		{
			VertexDataStream_ << RsColour( Vertex.Colour_ ).asABGR();
		}

		// Expand AABB.
		AABB.expandBy( Vertex.Position_ );
	}
}

#endif
