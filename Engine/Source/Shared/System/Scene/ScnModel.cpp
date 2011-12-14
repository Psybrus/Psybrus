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
BcBool ScnModel::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const std::string& FileName = Object[ "source" ].asString();
	MdlNode* pNode = MdlLoader::loadModel( FileName.c_str() );

	// Add root dependancy.
	DependancyList.push_back( FileName );
	
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

		// Delete root node.
		delete pNode;
		pNode = NULL;
		
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
	if( pNode->pMeshObject() != NULL || pNode->pSkinObject() != NULL )
	{
		MdlMesh* pMesh = pNode->pMeshObject() ? pNode->pMeshObject() : pNode->pSkinObject();
		
		// Split up mesh by material.
		const std::vector< MdlMesh >& SubMeshes = pMesh->splitByMaterial();
		
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
					BcStrCopyN( PrimitiveData.MaterialName_, (*MaterialRef->getName()).c_str(), sizeof( PrimitiveData.MaterialName_ ) );
				}
				else
				{
					BcStrCopy( PrimitiveData.MaterialName_, "default" );
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
// StaticPropertyTable
void ScnModel::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnMaterial" )
		.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

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
	if( pHeader_ != NULL )
	{
		for( BcU32 Idx = 0; Idx < PrimitiveRuntimes_.size(); ++Idx )
		{
			if( PrimitiveRuntimes_[ Idx ].MaterialRef_.isValid() )
			{
				if( PrimitiveRuntimes_[ Idx ].MaterialRef_.isReady() == BcFalse )
				{
					return BcFalse;
				}
			}
		}
		
	}
	return pHeader_ != NULL && PrimitiveRuntimes_.size() > 0;
}

