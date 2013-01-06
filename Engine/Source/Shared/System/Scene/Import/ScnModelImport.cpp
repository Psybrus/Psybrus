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
		const std::vector< MdlMesh >& SubMeshes = pMesh->splitByMaterial();
		
		// Export a primitive for each submesh.
		for( BcU32 SubMeshIdx = 0; SubMeshIdx < SubMeshes.size(); ++SubMeshIdx )
		{
			const MdlMesh* pSubMesh = &SubMeshes[ SubMeshIdx ];

			if( pSubMesh->nVertices() > 0 )
			{
				BcU32 VertexFormat = 0;
				BcU32 ShaderPermutation = 0;
				if( IsSkin )
				{
					VertexFormat = rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TANGENT_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_SKIN_INDICES | rsVDF_SKIN_WEIGHTS | rsVDF_COLOUR_ABGR8;
					ShaderPermutation = scnSPF_SKINNED_3D;
				}
				else
				{
					VertexFormat = rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TANGENT_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_ABGR8;
					ShaderPermutation = scnSPF_STATIC_3D;
				}

				// NOTE: This next section needs to be picky to be optimal. Optimise later :)
				ScnModelPrimitiveData PrimitiveData = 
				{
					ParentIndex,
					rsPT_TRIANGLELIST,	
					VertexFormat,
					ShaderPermutation,
					pSubMesh->nVertices(),
					pSubMesh->nIndices(),
					BcErrorCode,
					0, // padding0
					0, // padding1
					BcAABB()
				};

				// Export vertices.
				MdlVertex Vertex;
				BcU32 MaxBone = 0;
				for( BcU32 VertexIdx = 0; VertexIdx < pSubMesh->nVertices(); ++VertexIdx )
				{
					Vertex = pSubMesh->vertex( VertexIdx );
					VertexDataStream_ << Vertex.Position_.x() << Vertex.Position_.y() << Vertex.Position_.z();
					VertexDataStream_ << Vertex.Normal_.x() << Vertex.Normal_.y() << Vertex.Normal_.z();
					VertexDataStream_ << Vertex.Tangent_.x() << Vertex.Tangent_.y() << Vertex.Tangent_.z();
					VertexDataStream_ << Vertex.UV_.x() << Vertex.UV_.y();

					if( IsSkin )
					{
						VertexDataStream_ << BcF32( Vertex.iJoints_[ 0 ] ) << BcF32( Vertex.iJoints_[ 1 ] ) << BcF32( Vertex.iJoints_[ 2 ] ) << BcF32( Vertex.iJoints_[ 3 ] );
						VertexDataStream_ << Vertex.Weights_[ 0 ] << Vertex.Weights_[ 1 ] << Vertex.Weights_[ 2 ] << Vertex.Weights_[ 3 ];

						MaxBone = BcMax( MaxBone, Vertex.iJoints_[ 0 ] );
						MaxBone = BcMax( MaxBone, Vertex.iJoints_[ 1 ] );
						MaxBone = BcMax( MaxBone, Vertex.iJoints_[ 2 ] );
						MaxBone = BcMax( MaxBone, Vertex.iJoints_[ 3 ] );
					}
					
					VertexDataStream_ << RsColour( Vertex.Colour_ ).asABGR();

					// Expand AABB.
					PrimitiveData.AABB_.expandBy( Vertex.Position_ );
				}

				// Setup bone palette with an offset.
				// TODO: Need to break up into multiple meshes for this really!!
				for( BcU32 BoneIdx = 0; BoneIdx < 24; ++BoneIdx )
				{
					if( BoneIdx <= MaxBone )
					{
						PrimitiveData.BonePalette_[ BoneIdx ] = BoneIdx + NodeIndex;
					}
					else
					{
						PrimitiveData.BonePalette_[ BoneIdx ] = BcErrorCode;
					}
				}

				// Grab material name.
				MdlMaterial Material = pSubMesh->material( 0 );
			
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
				for( BcU32 IndexIdx = 0; IndexIdx < pSubMesh->nIndices(); ++IndexIdx )
				{
					Index = pSubMesh->index( IndexIdx );
					BcAssert( Index.iVertex_ < 0x10000 );
					IndexDataStream_ << BcU16( Index.iVertex_ );
				}
			
				// Update primitive index.
				++PrimitiveIndex;
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

#endif
