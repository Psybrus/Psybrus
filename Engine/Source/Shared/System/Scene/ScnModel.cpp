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

#include "System/Scene/ScnModel.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/ScnViewComponent.h"

#include "System/Scene/ScnLightingVisitor.h"

#include "System/Content/CsCore.h"
#include "System/SysKernel.h"

#ifdef PSY_SERVER
#include "System/Scene/Import/ScnModelImport.h"

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnModel::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	ScnModelImport ModelImport;
	return ModelImport.import( Importer, Object );
}

#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnModel );

void ScnModel::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "pHeader_",				&ScnModel::pHeader_ ),
	};
		
	ReRegisterClass< ScnModel, Super >( Fields );
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
	pMeshData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnModel::create()
{
	// NOTE: This should try and compact index and vertex buffers so we create less
	//       GPU resources. This could be done import time, but it could vary
	//       platform to platform.
	//       Also, if we compact, we will need to split it all up by vertex format,
	//       possibly even sort it by vertex format.
	
	// Setup primitive runtime.
	MeshRuntimes_.reserve( pHeader_->NoofPrimitives_ );
	
	BcU8* pVertexBufferData = pVertexBufferData_;
	BcU8* pIndexBufferData = pIndexBufferData_;
	
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < pHeader_->NoofPrimitives_; ++PrimitiveIdx )
	{
		ScnModelMeshData* pMeshData = &pMeshData_[ PrimitiveIdx ];
		
		//ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pMeshData->NodeIndex_ ];
		
		// Create GPU resources.
		RsVertexDeclarationDesc VertexDeclarationDesc( pMeshData_->NoofVertexElements_ );
		for( BcU32 Idx = 0; Idx < pMeshData->NoofVertexElements_; ++Idx )
		{
			VertexDeclarationDesc.addElement( pMeshData_->VertexElements_[ Idx ] );
		}
		
		RsVertexDeclaration* pVertexDeclaration = RsCore::pImpl()->createVertexDeclaration( VertexDeclarationDesc );
		BcU32 VertexBufferSize = pMeshData->NoofVertices_ * pMeshData->VertexStride_;
		RsBuffer* pVertexBuffer = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsBufferType::VERTEX, 
				RsResourceCreationFlags::STATIC,
				VertexBufferSize ) );

		RsCore::pImpl()->updateBuffer( 
			pVertexBuffer, 0, pMeshData->NoofVertices_ * pMeshData->VertexStride_, 
			RsResourceUpdateFlags::ASYNC,
			[ pVertexBufferData, VertexBufferSize ]
			( RsBuffer* Buffer, const RsBufferLock& BufferLock )
			{
				BcAssert( Buffer->getDesc().SizeBytes_ == VertexBufferSize );
				BcMemCopy( BufferLock.Buffer_, pVertexBufferData, 
					VertexBufferSize );
			} );
	
		BcU32 IndexBufferSize = pMeshData->NoofIndices_ * sizeof( BcU16 );
		RsBuffer* pIndexBuffer = 
			RsCore::pImpl()->createBuffer( 
				RsBufferDesc( 
					RsBufferType::INDEX, 
					RsResourceCreationFlags::STATIC, 
					IndexBufferSize ) );

		RsCore::pImpl()->updateBuffer( 
			pIndexBuffer, 0, pMeshData->NoofIndices_ * sizeof( BcU16 ), 
			RsResourceUpdateFlags::ASYNC,
			[ pIndexBufferData, IndexBufferSize ]
			( RsBuffer* Buffer, const RsBufferLock& BufferLock )
			{
				BcAssert( Buffer->getDesc().SizeBytes_ == IndexBufferSize );
				BcMemCopy( BufferLock.Buffer_, pIndexBufferData, 
					IndexBufferSize );
			} );
		
		// Setup runtime structure.
		ScnModelMeshRuntime MeshRuntime = 
		{
			PrimitiveIdx,
			pVertexDeclaration,
			pVertexBuffer,
			pIndexBuffer,
			NULL
		};
		
		// Get resource.
		auto Resource = getPackage()->getPackageCrossRef( pMeshData->MaterialRef_ );
		MeshRuntime.MaterialRef_ = Resource;
		BcAssertMsg( MeshRuntime.MaterialRef_.isValid(), "ScnModel: Material reference is invalid. Packing error." );

		// Push into array.
		MeshRuntimes_.push_back( MeshRuntime );
		
		// Advance vertex and index buffers.
		pVertexBufferData += pMeshData->NoofVertices_ * pMeshData->VertexStride_;
		pIndexBufferData += pMeshData->NoofIndices_ * sizeof( BcU16 );
	}

	// Mark as ready.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnModel::destroy()
{
	// Destroy internal data.
	for( BcU32 Idx = 0; Idx < MeshRuntimes_.size(); ++Idx )
	{
		ScnModelMeshRuntime& MeshRuntime( MeshRuntimes_[ Idx ] );

		RsCore::pImpl()->destroyResource( MeshRuntime.pVertexBuffer_ );
		RsCore::pImpl()->destroyResource( MeshRuntime.pIndexBuffer_ );
		
		MeshRuntime.MaterialRef_ = NULL;
	}

	MeshRuntimes_.clear();
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnModel::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
	requestChunk( 1 );
	requestChunk( 2 );
	requestChunk( 3 );
	requestChunk( 4 );
	requestChunk( 5 );
	requestChunk( 6 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnModel::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		requestChunk( 0 );
		return;
	}

	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = (ScnModelHeader*)pData;
	}
	else if( ChunkID == BcHash( "nodetransformdata" ) )
	{
		pNodeTransformData_ = (ScnModelNodeTransformData*)pData;
	}
	else if( ChunkID == BcHash( "nodepropertydata" ) )
	{
		pNodePropertyData_ = (ScnModelNodePropertyData*)pData;

		// Mark up node names.
		// TODO: Automate this process with reflection!
		for( BcU32 NodeIdx = 0; NodeIdx < pHeader_->NoofNodes_; ++NodeIdx )
		{
			ScnModelNodePropertyData* pNodePropertyNode = &pNodePropertyData_[ NodeIdx ];
			markupName( pNodePropertyNode->Name_ );
		}
	}
	else if( ChunkID == BcHash( "vertexdata" ) )
	{
		BcAssert( pVertexBufferData_ == NULL || pVertexBufferData_ == pData );
		pVertexBufferData_ = (BcU8*)pData;
	}
	else if( ChunkID == BcHash( "indexdata" ) )
	{
		BcAssert( pIndexBufferData_ == NULL || pIndexBufferData_ == pData );
		pIndexBufferData_ = (BcU8*)pData;
	}
	else if( ChunkID == BcHash( "vertexelements" ) )
	{
		pVertexElements_ = (RsVertexElement*)pData;
	}
	else if( ChunkID == BcHash( "meshdata" ) )
	{
		pMeshData_ = (ScnModelMeshData*)pData;

		RsVertexElement* pVertexElements = pVertexElements_;
		for( BcU32 Idx = 0; Idx < pHeader_->NoofPrimitives_; ++Idx )
		{
			pMeshData_->VertexElements_ = pVertexElements;
			pVertexElements += pMeshData_->NoofVertexElements_;
		}
		
		markCreate(); // All data loaded, time to create.
	}
}


