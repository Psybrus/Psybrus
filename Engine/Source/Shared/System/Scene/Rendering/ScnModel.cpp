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
#include "System/Scene/ScnComponentProcessor.h"
#include "System/Scene/ScnEntity.h"

#include "System/Scene/Rendering/ScnLightingVisitor.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsImGuiFieldEditor.h"

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
// ScnModelUniforms
REFLECTION_DEFINE_BASIC( ScnModelUniforms );

void ScnModelUniforms::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Name_", &ScnModelUniforms::Name_ ),
		new ReField( "Data_", &ScnModelUniforms::Data_ ),
	};
	ReRegisterClass< ScnModelUniforms >( Fields );
}

ScnModelUniforms::ScnModelUniforms():
	Name_( "" ),
	Data_(),
	Buffer_()
{

}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnModel );

void ScnModel::StaticRegisterClass()
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
		//TODO: move support. new ReField( "MeshRuntimes_", &ScnModel::MeshRuntimes_, bcRFF_TRANSIENT ),
	};
		
	auto& Class = ReRegisterClass< ScnModel, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnModelImport::StaticGetClass(), 0 ) );
#endif

	Class.addAttribute( 
		new DsImGuiFieldEditor( 
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				auto Model = static_cast< ScnModel* >( Object );
				if( ImGui::TreeNode( Model, "Materials") )
				{
					for( auto& MeshRuntime : Model->MeshRuntimes_ )
					{
						DsImGuiFieldEditor* FieldEditor = DsImGuiFieldEditor::Get( MeshRuntime.MaterialRef_->getClass() );
						if( FieldEditor )
						{
							FieldEditor->onEdit( MeshRuntime.MaterialRef_->getFullName(), MeshRuntime.MaterialRef_, ScnMaterial::StaticGetClass(),
								ReFieldFlags( Flags & bcRFF_CONST ) );
						}
					}
					ImGui::TreePop();
				}

				// Defaults.
				DsCore::pImpl()->drawObjectEditor( ThisFieldEditor, Object, Class, Flags );
			} ) );
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

	size_t VertexBufferSize = 0;
	size_t IndexBufferSize = 0;
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < pHeader_->NoofPrimitives_; ++PrimitiveIdx )
	{
		ScnModelMeshData* pMeshData = &pMeshData_[ PrimitiveIdx ];
		VertexBufferSize += pMeshData->NoofVertices_ * pMeshData->VertexStride_;
		IndexBufferSize += pMeshData->NoofIndices_ * sizeof( BcU16 );
	}

	// Create large VB + IBs.
	VertexBuffer_ = RsCore::pImpl()->createBuffer( 
		RsBufferDesc( 
			RsResourceBindFlags::VERTEX_BUFFER, 
			RsResourceCreationFlags::STATIC,
			VertexBufferSize ),
		getFullName().c_str() );

	RsCore::pImpl()->updateBuffer( 
		VertexBuffer_.get(), 0, VertexBufferSize, 
		RsResourceUpdateFlags::ASYNC,
		[ pVertexBufferData, VertexBufferSize ]
		( RsBuffer* Buffer, const RsBufferLock& BufferLock )
		{
			BcAssert( Buffer->getDesc().SizeBytes_ == VertexBufferSize );
			BcMemCopy( BufferLock.Buffer_, pVertexBufferData, 
				VertexBufferSize );
		} );

	IndexBuffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc( 
				RsResourceBindFlags::INDEX_BUFFER, 
				RsResourceCreationFlags::STATIC, 
				IndexBufferSize ),
			getFullName().c_str() );

	RsCore::pImpl()->updateBuffer( 
		IndexBuffer_.get(), 0, IndexBufferSize, 
		RsResourceUpdateFlags::ASYNC,
		[ pIndexBufferData, IndexBufferSize ]
		( RsBuffer* Buffer, const RsBufferLock& BufferLock )
		{
			BcAssert( Buffer->getDesc().SizeBytes_ == IndexBufferSize );
			BcMemCopy( BufferLock.Buffer_, pIndexBufferData, 
				IndexBufferSize );
		} );

	size_t VertexBufferOffset = 0;
	size_t IndexBufferOffset = 0;
	size_t BindVertexBufferBase = 0;
	size_t BindIndexBufferBase = 0;

	RsVertexDeclaration* PrevVertexDeclaration = nullptr;
	RsVertexDeclaration* VertexDeclaration = nullptr;
	RsGeometryBinding* GeometryBinding = nullptr;
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

		// Find or create vertex declaration.
		{
			for( const auto& CurrVertexDecl : VertexDeclarations_ )
			{
				if( CurrVertexDecl->getDesc().getHash() == VertexDeclarationDesc.getHash() )
				{
					VertexDeclaration = CurrVertexDecl.get();
					break;
				}
			}		
			if( VertexDeclaration == nullptr )
			{
				RsVertexDeclarationUPtr NewVertexDeclaration(
					RsCore::pImpl()->createVertexDeclaration( VertexDeclarationDesc,
						getFullName().c_str() ) );
				VertexDeclaration = NewVertexDeclaration.get();
				VertexDeclarations_.emplace_back( std::move( NewVertexDeclaration ) );
			}
		}

		// Create geometry binding if need be.
		if( VertexDeclaration != PrevVertexDeclaration )
		{
			PrevVertexDeclaration = VertexDeclaration;

			RsGeometryBindingDesc GeometryBindingDesc;
			GeometryBindingDesc.setIndexBuffer( IndexBuffer_.get(), 2, BindIndexBufferBase );
			GeometryBindingDesc.setVertexBuffer( 0, VertexBuffer_.get(), pMeshData->VertexStride_, BindVertexBufferBase );
			GeometryBindingDesc.setVertexDeclaration( VertexDeclaration );
			RsGeometryBindingUPtr NewGeometryBinding( 
				RsCore::pImpl()->createGeometryBinding( GeometryBindingDesc, getFullName().c_str() ) );
			GeometryBinding = NewGeometryBinding.get();
			GeometryBindings_.emplace_back( std::move( NewGeometryBinding ) );

			VertexBufferOffset = 0;
			IndexBufferOffset = 0;
		}

		BcAssert( GeometryBinding );

		// Setup runtime structure.
		ScnModelMeshRuntime MeshRuntime;
		MeshRuntime.MeshDataIndex_ = PrimitiveIdx;
		MeshRuntime.GeometryBinding_ = GeometryBinding;
		MeshRuntime.VertexBufferOffset_ = VertexBufferOffset;
		MeshRuntime.IndexBufferOffset_ = IndexBufferOffset;
		MeshRuntime.MaterialRef_ = nullptr;
		
		// Get resource.
		auto Resource = getPackage()->getCrossRefResource( pMeshData->MaterialRef_ );
		MeshRuntime.MaterialRef_ = Resource;
		BcAssertMsg( MeshRuntime.MaterialRef_.isValid(), "ScnModel: Material reference is invalid. Packing error." );

		// Push into array.
		MeshRuntimes_.emplace_back( std::move( MeshRuntime ) );
		
		// Advance vertex and index buffer bases
		VertexBufferOffset += pMeshData->NoofVertices_;
		IndexBufferOffset += pMeshData->NoofIndices_;
		BindVertexBufferBase += pMeshData->NoofVertices_ * pMeshData->VertexStride_;
		BindIndexBufferBase += pMeshData->NoofIndices_ * sizeof( BcU16 );
	}

	// Mark as ready.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnModel::destroy()
{
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
class ScnModelViewRenderData : 
	public ScnViewRenderData
{
public:
	ScnModelViewRenderData()
	{
	}

	virtual ~ScnModelViewRenderData()
	{
	}

	struct MaterialBinding
	{
		RsProgramBindingUPtr ProgramBinding_;
		RsRenderState* RenderState_;
	};

	std::vector< MaterialBinding > MaterialBindings_; 
};

//////////////////////////////////////////////////////////////////////////
// Define resource.
REFLECTION_DEFINE_DERIVED( ScnModelComponent );

void ScnModelComponent::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Model_", &ScnModelComponent::Model_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
		new ReField( "Layer_", &ScnModelComponent::Layer_, bcRFF_IMPORTER ),
		new ReField( "Position_", &ScnModelComponent::Position_, bcRFF_IMPORTER ),
		new ReField( "Scale_", &ScnModelComponent::Scale_, bcRFF_IMPORTER ),
		new ReField( "Rotation_", &ScnModelComponent::Rotation_, bcRFF_IMPORTER ),
		new ReField( "Uniforms_", &ScnModelComponent::Uniforms_, bcRFF_IMPORTER ),

		new ReField( "BaseTransform_", &ScnModelComponent::BaseTransform_ ),
		new ReField( "UploadFence_", &ScnModelComponent::UploadFence_, bcRFF_TRANSIENT ),
		new ReField( "pNodeTransformData_", &ScnModelComponent::pNodeTransformData_, bcRFF_TRANSIENT ),
		new ReField( "AABB_", &ScnModelComponent::AABB_, bcRFF_TRANSIENT ),
	};

	using namespace std::placeholders;
	ReRegisterClass< ScnModelComponent, Super >( Fields )
		.addAttribute( new ScnComponentProcessor( 
			{
				ScnComponentProcessFuncEntry(
					"Update",
					ScnComponentPriority::MODEL_UPDATE,
					std::bind( &ScnModelComponent::updateModels, _1 ) )
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelComponent::ScnModelComponent():
	Model_(),
	Layer_( 0 ),
	Position_( 0.0f, 0.0f, 0.0f ),
	Scale_( 1.0f, 1.0f, 1.0f ),
	Rotation_( 0.0f, 0.0f, 0.0f ),
	pNodeTransformData_( nullptr ),
	UploadFence_(),
	AABB_(),
	PerComponentMeshDataList_()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelComponent::ScnModelComponent( ScnModelRef Model ):
	ScnModelComponent()
{
	Model_ = Model;
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
// setUniforms
void ScnModelComponent::setUniforms( const ReClass* UniformClass, const void* UniformData )
{
#if !PSY_PRODUCTION
	const std::string DebugName = getFullName();
	const char* DebugNameCStr = DebugName.c_str();
#else
	const char* DebugNameCStr = nullptr;
#endif

	auto FoundIt = std::find_if( Uniforms_.begin(), Uniforms_.end(),
		[ UniformClass ]( const ScnModelUniforms& Uniform )
		{
			return Uniform.Name_ == *UniformClass->getName();
		} );

	auto Size = UniformClass->getSize();
	if( FoundIt == Uniforms_.end() )
	{
		ScnModelUniforms Uniform;
		Uniform.Name_ = *UniformClass->getName();
		Uniform.Data_ = std::move( BcBinaryData( Size ) );
		Uniform.Buffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc(
				RsResourceBindFlags::UNIFORM_BUFFER,
				RsResourceCreationFlags::STREAM,
				Size ), DebugNameCStr );	
		Uniforms_.emplace_back( std::move( Uniform ) );
		FoundIt = Uniforms_.end() - 1;

		// May need to rebind, so reset view data.
		ScnRenderableComponent::resetViewRenderData( nullptr );
	}

	// Copy into intermedate.
	memcpy( FoundIt->Data_.getData< BcU8 >(), UniformData, Size );
	UniformData = FoundIt->Data_.getData< BcU8 >();

	// Copy into buffer.
	RsCore::pImpl()->updateBuffer(
		FoundIt->Buffer_.get(), 0, Size, RsResourceUpdateFlags::ASYNC,
		[ UniformData, Size ]( RsBuffer* Buffer, RsBufferLock Lock )
		{
			memcpy( Lock.Buffer_, UniformData, Size );
		} );
}

//////////////////////////////////////////////////////////////////////////
// getNoofNodes
BcU32 ScnModelComponent::getNoofNodes() const
{
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	return NoofNodes;
}

#if 0
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
			MaterialComponents.emplace_back( PerComponentMeshDataList_[ Idx ].MaterialComponentRef_ );
		}
	}

	return std::move( MaterialComponents );
}
#endif

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
// recursiveModelUpdate
//static 
BcU32 ScnModelComponent::recursiveModelUpdate( const ScnComponentList& Components, BcU32 StartIdx, BcU32 EndIdx, BcU32 MaxNodesPerJob, SysFence* Fence )
{
	// Calculate number of nodes.
	BcU32 NoofNodes = 0;
	for( BcU32 Idx = StartIdx; Idx < EndIdx; ++Idx )
	{
		auto Component = Components[ Idx ];
		auto* ModelComponent = static_cast< ScnModelComponent* >( Component.get() );
		NoofNodes += ModelComponent->Model_->pHeader_->NoofNodes_;
	}

	// Check if we're updating a single component, hit node limit, or have no node limit.
	const BcU32 NoofComponents = ( EndIdx - StartIdx );
	if( NoofComponents == 1 || NoofNodes <= MaxNodesPerJob || MaxNodesPerJob == 0 )
	{
		for( BcU32 Idx = StartIdx; Idx < EndIdx; ++Idx )
		{
			auto Component = Components[ Idx ];
			auto* ModelComponent = static_cast< ScnModelComponent* >( Component.get() );
			
			MaMat4d Matrix = ModelComponent->BaseTransform_ * ModelComponent->getParentEntity()->getWorldMatrix();
			ModelComponent->updateNodes( Matrix );
		}

		Fence->decrement( NoofComponents );
	}
	else
	{
		using namespace std::placeholders;

		BcU32 MidIdx = ( StartIdx + EndIdx ) / 2;

		if( MidIdx > StartIdx )
		{
			SysKernel::pImpl()->pushFunctionJob( SysKernel::DEFAULT_JOB_QUEUE_ID,
				[ &Components, StartIdx, MidIdx, MaxNodesPerJob, Fence ]()
				{
					ScnModelComponent::recursiveModelUpdate( Components, StartIdx, MidIdx, MaxNodesPerJob, Fence );
				} );
		}

		if( EndIdx > MidIdx )
		{
			SysKernel::pImpl()->pushFunctionJob( SysKernel::DEFAULT_JOB_QUEUE_ID,
				[ &Components, MidIdx, EndIdx, MaxNodesPerJob, Fence ]()
				{
					ScnModelComponent::recursiveModelUpdate( Components, MidIdx, EndIdx, MaxNodesPerJob, Fence );
				} );
		}
	}

	return NoofNodes;
}

