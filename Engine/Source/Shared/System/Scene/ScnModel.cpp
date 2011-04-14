/**************************************************************************
*
* File:		ScnModel.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnModel.h"

#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#include "Mdl.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnModel::import( const Json::Value& Object )
{
	const std::string& FileName = Object[ "source" ].asString();
	MdlNode* pNode = MdlLoader::loadModel( FileName.c_str() );
	
	if( pNode != NULL )
	{
		BcStream HeaderStream;
		BcStream NodeTransformDataStream;
		BcStream NodePropertyDataStream;
		BcStream VertexDataStream;
		BcStream IndexDataStream;
		BcStream PrimitiveDataStream;
		
		BcU32 NodeIndex = 0;
		BcU32 PrimitiveIndex = 0;
		
		recursiveSerialiseNodes( NodeTransformDataStream,
								 NodePropertyDataStream, 
								 VertexDataStream, 
								 IndexDataStream, 
								 PrimitiveDataStream, 
								 pNode, 
								 BcErrorCode, 
								 NodeIndex, 
								 PrimitiveIndex );

		// Setup header.
		THeader Header = 
		{
			NodeIndex,
			PrimitiveIndex
		};
		
		HeaderStream << Header;
		
		// Write to file.
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "nodetransformdata" ), NodeTransformDataStream.pData(), NodeTransformDataStream.dataSize() );
		pFile_->addChunk( BcHash( "nodepropertydata" ), NodePropertyDataStream.pData(), NodePropertyDataStream.dataSize() );
		pFile_->addChunk( BcHash( "vertexdata" ), VertexDataStream.pData(), VertexDataStream.dataSize() );
		pFile_->addChunk( BcHash( "indexdata" ), IndexDataStream.pData(), IndexDataStream.dataSize() );
		pFile_->addChunk( BcHash( "primitivedata" ), PrimitiveDataStream.pData(), PrimitiveDataStream.dataSize() );
		
		//
		return BcTrue;
	}
	return BcFalse;
}

void ScnModel::recursiveSerialiseNodes( BcStream& TransformStream,
									    BcStream& PropertyStream,
									    BcStream& VertexStream,
									    BcStream& IndexStream,
									    BcStream& PrimitiveStream,
									    MdlNode* pNode,
									    BcU32 ParentIndex,
									    BcU32& NodeIndex,
									    BcU32& PrimitiveIndex )
{
	// Setup structs.
	TNodeTransformData NodeTransformData =
	{
		pNode->relativeTransform(),
		pNode->absoluteTransform(),
		pNode->inverseBindpose()
	};
	
	TNodePropertyData NodePropertyData = 
	{
		ParentIndex
	};
	
	// Serialise.
	TransformStream << NodeTransformData;
	PropertyStream << NodePropertyData;
	
	// Update parent & node index.
	ParentIndex = NodeIndex++;

	// Setup primitive data.
	// NOTE: Do skin later.
	if( pNode->pMeshObject() != NULL )
	{
		MdlMesh* pMesh = pNode->pMeshObject();
		
		// Split up mesh by material.
		const std::vector< MdlMesh >& SubMeshes = pNode->pMeshObject()->splitByMaterial();
		
		// Export a primitive for each submesh.
		for( BcU32 SubMeshIdx = 0; SubMeshIdx < SubMeshes.size(); ++SubMeshIdx )
		{
			const MdlMesh* pSubMesh = &SubMeshes[ SubMeshIdx ];

			// NOTE: This next section needs to be picky to be optimal. Optimise later :)
			TPrimitiveData PrimitiveData = 
			{
				ParentIndex,
				rsPT_TRIANGLELIST,	
				rsVDF_POSITION_XYZ | rsVDF_NORMAL_XYZ | rsVDF_TANGENT_XYZ | rsVDF_TEXCOORD_UV0 | rsVDF_COLOUR_RGBA8,
				pSubMesh->nVertices(),
				pSubMesh->nIndices()
			};
			
			// Grab material name.
			MdlMaterial Material = pSubMesh->material( 0 );
			BcMemZero( PrimitiveData.MaterialName_, sizeof( PrimitiveData.MaterialName_ ) );
			
			// Import material.
			// TODO: Pass through parameters from the model into import?
			if( Material.Name_.length() > 0 )
			{
				ScnMaterialRef MaterialRef;
				if( CsCore::pImpl()->importResource( Material.Name_, MaterialRef ) )
				{
					BcStrCopyN( PrimitiveData.MaterialName_, MaterialRef->getName().c_str(), sizeof( PrimitiveData.MaterialName_ ) );
				}
			}
			
			PrimitiveStream << PrimitiveData;
			
			// Export vertices.
			MdlVertex Vertex;
			for( BcU32 VertexIdx = 0; VertexIdx < pSubMesh->nVertices(); ++VertexIdx )
			{
				Vertex = pSubMesh->vertex( VertexIdx );
				VertexStream << Vertex.Position_.x() << Vertex.Position_.y() << Vertex.Position_.z();
				VertexStream << Vertex.Normal_.x() << Vertex.Normal_.y() << Vertex.Normal_.z();
				VertexStream << Vertex.Tangent_.x() << Vertex.Tangent_.y() << Vertex.Tangent_.z();
				VertexStream << Vertex.UV_.x() << Vertex.UV_.y();
				VertexStream << RsColour( Vertex.Colour_ ).asRGBA();
			}
		
			// Export indices.
			MdlIndex Index;
			for( BcU32 IndexIdx = 0; IndexIdx < pSubMesh->nIndices(); ++IndexIdx )
			{
				Index = pSubMesh->index( IndexIdx );
				IndexStream << BcU16( IndexIdx );
			}
			
			// Update primitive index.
			++PrimitiveIndex;
		}
	}
		
	// Recurse into children.
	MdlNode* pChild = pNode->pChild();
	
	while( pChild != NULL )
	{
		recursiveSerialiseNodes( TransformStream,
								 PropertyStream,
								 VertexStream,
								 IndexStream,
								 PrimitiveStream,
								 pChild,
								 ParentIndex,
								 NodeIndex,
								 PrimitiveIndex );
		
		pChild = pChild->pNext();
	}
}



#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnModel );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnModel::initialise()
{
	// NULL internals.
	pHeader_ = NULL;
	pNodeTransformData_ = NULL;
	pNodePropertyData_ = NULL;
	pVertexBufferData_ = NULL;
	pIndexBufferData_ = NULL;
	pPrimitiveData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnModel::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnModel::destroy()
{
	// TODO: Release runtime data.
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnModel::isReady()
{
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// setup
void ScnModel::setup()
{
	// NOTE: This should try and compact index and vertex buffers so we create less
	//       GPU resources. This could be done import time, but it could vary
	//       platform to platform.
	//       Also, if we compact, we will need to split it all up by vertex format,
	//       possibly even sort it by vertex format.
	
	// Setup primitive runtime.
	PrimitiveRuntimes_.reserve( pHeader_->NoofPrimitives_ );
	
	BcU8* pVertexBufferData = pVertexBufferData_;
	BcU8* pIndexBufferData = pIndexBufferData_;
	
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < pHeader_->NoofPrimitives_; ++PrimitiveIdx )
	{
		TPrimitiveData* pPrimitiveData = &pPrimitiveData_[ PrimitiveIdx ];
		TNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pPrimitiveData->NodeIndex_ ];

		// Create GPU resources.
		RsVertexBuffer* pVertexBuffer = RsCore::pImpl() ? RsCore::pImpl()->createVertexBuffer( pPrimitiveData->VertexFormat_, pPrimitiveData->NoofVertices_, pVertexBufferData ) : NULL;
		RsIndexBuffer* pIndexBuffer = RsCore::pImpl() ? RsCore::pImpl()->createIndexBuffer( pPrimitiveData_->NoofIndices_, pIndexBufferData ) : NULL;
		RsPrimitive* pPrimitive = RsCore::pImpl() ? RsCore::pImpl()->createPrimitive( pVertexBuffer, pIndexBuffer ) : NULL;

		// Setup runtime structure.
		TPrimitiveRuntime PrimitiveRuntime = 
		{
			pNodeTransformData,
			pPrimitiveData,
			pVertexBuffer,
			pIndexBuffer,
			pPrimitive,
		};
		
		// Get resource.
		if( CsCore::pImpl()->requestResource( pPrimitiveData->MaterialName_, PrimitiveRuntime.MaterialRef_ ) )
		{
			int a = 0;
			++a;
		}
		
		// Push into array.
		PrimitiveRuntimes_.push_back( PrimitiveRuntime );
		
		// Advance vertex and index buffers.
		pVertexBufferData_ += pPrimitiveData->NoofVertices_ * RsVertexDeclSize( pPrimitiveData->VertexFormat_ );
		pIndexBufferData_ += pPrimitiveData->NoofIndices_ * sizeof( BcU16 );
	}
}

//////////////////////////////////////////////////////////////////////////
// updateNodes
void ScnModel::updateNodes()
{
	// NOTE: This will be moved into an instance object, or
	//       made to act on an instance object.
	for( BcU32 NodeIdx = 0; NodeIdx < pHeader_->NoofNodes_; ++NodeIdx )
	{
		TNodeTransformData* pNodeTransformData = &pNodeTransformData_[ NodeIdx ];
		TNodePropertyData* pNodePropertyData = &pNodePropertyData_[ NodeIdx ];
		
		// Check parent index and process.
		if( pNodePropertyData->ParentIndex_ != BcErrorCode )
		{
			TNodeTransformData* pParentNodeTransformData = &pNodeTransformData_[ pNodePropertyData->ParentIndex_ ];
			
			pNodeTransformData->AbsoluteTransform_ = pParentNodeTransformData->AbsoluteTransform_ * pNodeTransformData->RelativeTransform_;
		}
		else
		{
			pNodeTransformData->AbsoluteTransform_ = pNodeTransformData->RelativeTransform_;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// renderPrimitives
void ScnModel::renderPrimitives()
{
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < PrimitiveRuntimes_.size(); ++PrimitiveIdx )
	{
		TPrimitiveRuntime* pPrimitiveRuntime = &PrimitiveRuntimes_[ PrimitiveIdx ];
		TPrimitiveData* pPrimitiveData = pPrimitiveRuntime->pPrimitiveData_;
		BcU32 Offset = 0; // This will change when index buffers are merged.

		// Bind material.
		if( pPrimitiveRuntime->MaterialRef_.isValid() )
		{
			//pPrimitiveRuntime->MaterialRef_->bind();
		
			// Render primitive.
			pPrimitiveRuntime->pPrimitive_->render( pPrimitiveData->Type_,
												    Offset,
												    pPrimitiveData->NoofIndices_ );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnModel::fileReady()
{
	// File is ready, get the header chunk.
	pFile_->getChunk( 0 );
	pFile_->getChunk( 1 );
	pFile_->getChunk( 2 );
	pFile_->getChunk( 3 );
	pFile_->getChunk( 4 );
	pFile_->getChunk( 5 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnModel::fileChunkReady( const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
	}
	else if( pChunk->ID_ == BcHash( "nodetransformdata" ) )
	{
		pNodeTransformData_ = (TNodeTransformData*)pData;
	}
	else if( pChunk->ID_ == BcHash( "nodepropertydata" ) )
	{
		pNodePropertyData_ = (TNodePropertyData*)pData;
	}
	else if( pChunk->ID_ == BcHash( "vertexdata" ) )
	{
		BcAssert( pVertexBufferData_ == NULL || pVertexBufferData_ == pData );
		pVertexBufferData_ = (BcU8*)pData;
	}
	else if( pChunk->ID_ == BcHash( "indexdata" ) )
	{
		BcAssert( pIndexBufferData_ == NULL || pIndexBufferData_ == pData );
		pIndexBufferData_ = (BcU8*)pData;
	}
	else if( pChunk->ID_ == BcHash( "primitivedata" ) )
	{
		pPrimitiveData_ = (TPrimitiveData*)pData;
		
		// We've got everything, it's time to setup.
		setup();
	}
}

