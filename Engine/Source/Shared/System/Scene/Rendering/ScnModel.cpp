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

#include "System/Scene/Rendering/ScnModel.h"
#include "System/Scene/ScnEntity.h"
#include "System/Scene/Rendering/ScnViewComponent.h"

#include "System/Scene/Rendering/ScnLightingVisitor.h"

#include "System/Content/CsCore.h"
#include "System/SysKernel.h"

#include "Base/BcProfiler.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnModelImport.h"
#endif

#define DEBUG_RENDER_NODES ( 0 )

#if DEBUG_RENDER_NODES
#include "System/Scene/Rendering/ScnDebugRenderComponent.h"
#endif // DEBUG_RENDER_NODES

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnModel );

void ScnModel::StaticRegisterClass()
{
	{
		ReField* Fields[] = 
		{
			new ReField( "pHeader_", &ScnModel::pHeader_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
			new ReField( "pNodeTransformData_", &ScnModel::pNodeTransformData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
			new ReField( "pNodePropertyData_", &ScnModel::pNodePropertyData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
			new ReField( "pVertexBufferData_", &ScnModel::pVertexBufferData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
			new ReField( "pIndexBufferData_", &ScnModel::pIndexBufferData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA),
			new ReField( "pVertexElements_", &ScnModel::pVertexElements_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
			new ReField( "pMeshData_", &ScnModel::pMeshData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
			new ReField( "MeshRuntimes_", &ScnModel::MeshRuntimes_, bcRFF_TRANSIENT ),
		};
		
		auto& Class = ReRegisterClass< ScnModel, Super >( Fields );
		BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnModelImport::StaticGetClass(), 0 ) );
#endif
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModel::ScnModel():
	pHeader_( nullptr ),
	pNodeTransformData_( nullptr ),
	pNodePropertyData_( nullptr ),
	pVertexBufferData_( nullptr ),
	pIndexBufferData_( nullptr ),
	pMeshData_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnModel::~ScnModel()
{
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
			nullptr
		};
		
		// Get resource.
		auto Resource = getPackage()->getCrossRefResource( pMeshData->MaterialRef_ );
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
		RsCore::pImpl()->destroyResource( MeshRuntime.pVertexDeclaration_ );
		
		MeshRuntime.MaterialRef_ = nullptr;
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
REFLECTION_DEFINE_DERIVED( ScnModelComponent );

void ScnModelComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Model_", &ScnModelComponent::Model_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "Layer_", &ScnModelComponent::Layer_, bcRFF_IMPORTER ),
		new ReField( "Pass_", &ScnModelComponent::Pass_, bcRFF_IMPORTER ),
		new ReField( "Position_", &ScnModelComponent::Position_, bcRFF_IMPORTER ),
		new ReField( "Scale_", &ScnModelComponent::Scale_, bcRFF_IMPORTER ),
		new ReField( "Rotation_", &ScnModelComponent::Rotation_, bcRFF_IMPORTER ),

		new ReField( "BaseTransform_", &ScnModelComponent::BaseTransform_ ),
		new ReField( "UploadFence_", &ScnModelComponent::UploadFence_, bcRFF_TRANSIENT ),
		new ReField( "UpdateFence_", &ScnModelComponent::UpdateFence_, bcRFF_TRANSIENT ),
		new ReField( "pNodeTransformData_", &ScnModelComponent::pNodeTransformData_, bcRFF_TRANSIENT ),
		new ReField( "AABB_", &ScnModelComponent::AABB_, bcRFF_TRANSIENT ),
		new ReField( "PerComponentMeshDataList_", &ScnModelComponent::PerComponentMeshDataList_, bcRFF_TRANSIENT ),
	};
		
	ReRegisterClass< ScnModelComponent, Super >( Fields )
		.addAttribute( new ScnComponentAttribute( -2030 ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelComponent::ScnModelComponent():
	Model_(),
	Layer_( 0 ),
	Pass_( 0 ),
	Position_( 0.0f, 0.0f, 0.0f ),
	Scale_( 1.0f, 1.0f, 1.0f ),
	Rotation_( 0.0f, 0.0f, 0.0f ),
	pNodeTransformData_( nullptr ),
	UploadFence_(),
	UpdateFence_(),
	AABB_(),
	PerComponentMeshDataList_()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnModelComponent::~ScnModelComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnModelComponent::initialise()
{
	setBaseTransform( Position_, Scale_, Rotation_ );
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
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	const ScnModelNodePropertyData* pNodePropertyData = Model_->pNodePropertyData_;
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
// findNodeNameByIndex
const BcName& ScnModelComponent::findNodeNameByIndex( BcU32 NodeIdx ) const
{
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	const ScnModelNodePropertyData* pNodePropertyData = Model_->pNodePropertyData_;
	if( NodeIdx < NoofNodes )
	{
		return pNodePropertyData[ NodeIdx ].Name_;
	}

	return BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// setNode
void ScnModelComponent::setNode( BcU32 NodeIdx, const MaMat4d& LocalTransform )
{
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	if( NodeIdx < NoofNodes )
	{
		pNodeTransformData_[ NodeIdx ].LocalTransform_ = LocalTransform;
	}
}

//////////////////////////////////////////////////////////////////////////
// getNode
const MaMat4d& ScnModelComponent::getNode( BcU32 NodeIdx ) const
{
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
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
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
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
	
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponent
ScnMaterialComponentRef ScnModelComponent::getMaterialComponent( const BcName& MaterialName )
{
	for( BcU32 Idx = 0; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		if( MaterialName == PerComponentMeshDataList_[ Idx ].MaterialComponentRef_->getName() )
		{
			return PerComponentMeshDataList_[ Idx ].MaterialComponentRef_;
		}
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getMaterialComponents
ScnMaterialComponentList ScnModelComponent::getMaterialComponents( const BcName& MaterialName )
{
	ScnMaterialComponentList MaterialComponents;

	// Pessemistic reserve to ensure only 1 allocation.
	MaterialComponents.reserve( PerComponentMeshDataList_.size() );

	// Find all.
	for( BcU32 Idx = 0; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		if( MaterialName == PerComponentMeshDataList_[ Idx ].MaterialComponentRef_->getName() )
		{
			MaterialComponents.push_back( PerComponentMeshDataList_[ Idx ].MaterialComponentRef_ );
		}
	}

	return std::move( MaterialComponents );
}


//////////////////////////////////////////////////////////////////////////
// setBaseTransform
void ScnModelComponent::setBaseTransform( const MaVec3d& Position, const MaVec3d& Scale, const MaVec3d& Rotation )
{
	Position_ = Position;
	Scale_ = Scale;
	Rotation_ = Rotation;

	// Setup base transform.
	MaMat4d ScaleMatrix;
	ScaleMatrix.scale( Scale_ );
	BaseTransform_.identity();
	BaseTransform_.rotation( Rotation_ );
	BaseTransform_ = BaseTransform_ * ScaleMatrix;
	BaseTransform_.translation( Position_ );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnModelComponent::postUpdate( BcF32 Tick )
{
	PSY_PROFILE_FUNCTION;
	Super::postUpdate( Tick );

	UploadFence_.wait();
	UpdateFence_.increment();
	MaMat4d Matrix = BaseTransform_ * getParentEntity()->getWorldMatrix();
	SysKernel::pImpl()->pushFunctionJob( SysKernel::DEFAULT_JOB_QUEUE_ID,
		[ this, Matrix ]()->void
		{
			updateNodes( Matrix );
		} );

#if DEBUG_RENDER_NODES
	BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	for( BcU32 NodeIdx = 0; NodeIdx < NoofNodes; ++NodeIdx )
	{
		ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ NodeIdx ];
		ScnModelNodePropertyData* pNodePropertyData = &Model_->pNodePropertyData_[ NodeIdx ];

		MaMat4d ThisMatrix = pNodeTransformData->WorldTransform_;
		MaMat4d ParentMatrix = pNodePropertyData->ParentIndex_ != BcErrorCode ? 
			pNodeTransformData_[ pNodePropertyData->ParentIndex_ ].WorldTransform_ :
			getParentEntity()->getWorldMatrix();

		ScnDebugRenderComponent::pImpl()->drawMatrix( 
			ThisMatrix, RsColour::WHITE, 2000 );
		ScnDebugRenderComponent::pImpl()->drawLine( 
			ParentMatrix.translation(), 
			ThisMatrix.translation(), 
			RsColour::WHITE, 1000 );
	}

#endif // DEBUG_RENDER_NODES
}

//////////////////////////////////////////////////////////////////////////
// updateNodes
void ScnModelComponent::updateNodes( MaMat4d RootMatrix )
{
	PSY_PROFILE_FUNCTION;

	MaAABB FullAABB;

	// Update nodes.	
	BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	for( BcU32 NodeIdx = 0; NodeIdx < NoofNodes; ++NodeIdx )
	{
		ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ NodeIdx ];
		ScnModelNodePropertyData* pNodePropertyData = &Model_->pNodePropertyData_[ NodeIdx ];

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
	BcU32 NoofPrimitives = Model_->pHeader_->NoofPrimitives_;
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < NoofPrimitives; ++PrimitiveIdx )
	{
		ScnModelMeshRuntime* pNodeMeshRuntime = &Model_->MeshRuntimes_[ PrimitiveIdx ];
		ScnModelMeshData* pNodeMeshData = &Model_->pMeshData_[ pNodeMeshRuntime->MeshDataIndex_ ];

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
					ScnModelNodePropertyData* pNodePropertyData = &Model_->pNodePropertyData_[ BoneIndex ];
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
		ScnModelMeshRuntime* pNodeMeshRuntime = &Model_->MeshRuntimes_[ PrimitiveIdx ];
		ScnModelMeshData* pNodeMeshData = &Model_->pMeshData_[ pNodeMeshRuntime->MeshDataIndex_ ];
		TPerComponentMeshData& PerComponentMeshData = PerComponentMeshDataList_[ PrimitiveIdx ];

		UploadFence_.increment();

		if( pNodeMeshData->IsSkinned_ )
		{
			RsCore::pImpl()->updateBuffer( 
				PerComponentMeshData.UniformBuffer_,
				0, sizeof( ScnShaderBoneUniformBlockData ),
				RsResourceUpdateFlags::ASYNC,
				[ this, pNodeMeshData ]( RsBuffer* Buffer, const RsBufferLock& Lock )
				{
					PSY_PROFILE_FUNCTION;
					ScnShaderBoneUniformBlockData* BoneUniformBlock = reinterpret_cast< ScnShaderBoneUniformBlockData* >( Lock.Buffer_ );
					for( BcU32 Idx = 0; Idx < SCN_MODEL_BONE_PALETTE_SIZE; ++Idx )
					{
						BcU32 NodeIndex = pNodeMeshData->BonePalette_[ Idx ];
						if( NodeIndex != BcErrorCode )
						{
							BoneUniformBlock->BoneTransform_[ Idx ] =
								pNodeMeshData->BoneInverseBindpose_[ Idx ] * 
								pNodeTransformData_[ NodeIndex ].WorldTransform_;
						}
					}
					UploadFence_.decrement();
				} );
		}
		else
		{
			RsCore::pImpl()->updateBuffer( 
				PerComponentMeshData.UniformBuffer_,
				0, sizeof( ScnShaderObjectUniformBlockData ),
				RsResourceUpdateFlags::ASYNC,
				[ this, pNodeMeshData ]( RsBuffer* Buffer, const RsBufferLock& Lock )
				{
					PSY_PROFILE_FUNCTION;
					ScnShaderObjectUniformBlockData* ObjectUniformBlock = reinterpret_cast< ScnShaderObjectUniformBlockData* >( Lock.Buffer_ );
					ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pNodeMeshData->NodeIndex_ ];

					// World matrix.
					ObjectUniformBlock->WorldTransform_ = pNodeTransformData->WorldTransform_;

					// Normal matrix.
					ObjectUniformBlock->NormalTransform_ = pNodeTransformData->WorldTransform_;
					
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
	Super::onAttach( Parent );

	// Duplicate node data for update/rendering.
	BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	pNodeTransformData_ = new ScnModelNodeTransformData[ NoofNodes ];
	BcMemCopy( pNodeTransformData_, Model_->pNodeTransformData_, sizeof( ScnModelNodeTransformData ) * NoofNodes );

	// Create material instances to render with.
	ScnModelMeshRuntimeList& MeshRuntimes = Model_->MeshRuntimes_;
	ScnMaterialComponentRef MaterialComponentRef;
	PerComponentMeshDataList_.reserve( MeshRuntimes.size() );
	for( BcU32 Idx = 0; Idx < MeshRuntimes.size(); ++Idx )
	{
		ScnModelMeshData* pMeshData = &Model_->pMeshData_[ Idx ];
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
			ComponentData.MaterialComponentRef_ = Parent->attach< ScnMaterialComponent >( 
				BcName::INVALID, pMeshRuntime->MaterialRef_, ShaderPermutation );
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

	// Update nodes.
	UpdateFence_.increment();
	updateNodes( BaseTransform_ * getParentEntity()->getWorldMatrix() );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnModelComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Wait for update + upload to complete.
	UpdateFence_.wait();
	UploadFence_.wait();
	//SysKernel::pImpl()->flushJobQueue( RsCore::JOB_QUEUE_ID );

	// Detach material components from parent.
	for( BcU32 Idx = 0 ; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		auto& PerComponentMeshData( PerComponentMeshDataList_[ Idx ] );
		Parent->detach( PerComponentMeshData.MaterialComponentRef_ );
		PerComponentMeshData.MaterialComponentRef_ = nullptr;
	}

	// Destroy resources.
	for( BcU32 Idx = 0; Idx < PerComponentMeshDataList_.size(); ++Idx )
	{
		RsCore::pImpl()->destroyResource( PerComponentMeshDataList_[ Idx ].UniformBuffer_ );
	}
	
	// Delete duplicated node data.
	delete [] pNodeTransformData_;
	pNodeTransformData_ = nullptr;

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
	PSY_PROFILE_FUNCTION;

	Super::render( pViewComponent, pFrame, Sort );

	// Wait for model to have updated.
	UpdateFence_.wait();

#if 0
	// Gather lights.
	ScnLightingVisitor LightingVisitor( this );
#endif

	ScnModelMeshRuntimeList& MeshRuntimes = Model_->MeshRuntimes_;
	ScnModelMeshData* pMeshDatas = Model_->pMeshData_;

	// Set layer.
	Sort.Layer_ = Layer_;
	Sort.Pass_ = Pass_;

	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < MeshRuntimes.size(); ++PrimitiveIdx )
	{
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ PrimitiveIdx ];
		ScnModelMeshData* pMeshData = &pMeshDatas[ pMeshRuntime->MeshDataIndex_ ];
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

#if 0
		// Set lighting parameters.
		LightingVisitor.setMaterialParameters( PerComponentMeshData.MaterialComponentRef_ );
#endif		
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