//////////////////////////////////////////////////////////////////////////
// updateModels
//static
void ScnModelComponent::updateModels( const ScnComponentList& Components )
{
	PSY_PROFILE_FUNCTION;

	static bool UseJobs = true;

	PSY_PROFILER_INSTANT_EVENT( "Updating models", nullptr );

	BcTimer Timer;
	Timer.mark();

	auto Tick = SysKernel::pImpl()->getFrameTime();
	SysFence Fence;

	// Wait for all uploads to complete.
	for( BcU32 Idx = 0; Idx < Components.size(); ++Idx )
	{
		auto Component = Components[ Idx ];
		auto* ModelComponent = static_cast< ScnModelComponent* >( Component.get() );
					
		ModelComponent->UploadFence_.wait();
	}
	
	// Recursive model update func.
	static BcU32 MaxNodesPerJob = 64;
	Fence.increment( Components.size() );
	BcU32 NoofNodes = recursiveModelUpdate( Components, 0, Components.size(), UseJobs ? MaxNodesPerJob : 0, &Fence );
	Fence.wait();

#if !PSY_PRODUCTION
	if ( ImGui::Begin( "Engine Debug" ) )
	{
		if( ImGui::TreeNode( "ScnModelComponent" ) )
		{
			const BcF32 UpdateTime = Timer.time() * 1000.0f;
			ImGui::Checkbox( "Enable jobs", &UseJobs );
			ImGui::Text( "Time: %f ms", UpdateTime );
			ImGui::Text( "Components: %u", Components.size() );
			ImGui::Text( "Nodes: %u", NoofNodes );

			static std::array< BcF32, 256 > GraphPoints = { 0.0f };
			static int GraphPointIdx = 0;
			GraphPoints[ GraphPointIdx ] = UpdateTime;
			GraphPointIdx = ( GraphPointIdx + 1 ) % GraphPoints.size();
			ImGui::PlotLines( "", GraphPoints.data(), GraphPoints.size(), GraphPointIdx, nullptr, 0.0f, 2.0f, MaVec2d( 0.0f, 128.0f ) );


			ImGui::InputInt( "MaxNodesPerJob", (int*)&MaxNodesPerJob );
			ImGui::Separator();
			ImGui::TreePop();
		}
	}
	ImGui::End();
#endif

}