//////////////////////////////////////////////////////////////////////////
// createInstance
BcBool ScnModel::createInstance( const std::string& Name, ScnModelInstanceRef& Handle )
{
	return CsCore::pImpl()->createResource( Name, Handle, this );
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
		//TNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pPrimitiveData->NodeIndex_ ];

		// Create GPU resources.
		RsVertexBuffer* pVertexBuffer = RsCore::pImpl() ? RsCore::pImpl()->createVertexBuffer( pPrimitiveData->VertexFormat_, pPrimitiveData->NoofVertices_, pVertexBufferData ) : NULL;
		RsIndexBuffer* pIndexBuffer = RsCore::pImpl() ? RsCore::pImpl()->createIndexBuffer( pPrimitiveData_->NoofIndices_, pIndexBufferData ) : NULL;
		RsPrimitive* pPrimitive = RsCore::pImpl() ? RsCore::pImpl()->createPrimitive( pVertexBuffer, pIndexBuffer ) : NULL;

		// Setup runtime structure.
		TPrimitiveRuntime PrimitiveRuntime = 
		{
			PrimitiveIdx,
			pVertexBuffer,
			pIndexBuffer,
			pPrimitive,
			NULL
		};
		
		// Get resource.
		if( CsCore::pImpl()->requestResource( pPrimitiveData->MaterialName_, PrimitiveRuntime.MaterialRef_ ) )
		{
			// Push into array.
			PrimitiveRuntimes_.push_back( PrimitiveRuntime );
		}
				
		// Advance vertex and index buffers.
		pVertexBufferData_ += pPrimitiveData->NoofVertices_ * RsVertexDeclSize( pPrimitiveData->VertexFormat_ );
		pIndexBufferData_ += pPrimitiveData->NoofIndices_ * sizeof( BcU16 );
	}
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnModel::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
	getChunk( 1 );
	getChunk( 2 );
	getChunk( 3 );
	getChunk( 4 );
	getChunk( 5 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnModel::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
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


//////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( ScnModelInstance );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnModelInstance::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	PropertyTable.beginCatagory( "ScnModelInstance" )
		//.field( "source",					csPVT_FILE,			csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnModelInstance::initialise( ScnModelRef Parent )
{
	// Cache parent.
	Parent_ = Parent;
	
	// Duplicate node data for update/rendering.
	BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	pNodeTransformData_ = new ScnModel::TNodeTransformData[ NoofNodes ];
	BcMemCopy( pNodeTransformData_, Parent_->pNodeTransformData_, sizeof( ScnModel::TNodeTransformData ) * NoofNodes );

	// Create material instances to render with.
	ScnModel::TPrimitiveRuntimeList& PrimitiveRuntimes = Parent_->PrimitiveRuntimes_;
	ScnMaterialInstanceRef MaterialInstanceRef;
	MaterialInstanceDescList_.reserve( PrimitiveRuntimes.size() );
	for( BcU32 Idx = 0; Idx < PrimitiveRuntimes.size(); ++Idx )
	{
		ScnModel::TPrimitiveRuntime* pPrimitiveRuntime = &PrimitiveRuntimes[ Idx ];
		
		if( pPrimitiveRuntime->MaterialRef_.isValid() )
		{
			BcAssert( pPrimitiveRuntime->MaterialRef_.isReady() );
						
			// Even on failure add. List must be of same size for quick lookups.
			pPrimitiveRuntime->MaterialRef_->createInstance( getName().getValue() + "_MaterialInstance", MaterialInstanceRef, scnSPF_DEFAULT );
			
			TMaterialInstanceDesc MaterialInstanceDesc =
			{
				MaterialInstanceRef,
				MaterialInstanceRef->findParameter( "uWorldMatrix" )				
			};

			MaterialInstanceDescList_.push_back( MaterialInstanceDesc );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnModelInstance::destroy()
{
	// Delete duplicated node data.
	delete [] pNodeTransformData_;
	pNodeTransformData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnModelInstance::isReady()
{
	return Parent_->isReady();
}

//////////////////////////////////////////////////////////////////////////
// setTransform
void ScnModelInstance::setTransform( BcU32 NodeIdx, const BcMat4d& LocalTransform )
{
	BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	if( NodeIdx < NoofNodes )
	{
		pNodeTransformData_[ NodeIdx ].RelativeTransform_ = LocalTransform;
	}
}

//////////////////////////////////////////////////////////////////////////
// updateNodes
void ScnModelInstance::update()
{
	BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	for( BcU32 NodeIdx = 0; NodeIdx < NoofNodes; ++NodeIdx )
	{
		ScnModel::TNodeTransformData* pNodeTransformData = &pNodeTransformData_[ NodeIdx ];
		ScnModel::TNodePropertyData* pNodePropertyData = &Parent_->pNodePropertyData_[ NodeIdx ];
		
		// Check parent index and process.
		if( pNodePropertyData->ParentIndex_ != BcErrorCode )
		{
			ScnModel::TNodeTransformData* pParentNodeTransformData = &pNodeTransformData_[ pNodePropertyData->ParentIndex_ ];
			
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
class ScnModelInstanceRenderNode: public RsRenderNode
{
public:
	void render()
	{
		pPrimitive_->render( Type_,
		                     Offset_,
		                     NoofIndices_ );
	}

	eRsPrimitiveType Type_;
	BcU32 Offset_;
	BcU32 NoofIndices_;
	RsPrimitive* pPrimitive_;
};

void ScnModelInstance::render( RsFrame* pFrame, RsRenderSort Sort )
{
	ScnModel::TPrimitiveRuntimeList& PrimitiveRuntimes = Parent_->PrimitiveRuntimes_;
	ScnModel::TPrimitiveData* pPrimitiveDatas = Parent_->pPrimitiveData_;

	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < PrimitiveRuntimes.size(); ++PrimitiveIdx )
	{
		ScnModel::TPrimitiveRuntime* pPrimitiveRuntime = &PrimitiveRuntimes[ PrimitiveIdx ];
		ScnModel::TPrimitiveData* pPrimitiveData = &pPrimitiveDatas[ pPrimitiveRuntime->PrimitiveDataIndex_ ];
		ScnModel::TNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pPrimitiveData->NodeIndex_ ];
		TMaterialInstanceDesc& MaterialInstanceDesc = MaterialInstanceDescList_[ PrimitiveIdx ];
		BcU32 Offset = 0; // This will change when index buffers are merged.

		// If we have a valid material instance, then we can render the node.
		if( MaterialInstanceDesc.MaterialInstanceRef_.isValid() )
		{
			// Set model parameters on material.
			MaterialInstanceDesc.MaterialInstanceRef_->setParameter( MaterialInstanceDesc.WorldMatrixIdx_, pNodeTransformData->AbsoluteTransform_ );
			
			// Bind material.
			MaterialInstanceDesc.MaterialInstanceRef_->bind( pFrame, Sort );
			
			// Render primitive.
			ScnModelInstanceRenderNode* pRenderNode = pFrame->newObject< ScnModelInstanceRenderNode >();
			
			pRenderNode->Type_ = pPrimitiveData->Type_;
			pRenderNode->Offset_ = Offset;
			pRenderNode->NoofIndices_ = pPrimitiveData->NoofIndices_;
			pRenderNode->pPrimitive_ = pPrimitiveRuntime->pPrimitive_;
			pRenderNode->Sort_ = Sort;
			
			pFrame->addRenderNode( pRenderNode );
		}
	}
}
