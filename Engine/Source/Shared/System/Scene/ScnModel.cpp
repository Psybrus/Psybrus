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
	pPrimitiveData_ = NULL;
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
	PrimitiveRuntimes_.reserve( pHeader_->NoofPrimitives_ );
	
	BcU8* pVertexBufferData = pVertexBufferData_;
	BcU8* pIndexBufferData = pIndexBufferData_;
	
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < pHeader_->NoofPrimitives_; ++PrimitiveIdx )
	{
		ScnModelPrimitiveData* pPrimitiveData = &pPrimitiveData_[ PrimitiveIdx ];
		
		//ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pPrimitiveData->NodeIndex_ ];
		
		// Create GPU resources.
		RsVertexDeclarationDesc VertexDeclarationDesc( pPrimitiveData_->NoofVertexElements_ );
		for( BcU32 Idx = 0; Idx < pPrimitiveData_->NoofVertexElements_; ++Idx )
		{
			VertexDeclarationDesc.addElement( pPrimitiveData_->VertexElements_[ Idx ] );
		}
		
		RsVertexDeclaration* pVertexDeclaration = RsCore::pImpl()->createVertexDeclaration( VertexDeclarationDesc );
		RsVertexBuffer* pVertexBuffer = RsCore::pImpl()->createVertexBuffer( RsVertexBufferDesc( pPrimitiveData->NoofVertices_, pPrimitiveData->VertexStride_ ), pVertexBufferData );
		RsIndexBuffer* pIndexBuffer = RsCore::pImpl()->createIndexBuffer( RsIndexBufferDesc( pPrimitiveData_->NoofIndices_ ), pIndexBufferData );
		
		// Setup runtime structure.
		ScnModelPrimitiveRuntime PrimitiveRuntime = 
		{
			PrimitiveIdx,
			pVertexDeclaration,
			pVertexBuffer,
			pIndexBuffer,
			NULL
		};
		
		// Get resource.
		auto Resource = getPackage()->getPackageCrossRef( pPrimitiveData->MaterialRef_ );
		PrimitiveRuntime.MaterialRef_ = Resource;
		BcAssertMsg( PrimitiveRuntime.MaterialRef_.isValid(), "ScnModel: Material reference is invalid. Packing error." );

		// Push into array.
		PrimitiveRuntimes_.push_back( PrimitiveRuntime );
		
		// Advance vertex and index buffers.
		pVertexBufferData += pPrimitiveData->NoofVertices_ * PrimitiveRuntime.pVertexBuffer_->getVertexStride();
		pIndexBufferData += pPrimitiveData->NoofIndices_ * sizeof( BcU16 );
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
	for( BcU32 Idx = 0; Idx < PrimitiveRuntimes_.size(); ++Idx )
	{
		ScnModelPrimitiveRuntime& PrimitiveRuntime( PrimitiveRuntimes_[ Idx ] );

		RsCore::pImpl()->destroyResource( PrimitiveRuntime.pVertexBuffer_ );
		RsCore::pImpl()->destroyResource( PrimitiveRuntime.pIndexBuffer_ );
		
		PrimitiveRuntime.MaterialRef_ = NULL;
	}

	PrimitiveRuntimes_.clear();
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
	else if( ChunkID == BcHash( "primitivedata" ) )
	{
		pPrimitiveData_ = (ScnModelPrimitiveData*)pData;

		RsVertexElement* pVertexElements = pVertexElements_;
		for( BcU32 Idx = 0; Idx < pHeader_->NoofPrimitives_; ++Idx )
		{
			pPrimitiveData_->VertexElements_ = pVertexElements;
			pVertexElements += pPrimitiveData_->NoofVertexElements_;
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
		ReField( "PerComponentPrimitiveDataList_",	&ScnModelComponent::PerComponentPrimitiveDataList_ ),
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
	ScnModelPrimitiveRuntimeList& PrimitiveRuntimes = Parent_->PrimitiveRuntimes_;
	ScnMaterialComponentRef MaterialComponentRef;
	PerComponentPrimitiveDataList_.reserve( PrimitiveRuntimes.size() );
	for( BcU32 Idx = 0; Idx < PrimitiveRuntimes.size(); ++Idx )
	{
		ScnModelPrimitiveData* pPrimitiveData = &Parent_->pPrimitiveData_[ Idx ];
		ScnModelPrimitiveRuntime* pPrimitiveRuntime = &PrimitiveRuntimes[ Idx ];
		TPerComponentPrimitiveData ComponentData;

		if( pPrimitiveRuntime->MaterialRef_.isValid() )
		{
			BcAssert( pPrimitiveRuntime->MaterialRef_.isValid() && pPrimitiveRuntime->MaterialRef_->isReady() );

			BcU32 ShaderPermutation = pPrimitiveData->ShaderPermutation_;

			// Setup lighting.
			if( isLit() )
			{
				ShaderPermutation |= scnSPF_LIGHTING_DIFFUSE;
			}
			else
			{
				ShaderPermutation |= scnSPF_LIGHTING_NONE;
			}
						
			// Even on failure add. List must be of same size for quick lookups.
			CsCore::pImpl()->createResource( BcName::INVALID, getPackage(), MaterialComponentRef, pPrimitiveRuntime->MaterialRef_, ShaderPermutation );

			ComponentData.MaterialComponentRef_ = MaterialComponentRef;
		}

		// Create uniform buffer for object.
		if( pPrimitiveData->IsSkinned_ )
		{
			ComponentData.UniformBuffer_ = RsCore::pImpl() ? RsCore::pImpl()->createUniformBuffer( sizeof( ScnShaderBoneUniformBlockData ) , nullptr ) : nullptr;
		}
		else
		{
			ComponentData.UniformBuffer_ = RsCore::pImpl() ? RsCore::pImpl()->createUniformBuffer( sizeof( ScnShaderObjectUniformBlockData ), nullptr ) : nullptr;
		}

		//
		PerComponentPrimitiveDataList_.push_back( ComponentData );

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
	for( BcU32 Idx = 0; Idx < PerComponentPrimitiveDataList_.size(); ++Idx )
	{
		RsCore::pImpl()->destroyResource( PerComponentPrimitiveDataList_[ Idx ].UniformBuffer_ );
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
		pNodeTransformData_[ NodeIdx ].RelativeTransform_ = LocalTransform;
	}
}

//////////////////////////////////////////////////////////////////////////
// getNode
const MaMat4d& ScnModelComponent::getNode( BcU32 NodeIdx ) const
{
	const BcU32 NoofNodes = Parent_->pHeader_->NoofNodes_;
	if( NodeIdx < NoofNodes )
	{
		return pNodeTransformData_[ NodeIdx ].RelativeTransform_;
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
	if( Index < PerComponentPrimitiveDataList_.size() )
	{
		return PerComponentPrimitiveDataList_[ Index ].MaterialComponentRef_;
	}
	
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponent
ScnMaterialComponentRef ScnModelComponent::getMaterialComponent( const BcName& MaterialName )
{
	ScnModelPrimitiveData* pPrimitiveData = Parent_->pPrimitiveData_;

	for( BcU32 Idx = 0; Idx < PerComponentPrimitiveDataList_.size(); ++Idx )
	{
		if( MaterialName == PerComponentPrimitiveDataList_[ Idx ].MaterialComponentRef_->getName() )
		{
			return PerComponentPrimitiveDataList_[ Idx ].MaterialComponentRef_;
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
			
			pNodeTransformData->AbsoluteTransform_ = pNodeTransformData->RelativeTransform_ * pParentScnModelNodeTransformData->AbsoluteTransform_;
		}
		else
		{
			pNodeTransformData->AbsoluteTransform_ = pNodeTransformData->RelativeTransform_ * RootMatrix;
		}
	}

	// Calculate bounds.
	BcU32 NoofPrimitives = Parent_->pHeader_->NoofPrimitives_;
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < NoofPrimitives; ++PrimitiveIdx )
	{
		ScnModelPrimitiveRuntime* pNodePrimitiveRuntime = &Parent_->PrimitiveRuntimes_[ PrimitiveIdx ];
		ScnModelPrimitiveData* pNodePrimitiveData = &Parent_->pPrimitiveData_[ pNodePrimitiveRuntime->PrimitiveDataIndex_ ];

		// Special case the skinned models for now.
		if( pNodePrimitiveData->IsSkinned_ == BcFalse )
		{
			ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pNodePrimitiveData->NodeIndex_ ];
		
			MaAABB PrimitiveAABB = pNodePrimitiveData->AABB_;
			FullAABB.expandBy( PrimitiveAABB.transform( pNodeTransformData->AbsoluteTransform_ ) );
		}
		else
		{
			MaAABB SkeletalAABB;
			for( BcU32 Idx = 0; Idx < SCN_MODEL_BONE_PALETTE_SIZE; ++Idx )
			{
				BcU32 BoneIndex = pNodePrimitiveData->BonePalette_[ Idx ];
				if( BoneIndex != BcErrorCode )
				{
					// Get the distance from the parent bone, and make an AABB that size.
					ScnModelNodePropertyData* pNodePropertyData = &Parent_->pNodePropertyData_[ BoneIndex ];
					if( pNodePropertyData->ParentIndex_ != BcErrorCode && pNodePropertyData->IsBone_ )
					{
						ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ BoneIndex ];
						ScnModelNodeTransformData* pParentNodeTransformData = &pNodeTransformData_[ pNodePropertyData->ParentIndex_ ];
						MaAABB NewAABB( pNodeTransformData->AbsoluteTransform_.translation(), pParentNodeTransformData->AbsoluteTransform_.translation() );

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
		ScnModelPrimitiveRuntime* pNodePrimitiveRuntime = &Parent_->PrimitiveRuntimes_[ PrimitiveIdx ];
		ScnModelPrimitiveData* pNodePrimitiveData = &Parent_->pPrimitiveData_[ pNodePrimitiveRuntime->PrimitiveDataIndex_ ];
		TPerComponentPrimitiveData& PerComponentPrimitiveData = PerComponentPrimitiveDataList_[ PrimitiveIdx ];

		if( pNodePrimitiveData->IsSkinned_ )
		{
			BcAssertMsg( PerComponentPrimitiveData.UniformBuffer_->getDataSize() == sizeof( ScnShaderBoneUniformBlockData ), "BoneUniformBlock size mismatch." );
			ScnShaderBoneUniformBlockData* BoneUniformBlock = reinterpret_cast< ScnShaderBoneUniformBlockData* >( PerComponentPrimitiveData.UniformBuffer_->lock() );
			for( BcU32 Idx = 0; Idx < SCN_MODEL_BONE_PALETTE_SIZE; ++Idx )
			{
				BcU32 NodeIndex = pNodePrimitiveData->BonePalette_[ Idx ];
				if( NodeIndex != BcErrorCode )
				{
					BoneUniformBlock->BoneTransform_[ Idx ] = pNodeTransformData_[ NodeIndex ].InverseBindpose_ * pNodeTransformData_[ NodeIndex ].AbsoluteTransform_;
				}
			}

			PerComponentPrimitiveData.UniformBuffer_->unlock();		
		}
		else
		{
			BcAssertMsg( PerComponentPrimitiveData.UniformBuffer_->getDataSize() == sizeof( ScnShaderObjectUniformBlockData ), "ObjectUniformBlock size mismatch." );
			ScnShaderObjectUniformBlockData* ObjectUniformBlock = reinterpret_cast< ScnShaderObjectUniformBlockData* >( PerComponentPrimitiveData.UniformBuffer_->lock() );
			ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pNodePrimitiveData->NodeIndex_ ];
			ObjectUniformBlock->WorldTransform_ = pNodeTransformData->AbsoluteTransform_;
			PerComponentPrimitiveData.UniformBuffer_->unlock();		
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
	for( BcU32 Idx = 0 ; Idx < PerComponentPrimitiveDataList_.size(); ++Idx )
	{
		auto& PerComponentPrimitiveData( PerComponentPrimitiveDataList_[ Idx ] );
		Parent->attach( PerComponentPrimitiveData.MaterialComponentRef_ );
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
	for( BcU32 Idx = 0 ; Idx < PerComponentPrimitiveDataList_.size(); ++Idx )
	{
		auto& PerComponentPrimitiveData( PerComponentPrimitiveDataList_[ Idx ] );
		Parent->detach( PerComponentPrimitiveData.MaterialComponentRef_ );
		PerComponentPrimitiveData.MaterialComponentRef_ = NULL;
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
		pContext_->setVertexBuffer( 0, VertexBuffer_ );
		pContext_->setVertexDeclaration( VertexDeclaration_ );
		pContext_->drawIndexedPrimitives( Type_, Offset_, NoofIndices_, 0 );
	}

	RsTopologyType Type_;
	BcU32 Offset_;
	BcU32 NoofIndices_;
	RsIndexBuffer* IndexBuffer_;
	RsVertexBuffer* VertexBuffer_;
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

	ScnModelPrimitiveRuntimeList& PrimitiveRuntimes = Parent_->PrimitiveRuntimes_;
	ScnModelPrimitiveData* pPrimitiveDatas = Parent_->pPrimitiveData_;

	// Set layer.
	Sort.Layer_ = Layer_;
	Sort.Pass_ = Pass_;

	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < PrimitiveRuntimes.size(); ++PrimitiveIdx )
	{
		ScnModelPrimitiveRuntime* pPrimitiveRuntime = &PrimitiveRuntimes[ PrimitiveIdx ];
		ScnModelPrimitiveData* pPrimitiveData = &pPrimitiveDatas[ pPrimitiveRuntime->PrimitiveDataIndex_ ];
		ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pPrimitiveData->NodeIndex_ ];
		TPerComponentPrimitiveData& PerComponentPrimitiveData = PerComponentPrimitiveDataList_[ PrimitiveIdx ];
		BcU32 Offset = 0; // This will change when index buffers are merged.

		BcAssertMsg( PerComponentPrimitiveData.MaterialComponentRef_.isValid(), "Material not valid for use on ScnModelComponent \"%s\"", (*getName()).c_str() );

		// Set skinning parameters.
		if( pPrimitiveData->IsSkinned_ )
		{
			PerComponentPrimitiveData.MaterialComponentRef_->setBoneUniformBlock( PerComponentPrimitiveData.UniformBuffer_ );
		}
		else
		{
			PerComponentPrimitiveData.MaterialComponentRef_->setObjectUniformBlock( PerComponentPrimitiveData.UniformBuffer_ );
		}

		// Set lighting parameters.
		LightingVisitor.setMaterialParameters( PerComponentPrimitiveData.MaterialComponentRef_ );
			
		// Set material components for view.
		pViewComponent->setMaterialParameters( PerComponentPrimitiveData.MaterialComponentRef_ );
			
		// Bind material.
		PerComponentPrimitiveData.MaterialComponentRef_->bind( pFrame, Sort );
			
		// Render primitive.
		ScnModelComponentRenderNode* pRenderNode = pFrame->newObject< ScnModelComponentRenderNode >();
			
		pRenderNode->Type_ = pPrimitiveData->Type_;
		pRenderNode->Offset_ = Offset;
		pRenderNode->NoofIndices_ = pPrimitiveData->NoofIndices_;
		pRenderNode->IndexBuffer_ = pPrimitiveRuntime->pIndexBuffer_;
		pRenderNode->VertexBuffer_ = pPrimitiveRuntime->pVertexBuffer_;
		pRenderNode->VertexDeclaration_ = pPrimitiveRuntime->pVertexDeclaration_;
		pRenderNode->Sort_ = Sort;
			
		pFrame->addRenderNode( pRenderNode );
	}
}