//////////////////////////////////////////////////////////////////////////
// updateNodes
void ScnModelComponent::updateNodes( MaMat4d RootMatrix )
{
	PSY_PROFILE_FUNCTION;

	setBaseTransform( Position_, Scale_, Rotation_ );

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
			
			pNodeMeshRuntime->AABB_ = pNodeMeshData->AABB_.transform( pNodeTransformData->WorldTransform_ );
			FullAABB.expandBy( pNodeMeshRuntime->AABB_ );
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
				PerComponentMeshData.ObjectUniformBuffer_.get(),
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
				PerComponentMeshData.ObjectUniformBuffer_.get(),
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
					ObjectUniformBlock->NormalTransform_.translation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
					ObjectUniformBlock->NormalTransform_.inverse();
					ObjectUniformBlock->NormalTransform_.transpose();

					UploadFence_.decrement();
				} );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// createViewRenderData
//virtual
class ScnViewRenderData* ScnModelComponent::createViewRenderData( class ScnViewComponent* View )
{
	ScnModelViewRenderData* ViewRenderData = new ScnModelViewRenderData();

	// Setup program binding for all materials.
	ScnModelMeshRuntimeList& MeshRuntimes = Model_->MeshRuntimes_;
	ViewRenderData->MaterialBindings_.resize( MeshRuntimes.size() );
	for( BcU32 Idx = 0; Idx < MeshRuntimes.size(); ++Idx )
	{
		const auto& PerComponentMeshData = PerComponentMeshDataList_[ Idx ];
		ScnModelMeshData* pMeshData = &Model_->pMeshData_[ Idx ];
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ Idx ];
		auto Material = pMeshRuntime->MaterialRef_;