//////////////////////////////////////////////////////////////////////////
// Define resource.
DEFINE_RESOURCE( ScnModelComponent );

void ScnModelComponent::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Parent_",							&ScnModelComponent::Parent_ ),
		ReField( "Layer_",							&ScnModelComponent::Layer_ ),
		ReField( "Pass_",							&ScnModelComponent::Pass_ ),
		ReField( "AABB_",							&ScnModelComponent::AABB_ ),
		ReField( "PerComponentMeshDataList_",	&ScnModelComponent::PerComponentMeshDataList_ ),
	};
		
	ReRegisterClass< ScnModelComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2030 ) );
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnModelComponent::initialise( const Json::Value& Object, ScnModelRef Parent )
{
	Super::initialise( Object );

	// Cache parent.
	Parent_ = Parent;
	Layer_ = 0;
	
	// Duplicate node data for update/rendering.
	BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	pNodeTransformData_ = new ScnModelNodeTransformData[ NoofNodes ];
	BcMemCopy( pNodeTransformData_, Parent_->pNodeTransformData_, sizeof( ScnModelNodeTransformData ) * NoofNodes );

	// Create material instances to render with.
	ScnModelMeshRuntimeList& MeshRuntimes = Parent_->MeshRuntimes_;
	ScnMaterialComponentRef MaterialComponentRef;
	PerComponentMeshDataList_.reserve( MeshRuntimes.size() );
	for( BcU32 Idx = 0; Idx < MeshRuntimes.size(); ++Idx )
	{
		ScnModelMeshData* pMeshData = &Parent_->pMeshData_[ Idx ];
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ Idx ];
		TPerComponentMeshData ComponentData;

		if( pMeshRuntime->MaterialRef_.isValid() )
		{
			BcAssert( pMeshRuntime->MaterialRef_.isValid() && pMeshRuntime->MaterialRef_->isReady() );

			ScnShaderPermutationFlags ShaderPermutation = pMeshData->ShaderPermutation_;

			// Setup lighting.
			if( isLit() )
			{
				ShaderPermutation |= ScnShaderPermutationFlags::LIGHTING_DIFFUSE;
			}
			else
			{
				ShaderPermutation |= ScnShaderPermutationFlags::LIGHTING_NONE;
			}
						
			// Even on failure add. List must be of same size for quick lookups.
			CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), MaterialComponentRef, pMeshRuntime->MaterialRef_, ShaderPermutation );

			ComponentData.MaterialComponentRef_ = MaterialComponentRef;
		}

		// Create uniform buffer for object.
		if( pMeshData->IsSkinned_ )
		{
			ComponentData.UniformBuffer_ = RsCore::pImpl() ? 
				RsCore::pImpl()->createBuffer( 
					RsBufferDesc( 
						RsBufferType::UNIFORM,
						RsResourceCreationFlags::STREAM,
						ScnShaderBoneUniformBlockData::StaticGetClass()->getSize() ) ) : nullptr;
		}
		else
		{
			ComponentData.UniformBuffer_ = RsCore::pImpl() ? 
				RsCore::pImpl()->createBuffer( 
					RsBufferDesc( 
						RsBufferType::UNIFORM,
						RsResourceCreationFlags::STREAM,
						ScnShaderObjectUniformBlockData::StaticGetClass()->getSize() ) ) : nullptr;
		}

		//
		PerComponentMeshDataList_.push_back( ComponentData );

	}
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnModelComponent::initialise( const Json::Value& Object )
{
	ScnModelRef ModelRef;
	ModelRef = getPackage()->getPackageCrossRef( Object[ "model" ].asUInt() );
	initialise( Object, ModelRef );

	// Setup additional stuff.
	Layer_ = Object.get( "layer", 0 ).asUInt();
	Pass_ = Object.get( "pass", 0 ).asUInt();

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnModelComponent::destroy()
{
	for( BcU32 Idx = 0; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		RsCore::pImpl()->destroyResource( PerComponentMeshDataList_[ Idx ].UniformBuffer_ );
	}
	
	// Delete duplicated node data.
	delete [] pNodeTransformData_;
	pNodeTransformData_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// getAABB
//virtual
MaAABB ScnModelComponent::getAABB() const
{
	UpdateFence_.wait();

	return AABB_;
}

//////////////////////////////////////////////////////////////////////////
// findNodeIndexByName
BcU32 ScnModelComponent::findNodeIndexByName( const BcName& Name ) const
{
	const BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	const ScnModelNodePropertyData* pNodePropertyData = Parent_->pNodePropertyData_;
	for( BcU32 NodeIdx = 0; NodeIdx < NoofNodes; ++NodeIdx )
	{
		if( pNodePropertyData[ NodeIdx ].Name_ == Name )
		{
			return NodeIdx;
		}
	}

	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setNode
void ScnModelComponent::setNode( BcU32 NodeIdx, const MaMat4d& LocalTransform )
{
	const BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	if( NodeIdx < NoofNodes )
	{
		pNodeTransformData_[ NodeIdx ].LocalTransform_ = LocalTransform;
	}
}

//////////////////////////////////////////////////////////////////////////
// getNode
const MaMat4d& ScnModelComponent::getNode( BcU32 NodeIdx ) const
{
	const BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	if( NodeIdx < NoofNodes )
	{
		return pNodeTransformData_[ NodeIdx ].LocalTransform_;
	}

	static MaMat4d Default;
	return Default;
}

//////////////////////////////////////////////////////////////////////////
// getNoofNodes
BcU32 ScnModelComponent::getNoofNodes() const
{
	const BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	return NoofNodes;
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponent
ScnMaterialComponentRef ScnModelComponent::getMaterialComponent( BcU32 Index )
{
	if( Index < PerComponentMeshDataList_.size() )
	{
		return PerComponentMeshDataList_[ Index ].MaterialComponentRef_;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponent
ScnMaterialComponentRef ScnModelComponent::getMaterialComponent( const BcName& MaterialName )
{
	ScnModelMeshData* pMeshData = Parent_->pMeshData_;

	for( BcU32 Idx = 0; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		if( MaterialName == PerComponentMeshDataList_[ Idx ].MaterialComponentRef_->getName() )
		{
			return PerComponentMeshDataList_[ Idx ].MaterialComponentRef_;
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnModelComponent::postUpdate( BcF32 Tick )
{
	Super::postUpdate( Tick );

	UpdateFence_.increment();
#if 0
	// TODO: Break out into it's own job class.
	typedef BcDelegate< void(*)( MaMat4d ) > UpdateNodeDelegate;
	UpdateNodeDelegate Delegate = UpdateNodeDelegate::bind< ScnModelComponent, &ScnModelComponent::updateNodes >( this );
	SysKernel::pImpl()->pushDelegateJob( SysKernel::DEFAULT_JOB_QUEUE_ID, Delegate, getParentEntity()->getWorldMatrix() );
#else
	updateNodes( getParentEntity()->getWorldMatrix() );
#endif
}

//////////////////////////////////////////////////////////////////////////
// updateNodes
void ScnModelComponent::updateNodes( MaMat4d RootMatrix )
{
	MaAABB FullAABB;

	// Wait for previous upload to finish.
	UploadFence_.wait();

	// Update nodes.	
	BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	for( BcU32 NodeIdx = 0; NodeIdx < NoofNodes; ++NodeIdx )
	{
		ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ NodeIdx ];
		ScnModelNodePropertyData* pNodePropertyData = &Parent_->pNodePropertyData_[ NodeIdx ];

		// Check parent index and process.
		if( pNodePropertyData->ParentIndex_ != BcErrorCode )
		{
			ScnModelNodeTransformData* pParentScnModelNodeTransformData = &pNodeTransformData_[ pNodePropertyData->ParentIndex_ ];
			
			pNodeTransformData->WorldTransform_ = pNodeTransformData->LocalTransform_ * pParentScnModelNodeTransformData->WorldTransform_;
		}
		else
		{
			pNodeTransformData->WorldTransform_ = pNodeTransformData->LocalTransform_ * RootMatrix;
		}
	}

	// Calculate bounds.
	BcU32 NoofPrimitives = Parent_->pHeader_->NoofPrimitives_;
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < NoofPrimitives; ++PrimitiveIdx )
	{
		ScnModelMeshRuntime* pNodeMeshRuntime = &Parent_->MeshRuntimes_[ PrimitiveIdx ];
		ScnModelMeshData* pNodeMeshData = &Parent_->pMeshData_[ pNodeMeshRuntime->MeshDataIndex_ ];

		// Special case the skinned models for now.
		if( pNodeMeshData->IsSkinned_ == BcFalse )
		{
			ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pNodeMeshData->NodeIndex_ ];
		
			MaAABB PrimitiveAABB = pNodeMeshData->AABB_;
			FullAABB.expandBy( PrimitiveAABB.transform( pNodeTransformData->WorldTransform_ ) );
		}
		else
		{
			MaAABB SkeletalAABB;
			for( BcU32 Idx = 0; Idx < SCN_MODEL_BONE_PALETTE_SIZE; ++Idx )
			{
				BcU32 BoneIndex = pNodeMeshData->BonePalette_[ Idx ];
				if( BoneIndex != BcErrorCode )
				{
					// Get the distance from the parent bone, and make an AABB that size.
					ScnModelNodePropertyData* pNodePropertyData = &Parent_->pNodePropertyData_[ BoneIndex ];
					if( pNodePropertyData->ParentIndex_ != BcErrorCode && pNodePropertyData->IsBone_ )
					{
						ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ BoneIndex ];
						ScnModelNodeTransformData* pParentNodeTransformData = &pNodeTransformData_[ pNodePropertyData->ParentIndex_ ];
						MaAABB NewAABB( pNodeTransformData->WorldTransform_.translation(), pParentNodeTransformData->WorldTransform_.translation() );

						//
						SkeletalAABB.expandBy( NewAABB );
					}
				}
			}

			// HACK: Expand AABB slightly to cover skin. Should calculate bone sizes and pack them really.
			MaVec3d Centre = SkeletalAABB.centre();
			MaVec3d Dimensions = SkeletalAABB.dimensions() * 0.75f;	// 1.5 x size.
			SkeletalAABB.min( Centre - Dimensions );
			SkeletalAABB.max( Centre + Dimensions );

			//
			FullAABB.expandBy( SkeletalAABB );
		}
	}

	AABB_ = FullAABB;

	// Setup skinning buffers.
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < NoofPrimitives; ++PrimitiveIdx )
	{
		ScnModelMeshRuntime* pNodeMeshRuntime = &Parent_->MeshRuntimes_[ PrimitiveIdx ];
		ScnModelMeshData* pNodeMeshData = &Parent_->pMeshData_[ pNodeMeshRuntime->MeshDataIndex_ ];
		TPerComponentMeshData& PerComponentMeshData = PerComponentMeshDataList_[ PrimitiveIdx ];

		UploadFence_.increment();

		if( pNodeMeshData->IsSkinned_ )
		{
			RsCore::pImpl()->updateBuffer( 
				PerComponentMeshData.UniformBuffer_,
				0, sizeof( ScnShaderBoneUniformBlockData ),
				RsResourceUpdateFlags::ASYNC,
				[ & ]( RsBuffer* Buffer, const RsBufferLock& Lock )
				{
					ScnShaderBoneUniformBlockData* BoneUniformBlock = reinterpret_cast< ScnShaderBoneUniformBlockData* >( Lock.Buffer_ );
					for( BcU32 Idx = 0; Idx < SCN_MODEL_BONE_PALETTE_SIZE; ++Idx )
					{
						BcU32 NodeIndex = pNodeMeshData->BonePalette_[ Idx ];
						if( NodeIndex != BcErrorCode )
						{
							BoneUniformBlock->BoneTransform_[ Idx ] = pNodeMeshData->BoneInverseBindpose_[ Idx ] * pNodeTransformData_[ NodeIndex ].WorldTransform_;
						}
					}
					UploadFence_.decrement();
				} );
		}
		else
		{
			RsCore::pImpl()->updateBuffer( 
				PerComponentMeshData.UniformBuffer_,
				0, sizeof( ScnShaderBoneUniformBlockData ),
				RsResourceUpdateFlags::ASYNC,
				[ & ]( RsBuffer* Buffer, const RsBufferLock& Lock )
				{
					ScnShaderObjectUniformBlockData* ObjectUniformBlock = reinterpret_cast< ScnShaderObjectUniformBlockData* >( Lock.Buffer_ );
					ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pNodeMeshData->NodeIndex_ ];
					ObjectUniformBlock->WorldTransform_ = pNodeTransformData->WorldTransform_;
					UploadFence_.decrement();
				} );
		}
	}

	UpdateFence_.decrement();
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnModelComponent::onAttach( ScnEntityWeakRef Parent )
{
	// Attach material components to parent.
	for( BcU32 Idx = 0 ; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		auto& PerComponentMeshData( PerComponentMeshDataList_[ Idx ] );
		Parent->attach( PerComponentMeshData.MaterialComponentRef_ );
	}
	
	//
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnModelComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Wait for update to complete.
	UpdateFence_.wait();

	// Detach material components from parent.
	for( BcU32 Idx = 0 ; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		auto& PerComponentMeshData( PerComponentMeshDataList_[ Idx ] );
		Parent->detach( PerComponentMeshData.MaterialComponentRef_ );
		PerComponentMeshData.MaterialComponentRef_ = NULL;
	}

	//
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// renderPrimitives
class ScnModelComponentRenderNode: public RsRenderNode
{
public:
	void render()
	{
		PSY_PROFILER_SECTION( RenderRoot, "ScnModelComponentRenderNode::render" );
		pContext_->setIndexBuffer( IndexBuffer_ );
		pContext_->setVertexBuffer( 0, VertexBuffer_, VertexStride_ );
		pContext_->setVertexDeclaration( VertexDeclaration_ );
		pContext_->drawIndexedPrimitives( Type_, Offset_, NoofIndices_, 0 );
	}

	RsTopologyType Type_;
	BcU32 Offset_;
	BcU32 NoofIndices_;
	RsBuffer* IndexBuffer_;
	RsBuffer* VertexBuffer_;
	BcU32 VertexStride_;
	RsVertexDeclaration* VertexDeclaration_;
};

void ScnModelComponent::render( class ScnViewComponent* pViewComponent, RsFrame* pFrame, RsRenderSort Sort )
{
	PSY_PROFILER_SECTION( RenderRoot, std::string( "ScnModelComponent::render (" ) + *getName() + std::string( ")" ) );

	Super::render( pViewComponent, pFrame, Sort );

	// Wait for model to have updated.
	UpdateFence_.wait();

	// Gather lights.
	ScnLightingVisitor LightingVisitor( this );

	ScnModelMeshRuntimeList& MeshRuntimes = Parent_->MeshRuntimes_;
	ScnModelMeshData* pMeshDatas = Parent_->pMeshData_;

	// Set layer.
	Sort.Layer_ = Layer_;
	Sort.Pass_ = Pass_;

	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < MeshRuntimes.size(); ++PrimitiveIdx )
	{
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ PrimitiveIdx ];
		ScnModelMeshData* pMeshData = &pMeshDatas[ pMeshRuntime->MeshDataIndex_ ];
		ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pMeshData->NodeIndex_ ];
		TPerComponentMeshData& PerComponentMeshData = PerComponentMeshDataList_[ PrimitiveIdx ];
		BcU32 Offset = 0; // This will change when index buffers are merged.

		BcAssertMsg( PerComponentMeshData.MaterialComponentRef_.isValid(), "Material not valid for use on ScnModelComponent \"%s\"", (*getName()).c_str() );

		// Set skinning parameters.
		if( pMeshData->IsSkinned_ )
		{
			PerComponentMeshData.MaterialComponentRef_->setBoneUniformBlock( PerComponentMeshData.UniformBuffer_ );
		}
		else
		{
			PerComponentMeshData.MaterialComponentRef_->setObjectUniformBlock( PerComponentMeshData.UniformBuffer_ );
		}

		// Set lighting parameters.
		LightingVisitor.setMaterialParameters( PerComponentMeshData.MaterialComponentRef_ );
			
		// Set material components for view.
		pViewComponent->setMaterialParameters( PerComponentMeshData.MaterialComponentRef_ );
			
		// Bind material.
		PerComponentMeshData.MaterialComponentRef_->bind( pFrame, Sort );
			
		// Render primitive.
		ScnModelComponentRenderNode* pRenderNode = pFrame->newObject< ScnModelComponentRenderNode >();
			
		pRenderNode->Type_ = pMeshData->Type_;
		pRenderNode->Offset_ = Offset;
		pRenderNode->NoofIndices_ = pMeshData->NoofIndices_;
		pRenderNode->IndexBuffer_ = pMeshRuntime->pIndexBuffer_;
		pRenderNode->VertexBuffer_ = pMeshRuntime->pVertexBuffer_;
		pRenderNode->VertexStride_ = pMeshData->VertexStride_;
		pRenderNode->VertexDeclaration_ = pMeshRuntime->pVertexDeclaration_;
		pRenderNode->Sort_ = Sort;
			
		pFrame->addRenderNode( pRenderNode );
	}
}