		if( Material.isValid() )
		{
			BcAssert( Material.isValid() && Material->isReady() );

			ScnShaderPermutationFlags ShaderPermutation = pMeshData->ShaderPermutation_;
			ShaderPermutation |= isLit() ? ScnShaderPermutationFlags::LIGHTING_DIFFUSE : ScnShaderPermutationFlags::LIGHTING_NONE;
			ShaderPermutation |= View->getRenderPermutation();

			auto Program = Material->getProgram( ShaderPermutation );
			auto ProgramBindingDesc = Material->getProgramBinding( ShaderPermutation );

			for( const auto& Uniform : Uniforms_ )
			{
				auto Slot = Program->findUniformBufferSlot( Uniform.Name_.c_str() );
				if( Slot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( Slot, Uniform.Buffer_.get() );
				}
			}

			if( pMeshData->IsSkinned_ )
			{
				auto Slot = Program->findUniformBufferSlot( "ScnShaderBoneUniformBlockData" );
				if( Slot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( Slot, PerComponentMeshData.ObjectUniformBuffer_.get() );
				}
			}
			else
			{
				auto Slot = Program->findUniformBufferSlot( "ScnShaderObjectUniformBlockData" );
				if( Slot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( Slot, PerComponentMeshData.ObjectUniformBuffer_.get() );
				}
			}

			{
				auto Slot = Program->findUniformBufferSlot( "ScnShaderViewUniformBlockData" );
				if( Slot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( Slot, View->getViewUniformBuffer() );
				}
			}

			if( PerComponentMeshData.LightingUniformBuffer_ )
			{
				auto Slot = Program->findUniformBufferSlot( "ScnShaderLightUniformBlockData" );
				if( Slot != BcErrorCode )
				{	
					ProgramBindingDesc.setUniformBuffer( Slot, PerComponentMeshData.LightingUniformBuffer_.get() );
				}
			}

			ViewRenderData->MaterialBindings_[ Idx ].ProgramBinding_ = RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, getFullName().c_str() );
			ViewRenderData->MaterialBindings_[ Idx ].RenderState_ = Material->getRenderState();
		}
	}
	return ViewRenderData;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnModelComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );

#if !PSY_PRODUCTION
	const std::string DebugName = getFullName();
	const char* DebugNameCStr = DebugName.c_str();
#else
	const char* DebugNameCStr = nullptr;
#endif

	// Duplicate node data for update/rendering.
	BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	pNodeTransformData_ = new ScnModelNodeTransformData[ NoofNodes ];
	BcMemCopy( pNodeTransformData_, Model_->pNodeTransformData_, sizeof( ScnModelNodeTransformData ) * NoofNodes );

	// Create uniform buffers.
	for( auto& Uniform : Uniforms_ )
	{
		auto Data = Uniform.Data_.getData< BcU8 >();
		auto Size = Uniform.Data_.getDataSize();
		if( Uniform.Buffer_ == nullptr )
		{
			Uniform.Buffer_ = RsCore::pImpl()->createBuffer( 
				RsBufferDesc(
					RsResourceBindFlags::UNIFORM_BUFFER,
					RsResourceCreationFlags::STREAM,
					Size ), DebugNameCStr );	
		}

		RsCore::pImpl()->updateBuffer(
			Uniform.Buffer_.get(), 0, Size, RsResourceUpdateFlags::ASYNC,
			[ Data, Size ]( RsBuffer* Buffer, RsBufferLock Lock )
			{
				memcpy( Lock.Buffer_, Data, Size );
			} );
	}

	// Create material instances to render with.
	ScnModelMeshRuntimeList& MeshRuntimes = Model_->MeshRuntimes_;
	ScnMaterialComponentRef MaterialComponentRef;
	PerComponentMeshDataList_.reserve( MeshRuntimes.size() );
	for( BcU32 Idx = 0; Idx < MeshRuntimes.size(); ++Idx )
	{
		ScnModelMeshData* pMeshData = &Model_->pMeshData_[ Idx ];
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ Idx ];
		auto Material = pMeshRuntime->MaterialRef_;
		TPerComponentMeshData ComponentData;

		// Create object uniform buffer - bone or object.
		if( pMeshData->IsSkinned_ )
		{
			ComponentData.ObjectUniformBuffer_ = Material->createUniformBuffer< ScnShaderBoneUniformBlockData >( DebugNameCStr );
		}
		else
		{
			ComponentData.ObjectUniformBuffer_ = Material->createUniformBuffer< ScnShaderObjectUniformBlockData >( DebugNameCStr );
		}

		// Try create lighting uniform buffer.
		ComponentData.LightingUniformBuffer_ = Material->createUniformBuffer< ScnShaderLightUniformBlockData >( DebugNameCStr );

		//
		PerComponentMeshDataList_.emplace_back( std::move( ComponentData ) );
	}

	// Update nodes.
	updateNodes( BaseTransform_ * getParentEntity()->getWorldMatrix() );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnModelComponent::onDetach( ScnEntityWeakRef Parent )
{
	// Wait for upload + render to complete.
	UploadFence_.wait();

	PerComponentMeshDataList_.clear();
	
	// Delete duplicated node data.
	delete [] pNodeTransformData_;
	pNodeTransformData_ = nullptr;

	//
	Super::onDetach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// renderPrimitives
void ScnModelComponent::render( ScnRenderContext & RenderContext )
{
	PSY_PROFILE_FUNCTION;

	Super::render( RenderContext );

	ScnModelMeshRuntimeList& MeshRuntimes = Model_->MeshRuntimes_;
	ScnModelMeshData* pMeshDatas = Model_->pMeshData_;
	auto* ViewRenderData = static_cast< ScnModelViewRenderData* >( RenderContext.ViewRenderData_ );

	// Set layer.
	RsRenderSort Sort = RenderContext.Sort_;
	Sort.Layer_ = Layer_;

	// Lighting visitors.
	if( isLit() )
	{
		for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < MeshRuntimes.size(); ++PrimitiveIdx )
		{
			ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ PrimitiveIdx ];
			TPerComponentMeshData& PerComponentMeshData = PerComponentMeshDataList_[ PrimitiveIdx ];
			if( PerComponentMeshData.LightingUniformBuffer_ )
			{
				ScnLightingVisitor LightingVisitor( pMeshRuntime->AABB_ );
				RsCore::pImpl()->updateBuffer( 
					PerComponentMeshData.LightingUniformBuffer_.get(), 0, sizeof( ScnShaderLightUniformBlockData ), 
					RsResourceUpdateFlags::ASYNC,
					[ LightUniformBlockData = LightingVisitor.getLightUniformBlockData() ]
					( RsBuffer* Buffer, const RsBufferLock& BufferLock )
					{
						BcAssert( Buffer->getDesc().SizeBytes_ == sizeof( LightUniformBlockData ) );
						BcMemCopy( BufferLock.Buffer_, &LightUniformBlockData, 
							sizeof( LightUniformBlockData ) );
					} );
			}
		}
	}

	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < MeshRuntimes.size(); ++PrimitiveIdx )
	{
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ PrimitiveIdx ];
		ScnModelMeshData* pMeshData = &pMeshDatas[ pMeshRuntime->MeshDataIndex_ ];
		TPerComponentMeshData& PerComponentMeshData = PerComponentMeshDataList_[ PrimitiveIdx ];
		auto& MaterialBinding = ViewRenderData->MaterialBindings_[ PrimitiveIdx ];
		BcU32 Offset = 0; // This will change when index buffers are merged.
		
		// Render primitive.
		RenderContext.pFrame_->queueRenderNode( Sort,
			[
				GeometryBinding = pMeshRuntime->GeometryBinding_,
				DrawProgramBinding = MaterialBinding.ProgramBinding_.get(),
				RenderState = MaterialBinding.RenderState_,
				FrameBuffer = RenderContext.pViewComponent_->getFrameBuffer(),
				Viewport = RenderContext.pViewComponent_->getViewport(),
				PrimitiveType = pMeshData->Type_,
				NoofIndices = pMeshData->NoofIndices_,
				IndexBuffereOffset = pMeshRuntime->IndexBufferOffset_,
				VertexBufferOffset = pMeshRuntime->VertexBufferOffset_
			]
			( RsContext* Context )
			{
				PSY_PROFILE_FUNCTION;
				Context->drawIndexedPrimitives(
					GeometryBinding,
					DrawProgramBinding, 
					RenderState, 
					FrameBuffer, 
					&Viewport,
					nullptr,
					PrimitiveType,
					IndexBuffereOffset, NoofIndices, VertexBufferOffset,
					0, 1 );
			} );
	}
}
