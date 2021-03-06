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
#include "System/Scene/Rendering/ScnRenderMesh.h"
#include "System/Scene/Rendering/ScnViewComponent.h"
#include "System/Scene/Rendering/ScnViewRenderData.h"

#include "System/Debug/DsCore.h"
#include "System/Debug/DsUtils.h"

#include "System/Content/CsCore.h"
#include "System/SysKernel.h"

#include "Editor/Editor.h"

#include "Base/BcProfiler.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnModelImport.h"
#endif

#define MAX_INSTANCES ( 128 )

//////////////////////////////////////////////////////////////////////////
// ScnModelUniforms
REFLECTION_DEFINE_BASIC( ScnModelUniforms );

void ScnModelUniforms::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Class_", &ScnModelUniforms::Class_ ),
		new ReField( "Data_", &ScnModelUniforms::Data_ ),
	};
	ReRegisterClass< ScnModelUniforms >( Fields );
}

ScnModelUniforms::ScnModelUniforms():
	Class_( nullptr ),
	Data_(),
	Buffer_()
{

}

//////////////////////////////////////////////////////////////////////////
// View render data.
class ScnModelViewRenderData : 
	public ScnViewRenderData
{
public:
	ScnModelViewRenderData()
	{
	}

	virtual ~ScnModelViewRenderData()
	{
		// Free MaterialBindings, but not InstancedMaterialBindings.
		for( auto& MaterialBinding : MaterialBindings_ )
		{
			RsCore::pImpl()->destroyResource( MaterialBinding.ProgramBinding_ );
		}
	}

	struct MaterialBinding
	{
		RsProgramBinding* ProgramBinding_;
		RsRenderState* RenderState_;
	};

	std::vector< MaterialBinding > MaterialBindings_; 
	std::vector< MaterialBinding > InstancedMaterialBindings_;
};

//////////////////////////////////////////////////////////////////////////
// Processor

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelProcessor::ScnModelProcessor():
	ScnComponentProcessor( {
		ScnComponentProcessFuncEntry(
			"Model Update",
			ScnComponentPriority::MODEL_UPDATE,
			std::bind( &ScnModelProcessor::updateModels, this, std::placeholders::_1 ) ),
		ScnComponentProcessFuncEntry(
			"Model Debug DraW",
			ScnComponentPriority::DEFAULT_DEBUG_DRAW,
			std::bind( &ScnModelProcessor::debugDraw, this, std::placeholders::_1 ) )
		} )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnModelProcessor::~ScnModelProcessor()
{
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnModelProcessor::initialise()
{
	ScnViewProcessor::pImpl()->registerRenderInterface( ScnModelComponent::StaticGetClass(), this );

#if !PSY_PRODUCTION
	if( DsCore::pImpl() )
	{
		DsCore::pImpl()->registerPanel( 
			"Processors", "ScnModelProcessor", nullptr, [ this ]( BcU32 )->void
			{
				const BcF32 UpdateTime = static_cast< BcF32 >( UpdateTime_ * 1000.0 );
				if( ImGui::TreeNode( "Update", "Update Time: %f ms", UpdateTime ) )
				{
					static std::array< BcF32, 256 > GraphPoints = { 0.0f };
					static int GraphPointIdx = 0;
					GraphPoints[ GraphPointIdx ] = UpdateTime;
					GraphPointIdx = ( GraphPointIdx + 1 ) % GraphPoints.size();
					ImGui::PlotLines( "", GraphPoints.data(), static_cast< BcU32 >( GraphPoints.size() ), GraphPointIdx, nullptr, 0.0f, 2.0f, MaVec2d( 0.0f, 128.0f ) );
					ImGui::TreePop();
				}
				const BcF32 SortTime = static_cast< BcF32 >( SortingTime_ * 1000.0 );
				if( ImGui::TreeNode( "Sort", "Sort Time: %f ms", SortTime ) )
				{
					static std::array< BcF32, 256 > GraphPoints = { 0.0f };
					static int GraphPointIdx = 0;
					GraphPoints[ GraphPointIdx ] = SortTime;
					GraphPointIdx = ( GraphPointIdx + 1 ) % GraphPoints.size();
					ImGui::PlotLines( "", GraphPoints.data(), static_cast< BcU32 >( GraphPoints.size() ), GraphPointIdx, nullptr, 0.0f, 2.0f, MaVec2d( 0.0f, 128.0f ) );
					ImGui::TreePop();
				}
				ImGui::Text( "Components: %u", NoofComponents_ );
				ImGui::Text( "Nodes: %u", NoofNodes_ );
				ImGui::Text( "Models Rendered: %u", ModelsRendered_ );
				ImGui::Text( "Total Draw Calls: %u", DrawCalls_ );

				ImGui::Checkbox( "Enable Jobs", &UseJobs_ );
				ImGui::InputInt( "MaxNodesPerJob", (int*)&MaxNodesPerJob_ );
			
				bool SortingEnabled = !!SortingEnabled_;
				bool InstancingEnabled = !!InstancingEnabled_;
				if( ImGui::Checkbox( "Sorting Enabled", &SortingEnabled ) )
				{
					SortingEnabled_ = SortingEnabled;
				}
				if( ImGui::Checkbox( "Instancing Enabled", &InstancingEnabled ) )
				{
					InstancingEnabled_ = InstancingEnabled;
				}
			} );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// shutdown
void ScnModelProcessor::shutdown()
{
	ScnViewProcessor::pImpl()->deregisterRenderInterface( ScnModelComponent::StaticGetClass(), this );

	InstancingData_.clear();
}

//////////////////////////////////////////////////////////////////////////
// createViewRenderData
class ScnViewRenderData* ScnModelProcessor::createViewRenderData( class ScnComponent* Component, class ScnViewComponent* View )
{
	BcAssert( Component->isTypeOf< ScnModelComponent >() );

#if !PSY_PRODUCTION
	const std::string DebugName = Component->getFullName();
	const char* DebugNameCStr = DebugName.c_str();
#else
	const char* DebugNameCStr = nullptr;
#endif

	auto* ModelComponent = static_cast< ScnModelComponent* >( Component );
	auto SortPassType = View->getSortPassType( ModelComponent->Passes_, ModelComponent->RenderPermutations_ );
	if( SortPassType == RsRenderSortPassType::INVALID )
	{
		return nullptr;
	}

	auto* ViewRenderData = new ScnModelViewRenderData();
	auto* Model = ModelComponent->Model_.get();
	auto ViewModelPair = std::make_pair( View, Model );
	const auto & Features = RsCore::pImpl()->getContext( nullptr )->getFeatures();

	// Setup program binding for all materials.
	ScnModelMeshRuntimeList& MeshRuntimes = ModelComponent->Model_->MeshRuntimes_;
	ViewRenderData->MaterialBindings_.resize( MeshRuntimes.size() );

	bool IsInstancable = Features.Instancing_;
	for( BcU32 Idx = 0; Idx < MeshRuntimes.size(); ++Idx )
	{
		auto& PerComponentMeshData = ModelComponent->PerComponentMeshDataList_[ Idx ];
		ScnModelMeshData* pMeshData = &Model->pMeshData_[ Idx ];
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ Idx ];
		auto Material = pMeshRuntime->MaterialRef_;

		if( Material != nullptr )
		{
			BcAssert( Material->isReady() );

			ScnShaderPermutationFlags ShaderPermutation = pMeshData->ShaderPermutation_;
			ShaderPermutation |= View->getRenderPermutation();

			IsInstancable &= Material->hasPermutation( ScnShaderPermutationFlags::MESH_INSTANCED_3D );

			auto Program = Material->getProgram( ShaderPermutation );
			BcAssert( Program );
			auto ProgramBindingDesc = Material->getProgramBinding( ShaderPermutation );

			for( const auto& Uniform : ModelComponent->Uniforms_ )
			{
				auto Slot = Program->findUniformBufferSlot( (*Uniform.Class_->getName()).c_str() );
				if( Slot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( Slot, Uniform.Buffer_.get(), 0, static_cast< BcU32 >( Uniform.Buffer_->getDesc().SizeBytes_ ) );

					auto* Attribute = Uniform.Class_->getAttribute< ScnShaderDataAttribute >();
					IsInstancable &= Attribute->isInstancable();
					IsInstancable &= Program->findUniformBufferSlot( (*Uniform.Class_->getName() + "Instanced" ).c_str() ) != BcErrorCode;
				}
			}

			if( pMeshData->IsSkinned_ )
			{
				auto Slot = Program->findUniformBufferSlot( "ScnShaderBoneUniformBlockData" );
				if( Slot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( Slot, PerComponentMeshData.ObjectUniformBuffer_.get(), 0, sizeof( ScnShaderBoneUniformBlockData ) );

					auto* Attribute = ScnShaderBoneUniformBlockData::StaticGetClass()->getAttribute< ScnShaderDataAttribute >();
					IsInstancable &= Attribute->isInstancable();
					IsInstancable &= Program->findUniformBufferSlot( "ScnShaderBoneUniformBlockDataInstanced" ) != BcErrorCode;
				}
			}
			else
			{
				auto Slot = Program->findUniformBufferSlot( "ScnShaderObjectUniformBlockData" );
				if( Slot != BcErrorCode )
				{
					ProgramBindingDesc.setUniformBuffer( Slot, PerComponentMeshData.ObjectUniformBuffer_.get(), 0, sizeof( ScnShaderObjectUniformBlockData ) );

					auto* Attribute = ScnShaderObjectUniformBlockData::StaticGetClass()->getAttribute< ScnShaderDataAttribute >();
					IsInstancable &= Attribute->isInstancable();
					IsInstancable &= Program->findUniformBufferSlot( "ScnShaderObjectUniformBlockDataInstanced" ) != BcErrorCode;
				}
			}

			{
				auto Slot = Program->findUniformBufferSlot( "ScnShaderLightUniformBlockData" );
				if( Slot != BcErrorCode )
				{
					// Try create lighting uniform buffer if we need to.
					if( PerComponentMeshData.LightingUniformBuffer_ == nullptr )
					{
						PerComponentMeshData.LightingUniformBuffer_ = Material->createUniformBuffer< ScnShaderLightUniformBlockData >( DebugNameCStr );
					}

					ProgramBindingDesc.setUniformBuffer( Slot, PerComponentMeshData.LightingUniformBuffer_.get(), 0, sizeof( ScnShaderLightUniformBlockData ) );

					auto* Attribute = ScnShaderLightUniformBlockData::StaticGetClass()->getAttribute< ScnShaderDataAttribute >();
					IsInstancable &= Attribute->isInstancable();
					IsInstancable &= Program->findUniformBufferSlot( "ScnShaderLightUniformBlockDataInstanced" ) != BcErrorCode;
				}
				else
				{
					// No binding, reset buffer.
					PerComponentMeshData.LightingUniformBuffer_.reset();
				}
			}

			// Set view resources.
			View->setViewResources( Program, ProgramBindingDesc );

			// Create program binding for non-instanced rendering.
			ViewRenderData->MaterialBindings_[ Idx ].ProgramBinding_ = RsCore::pImpl()->createProgramBinding( Program, ProgramBindingDesc, DebugNameCStr ).release();
			ViewRenderData->MaterialBindings_[ Idx ].RenderState_ = Material->getRenderState();
		}
	}

	if( IsInstancable )
	{
		ViewRenderData->InstancedMaterialBindings_.resize( MeshRuntimes.size() );

		// Find model instancing data, and add if it's missing.
		if( InstancingData_.find( ViewModelPair ) == InstancingData_.end() )
		{
			InstancingData_.insert( std::make_pair( ViewModelPair, InstancingData() ) );
		}
		auto& InstancingData = InstancingData_.find( ViewModelPair )->second;
		InstancingData.RefCount_++;

		for( BcU32 Idx = 0; Idx < MeshRuntimes.size(); ++Idx )
		{
			ScnModelMeshData* pMeshData = &Model->pMeshData_[ Idx ];
			ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ Idx ];
			auto Material = pMeshRuntime->MaterialRef_;

			if( Material != nullptr )
			{
				BcAssert( Material->isReady() );

				ScnShaderPermutationFlags ShaderPermutation = pMeshData->ShaderPermutation_;
				ShaderPermutation |= View->getRenderPermutation();

				// Create program binding for instanced rendering.
				if( BcContainsAnyFlags( ScnShaderPermutationFlags::MESH_STATIC_3D, ShaderPermutation ) )
				{
					ShaderPermutation &= ~ScnShaderPermutationFlags::MESH_STATIC_3D;
					ShaderPermutation |= ScnShaderPermutationFlags::MESH_INSTANCED_3D;

					auto InstancedProgram = Material->getProgram( ShaderPermutation );
					if( InstancedProgram )
					{
						auto InstancedProgramBindingDesc = Material->getProgramBinding( ShaderPermutation );

						InstancingData::UniformBuffer UniformBuffer;
						auto& UniformBuffers = InstancingData.UniformBuffers_;

						// Setup instanced uniform buffers.
						for( const auto& Uniform : ModelComponent->Uniforms_ )
						{
							auto Slot = InstancedProgram->findUniformBufferSlot( (*Uniform.Class_->getName() + "Instanced").c_str() );
							if( Slot != BcErrorCode )
							{
								BcU32 Size = static_cast< BcU32 >( Uniform.Class_->getSize() ) * MAX_INSTANCES;
								if( UniformBuffers.find( Uniform.Class_ ) == UniformBuffers.end() ) 
								{
									UniformBuffer.Buffer_ = RsCore::pImpl()->createBuffer(
										RsBufferDesc(
											RsBindFlags::UNIFORM_BUFFER,
											RsResourceCreationFlags::STREAM,
											Size ), DebugNameCStr ).release();
									UniformBuffers.insert( std::make_pair( Uniform.Class_, UniformBuffer ) );
								}
								UniformBuffer = UniformBuffers.find( Uniform.Class_ )->second;
								InstancedProgramBindingDesc.setUniformBuffer( Slot, UniformBuffer.Buffer_, 0, Size );

								auto* Attribute = Uniform.Class_->getAttribute< ScnShaderDataAttribute >();
								BcAssert( Attribute->isInstancable() );
							}
						}

						// Object uniforms.
						{
							BcAssert( !pMeshData->IsSkinned_ );
							auto Slot = InstancedProgram->findUniformBufferSlot( "ScnShaderObjectUniformBlockDataInstanced" );
							if( Slot != BcErrorCode )
							{
								const ReClass* Class = ScnShaderObjectUniformBlockData::StaticGetClass();
								BcU32 Size = static_cast< BcU32 >( sizeof( ScnShaderObjectUniformBlockData ) ) * MAX_INSTANCES;
								if( UniformBuffers.find( Class ) == UniformBuffers.end() ) 
								{
									UniformBuffer.Buffer_ = RsCore::pImpl()->createBuffer(
										RsBufferDesc(
											RsBindFlags::UNIFORM_BUFFER,
											RsResourceCreationFlags::STREAM,
											Size ), DebugNameCStr ).release();
									UniformBuffers.insert( std::make_pair( Class, UniformBuffer ) );
								}
								UniformBuffer = UniformBuffers.find( Class )->second;

								InstancedProgramBindingDesc.setUniformBuffer( Slot, UniformBuffer.Buffer_, 0, Size );

								auto* Attribute = ScnShaderObjectUniformBlockData::StaticGetClass()->getAttribute< ScnShaderDataAttribute >();
								BcAssert( Attribute->isInstancable() );
							}
						}

						// Light uniforms.
						{
							auto Slot = InstancedProgram->findUniformBufferSlot( "ScnShaderLightUniformBlockDataInstanced" );
							if( Slot != BcErrorCode )
							{
								const ReClass* Class = ScnShaderLightUniformBlockData::StaticGetClass();
								BcU32 Size = static_cast< BcU32 >( sizeof( ScnShaderLightUniformBlockData ) ) * MAX_INSTANCES;
								if( UniformBuffers.find( Class ) == UniformBuffers.end() ) 
								{
									UniformBuffer.Buffer_ = RsCore::pImpl()->createBuffer(
										RsBufferDesc(
											RsBindFlags::UNIFORM_BUFFER,
											RsResourceCreationFlags::STREAM,
											Size ), DebugNameCStr ).release();
									UniformBuffers.insert( std::make_pair( Class, UniformBuffer ) );
								}
								UniformBuffer = UniformBuffers.find( Class )->second;

								InstancedProgramBindingDesc.setUniformBuffer( Slot, UniformBuffer.Buffer_, 0, Size );

								auto* Attribute = ScnShaderLightUniformBlockData::StaticGetClass()->getAttribute< ScnShaderDataAttribute >();
								BcAssert( Attribute->isInstancable() );
							}
						}

						// Set view resources.
						View->setViewResources( InstancedProgram, InstancedProgramBindingDesc );

						// Create program binding for non-instanced rendering.
						auto ProgramBinding = RsCore::pImpl()->createProgramBinding( InstancedProgram, InstancedProgramBindingDesc, DebugNameCStr ).release();
						InstancingData.ProgramBindings_.push_back( ProgramBinding );

						ViewRenderData->InstancedMaterialBindings_[ Idx ].ProgramBinding_ = ProgramBinding;
						ViewRenderData->InstancedMaterialBindings_[ Idx ].RenderState_ = Material->getRenderState();
					}
				}
			}
		}
	}

	ViewRenderData->setSortPassType( SortPassType );
	return ViewRenderData;
}

//////////////////////////////////////////////////////////////////////////
// destroyViewRenderData
void ScnModelProcessor::destroyViewRenderData( class ScnComponent* Component, class ScnViewComponent* View, ScnViewRenderData* ViewRenderData )
{
	BcAssert( Component->isTypeOf< ScnModelComponent >() );

	auto* ModelComponent = static_cast< ScnModelComponent* >( Component );
	auto ViewModelPair = std::make_pair( View, ModelComponent->Model_.get() );
	auto InstancingDataIt = InstancingData_.find( ViewModelPair );
	if( InstancingDataIt != InstancingData_.end() )
	{
		auto& InstancingData = InstancingDataIt->second;
		if( --InstancingData.RefCount_ == 0 )
		{
			for( auto ProgramBinding : InstancingData.ProgramBindings_ )
			{
				RsCore::pImpl()->destroyResource( ProgramBinding );
			}
		
			for( auto Buffer : InstancingData.UniformBuffers_ )
			{
				RsCore::pImpl()->destroyResource( Buffer.second.Buffer_ );
			}
			InstancingData_.erase( InstancingDataIt );
		}
	}

	delete ViewRenderData;
}

//////////////////////////////////////////////////////////////////////////
// render
void ScnModelProcessor::render( const ScnViewComponentRenderData* ComponentRenderDatas, BcU32 NoofComponents, class ScnRenderContext& RenderContext )
{
	// Fill our vector with components we should render.
	// TODO: Per frame allocator, or pass up as non-const?
	ComponentRenderDatas_.clear();
	ComponentRenderDatas_.reserve( NoofComponents );
	for( BcU32 Idx = 0; Idx < NoofComponents; ++Idx )
	{
		const ScnViewComponentRenderData& ComponentRenderData( ComponentRenderDatas[ Idx ] );
		auto* ViewRenderData = ComponentRenderData.ViewRenderData_;
		if( ViewRenderData )
		{
			ComponentRenderDatas_.push_back( ComponentRenderDatas[ Idx ] );
		}
	}

	// Sort.
	if( SortingEnabled_ )
	{
		BcTimer Time;
		Time.mark();
		std::sort( ComponentRenderDatas_.begin(), ComponentRenderDatas_.end(), 
			[]( const ScnViewComponentRenderData& A, const ScnViewComponentRenderData& B )
			{
				BcAssert( A.Component_->isTypeOf< ScnModelComponent >() );
				BcAssert( B.Component_->isTypeOf< ScnModelComponent >() );
				const auto* ComponentA = static_cast< ScnModelComponent* >( A.Component_ );
				const auto* ComponentB = static_cast< ScnModelComponent* >( B.Component_ );
				return std::make_tuple( ComponentA->Layer_, ComponentA->Model_ ) < 
					std::make_tuple( ComponentB->Layer_, ComponentB->Model_ );
			} );
		SortingTime_ += Time.time();
	}

	// Render.
	ScnModelComponent* LastModelComponent = nullptr;
	ModelsRendered_ += ComponentRenderDatas_.size();
	for( size_t IdxA = 0; IdxA < ComponentRenderDatas_.size(); ++IdxA )
	{
		const ScnViewComponentRenderData& ComponentRenderData( ComponentRenderDatas_[ IdxA ] );
		BcAssert( ComponentRenderData.Component_->isTypeOf< ScnModelComponent >() );
		auto Component = static_cast< ScnModelComponent* >( ComponentRenderData.Component_ );
		auto Model = Component->Model_;

		BcBool DrawNonInstanced = BcTrue;

		if( InstancingEnabled_ )
		{
			LastModelComponent = Component;

			// Determine if we can instance.
			auto ViewModelPair = std::make_pair( RenderContext.View_, Model );
			auto InstancingDataIt = InstancingData_.find( ViewModelPair );
			if( InstancingDataIt != InstancingData_.end() )
			{
				auto* ViewRenderData = static_cast< ScnModelViewRenderData* >( ComponentRenderData.ViewRenderData_ );
				RenderContext.ViewRenderData_ = ViewRenderData;
				RenderContext.Sort_.Pass_ = BcU64( ViewRenderData->getSortPassType() );

				ScnModelMeshRuntimeList& MeshRuntimes = Model->MeshRuntimes_;
				ScnModelMeshData* pMeshDatas = Model->pMeshData_;

				BcU32 NoofInstances = 0;
				for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < MeshRuntimes.size(); ++PrimitiveIdx )
				{
					ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ PrimitiveIdx ];
					ScnModelMeshData* pMeshData = &pMeshDatas[ pMeshRuntime->MeshDataIdx_ ];
					auto& InstancedMaterialBinding = ViewRenderData->InstancedMaterialBindings_[ PrimitiveIdx ];
					BcAssert( InstancedMaterialBinding.ProgramBinding_ );
					BcAssert( InstancedMaterialBinding.RenderState_ );

					// Set layer.
					RsRenderSort Sort = RenderContext.Sort_;
					Sort.Layer_ = Component->Layer_;

					// Iterate until end, then draw.
					NoofInstances = 0;

					for( size_t IdxB = IdxA; IdxB < ComponentRenderDatas_.size(); ++IdxB )
					{
						const ScnViewComponentRenderData& InstancedComponentRenderData( ComponentRenderDatas_[ IdxB ] );
						BcAssert( InstancedComponentRenderData.Component_->isTypeOf< ScnModelComponent >() );
						auto InstancedComponent = static_cast< ScnModelComponent* >( InstancedComponentRenderData.Component_ );
						if( !LastModelComponent->isInstancingMatch( *InstancedComponent ) || NoofInstances >= MAX_INSTANCES )
						{
							break;
						}

						LastModelComponent = InstancedComponent;
						NoofInstances++;
					}

					if( NoofInstances > 0 )
					{
						// Allocate memory on frame for uniform uploads.
						auto& UniformBuffers = InstancingDataIt->second.UniformBuffers_;
						for( auto& UniformBufferIt : UniformBuffers )
						{
							auto BufferSize = UniformBufferIt.second.Buffer_->getDesc().SizeBytes_;
							BufferSize = ( BufferSize / MAX_INSTANCES ) * NoofInstances; 
							UniformBufferIt.second.UploadBuffer_ = reinterpret_cast< BcU8* >( RenderContext.pFrame_->allocMem( BufferSize ) );
						}

						for( BcU32 IdxB = 0; IdxB < NoofInstances; ++IdxB )
						{
							
							const ScnViewComponentRenderData& InstancedComponentRenderData( ComponentRenderDatas_[ IdxA + IdxB ] );
							BcAssert( InstancedComponentRenderData.Component_->isTypeOf< ScnModelComponent >() );
							auto InstancedComponent = static_cast< ScnModelComponent* >( InstancedComponentRenderData.Component_ );

							// TODO: Batch updates.
							for( BcU32 UniformIdx = 0; UniformIdx < InstancedComponent->Uniforms_.size(); ++UniformIdx )
							{
								auto& Uniform = InstancedComponent->Uniforms_[ UniformIdx ];
								auto UniformBufferIt = UniformBuffers.find( Uniform.Class_ );
								if( UniformBufferIt != UniformBuffers.end() )
								{
									auto Size = Uniform.Class_->getSize();
									auto Offset = Size * IdxB;
									auto UniformBuffer = UniformBufferIt->second;

									BcMemCopy( UniformBuffer.UploadBuffer_ + Offset, Uniform.Data_.getData< BcU8 >(), Size );
								}
							}

							// Do object uniform.
							{
								auto UniformBufferIt = UniformBuffers.find( ScnShaderObjectUniformBlockData::StaticGetClass() );
								if( UniformBufferIt != UniformBuffers.end() )
								{
									auto Size = sizeof( ScnShaderObjectUniformBlockData );
									auto Offset = Size * IdxB;
									auto UniformBuffer = UniformBufferIt->second;

									ScnShaderObjectUniformBlockData* ObjectUniformBlock = reinterpret_cast< ScnShaderObjectUniformBlockData* >( UniformBuffer.UploadBuffer_ + Offset );
									ScnModelNodeTransformData* pNodeTransformData = &InstancedComponent->pNodeTransformData_[ pMeshData->NodeIdx_ ];

									// World matrix.
									ObjectUniformBlock->WorldTransform_ = pNodeTransformData->WorldTransform_;

									// Normal matrix.
									ObjectUniformBlock->NormalTransform_ = pNodeTransformData->WorldTransform_;
									ObjectUniformBlock->NormalTransform_.translation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
#if 0 // Normal when using non-uniform scaling are broken without this. Consider implementing it as optional?
									ObjectUniformBlock->NormalTransform_.inverse();
									ObjectUniformBlock->NormalTransform_.transpose();
#endif
								}
							}

							// Do lighting uniform.
							auto UniformBufferIt = UniformBuffers.find( ScnShaderLightUniformBlockData::StaticGetClass() );
							if( UniformBufferIt != UniformBuffers.end() )
							{
								auto Size = sizeof( ScnShaderLightUniformBlockData );
								auto Offset = Size * IdxB;
								auto UniformBuffer = UniformBufferIt->second;

								ScnModelComponent::TPerComponentMeshData& PerComponentMeshData = Component->PerComponentMeshDataList_[ PrimitiveIdx ];
								ScnLightingVisitor LightingVisitor( PerComponentMeshData.AABB_ );
								BcMemCopy( UniformBuffer.UploadBuffer_ + Offset, &LightingVisitor.getLightUniformBlockData(), sizeof( ScnShaderLightUniformBlockData ) );
							}
						}

						// Upload all uniforms.
						for( auto& UniformBufferIt : UniformBuffers )
						{
							auto UniformBuffer = UniformBufferIt.second;
							auto Size = ( UniformBuffer.Buffer_->getDesc().SizeBytes_ / MAX_INSTANCES ) * NoofInstances;
							auto Data = UniformBuffer.UploadBuffer_;

							RenderContext.pFrame_->queueRenderNode( Sort,
								[
									Buffer = UniformBuffer.Buffer_,
									Size, Data
								]
								( RsContext* Context )
								{
									Context->updateBuffer( Buffer, 0, Size, 
										RsResourceUpdateFlags::ASYNC,
										[ Size, Data ]
										( RsBuffer* Buffer, const RsBufferLock& BufferLock )
										{
											BcMemCopy( BufferLock.Buffer_, Data, Size );
										} );
								} );
						}

						// Render primitive.
						RenderContext.pFrame_->queueRenderNode( Sort,
							[
								DrawProgramBinding = InstancedMaterialBinding.ProgramBinding_,
								RenderState = InstancedMaterialBinding.RenderState_,
								FrameBuffer = RenderContext.View_->getFrameBuffer(),
								Viewport = RenderContext.View_->getViewport(),
								PrimitiveType = pMeshRuntime->RenderMeshRef_->getHeader().ToplogyType_,
								RenderMesh = pMeshRuntime->RenderMeshRef_,
								DrawIdx = pMeshData->DrawIdx_,
								NoofInstances
							]
							( RsContext* Context )
							{
								PSY_PROFILE_FUNCTION;
								PSY_PROFILER_GPU_SECTION( "ScnModelProcessor::render" );
								auto Draw = RenderMesh->getDraw( DrawIdx );
								Context->drawIndexedPrimitives(
									RenderMesh->getGeometryBinding(),
									DrawProgramBinding, 
									RenderState, 
									FrameBuffer, 
									&Viewport,
									nullptr,
									PrimitiveType,
									Draw.IndexOffset_, 
									Draw.NoofIndices_, 
									Draw.VertexOffset_,
									0, NoofInstances );
							} );
						++DrawCalls_;
						DrawNonInstanced = BcFalse;
					}
				}

				BcAssert( NoofInstances > 0 );
				IdxA += NoofInstances - 1;
			}
		}

		if( DrawNonInstanced )
		{
			auto* ViewRenderData = static_cast< ScnModelViewRenderData* >( ComponentRenderData.ViewRenderData_ );
			RenderContext.ViewRenderData_ = ViewRenderData;
			RenderContext.Sort_.Pass_ = BcU64( ViewRenderData->getSortPassType() );

			ScnModelMeshRuntimeList& MeshRuntimes = Component->Model_->MeshRuntimes_;
			ScnModelMeshData* pMeshDatas = Model->pMeshData_;

			// Set layer.
			RsRenderSort Sort = RenderContext.Sort_;
			Sort.Layer_ = Component->Layer_;

			// Lighting visitors.
			for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < MeshRuntimes.size(); ++PrimitiveIdx )
			{
				ScnModelComponent::TPerComponentMeshData& PerComponentMeshData = Component->PerComponentMeshDataList_[ PrimitiveIdx ];
				if( PerComponentMeshData.LightingUniformBuffer_ )
				{
					ScnLightingVisitor LightingVisitor( PerComponentMeshData.AABB_ );
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

			for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < MeshRuntimes.size(); ++PrimitiveIdx )
			{
				ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ PrimitiveIdx ];
				ScnModelMeshData* pMeshData = &pMeshDatas[ pMeshRuntime->MeshDataIdx_ ];
				auto& MaterialBinding = ViewRenderData->MaterialBindings_[ PrimitiveIdx ];
		
				// Render primitive.
				RenderContext.pFrame_->queueRenderNode( Sort,
					[
						DrawProgramBinding = MaterialBinding.ProgramBinding_,
						RenderState = MaterialBinding.RenderState_,
						FrameBuffer = RenderContext.View_->getFrameBuffer(),
						Viewport = RenderContext.View_->getViewport(),
						PrimitiveType = pMeshRuntime->RenderMeshRef_->getHeader().ToplogyType_,
						RenderMesh = pMeshRuntime->RenderMeshRef_,
						DrawIdx = pMeshData->DrawIdx_
					]
					( RsContext* Context )
					{
						PSY_PROFILE_FUNCTION;
						auto Draw = RenderMesh->getDraw( DrawIdx );
						Context->drawIndexedPrimitives(
							RenderMesh->getGeometryBinding(),
							DrawProgramBinding, 
							RenderState, 
							FrameBuffer, 
							&Viewport,
							nullptr,
							PrimitiveType,
							Draw.IndexOffset_, 
							Draw.NoofIndices_, 
							Draw.VertexOffset_,
							0, 1 );
					} );
				++DrawCalls_;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getAABB
void ScnModelProcessor::getAABB( MaAABB* OutAABBs, class ScnComponent** Components, BcU32 NoofComponents )
{
	for( BcU32 Idx = 0; Idx < NoofComponents; ++Idx )
	{
		ScnComponent* Component = Components[ Idx ];
		BcAssert( Component->isTypeOf< ScnModelComponent >() );
		OutAABBs[ Idx ] = static_cast< ScnModelComponent* >( Component )->getAABB();
	}
}

//////////////////////////////////////////////////////////////////////////
// getRenderMask
void ScnModelProcessor::getRenderMask( BcU32* OutRenderMasks, class ScnComponent** Components, BcU32 NoofComponents )
{
	for( BcU32 Idx = 0; Idx < NoofComponents; ++Idx )
	{
		ScnComponent* Component = Components[ Idx ];
		BcAssert( Component->isTypeOf< ScnModelComponent >() );
		OutRenderMasks[ Idx ] = static_cast< ScnModelComponent* >( Component )->RenderMask_;
	}
}

//////////////////////////////////////////////////////////////////////////
// recursiveModelUpdate
size_t ScnModelProcessor::recursiveModelUpdate( const ScnComponentList& Components, size_t StartIdx, size_t EndIdx, size_t MaxNodesPerJob, SysFence* Fence )
{
	// Calculate number of nodes.
	size_t NoofNodes = 0;
	for( size_t Idx = StartIdx; Idx < EndIdx; ++Idx )
	{
		auto Component = Components[ Idx ];
		auto* ModelComponent = static_cast< ScnModelComponent* >( Component.get() );
		NoofNodes += ModelComponent->Model_->pHeader_->NoofNodes_;
	}

	// Check if we're updating a single component, hit node limit, or have no node limit.
	const size_t NoofComponents = ( EndIdx - StartIdx );
	if( NoofComponents == 1 || NoofNodes <= MaxNodesPerJob || MaxNodesPerJob == 0 )
	{
		for( size_t Idx = StartIdx; Idx < EndIdx; ++Idx )
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

		size_t MidIdx = ( StartIdx + EndIdx ) / 2;

		if( MidIdx > StartIdx )
		{
			SysKernel::pImpl()->pushFunctionJob( SysKernel::DEFAULT_JOB_QUEUE_ID,
				[ this, &Components, StartIdx, MidIdx, MaxNodesPerJob, Fence ]()
				{
					recursiveModelUpdate( Components, StartIdx, MidIdx, MaxNodesPerJob, Fence );
				} );
		}

		if( EndIdx > MidIdx )
		{
			SysKernel::pImpl()->pushFunctionJob( SysKernel::DEFAULT_JOB_QUEUE_ID,
				[ this, &Components, MidIdx, EndIdx, MaxNodesPerJob, Fence ]()
				{
					recursiveModelUpdate( Components, MidIdx, EndIdx, MaxNodesPerJob, Fence );
				} );
		}
	}

	return NoofNodes;
}

//////////////////////////////////////////////////////////////////////////
// updateModels
void ScnModelProcessor::updateModels( const ScnComponentList& Components )
{
	PSY_PROFILE_FUNCTION;

	PSY_PROFILER_INSTANT_EVENT( "Updating models", nullptr );

#if !PSY_PRODUCTION
	BcTimer Timer;
	Timer.mark();
#endif

	SysFence Fence;

	// Wait for all uploads to complete.
	for( BcU32 Idx = 0; Idx < Components.size(); ++Idx )
	{
		auto Component = Components[ Idx ];
		auto* ModelComponent = static_cast< ScnModelComponent* >( Component.get() );
					
		ModelComponent->UploadFence_.wait();
	}
	
	// Recursive model update func.
	Fence.increment( Components.size() );
	NoofNodes_ = recursiveModelUpdate( Components, 0, Components.size(), UseJobs_ ? MaxNodesPerJob_ : 0, &Fence );
	Fence.wait();
	ModelsRendered_ = 0;
	DrawCalls_ = 0;
#if !PSY_PRODUCTION
	UpdateTime_ = Timer.time();
#endif
	SortingTime_ = 0.0f;
	NoofComponents_ = Components.size();
}

//////////////////////////////////////////////////////////////////////////
// debugDraw
void ScnModelProcessor::debugDraw( const ScnComponentList& Components )
{
#if !PSY_PRODUCTION
	if( Debug::CanDraw( "Model" ) )
	{
		Debug::DrawCategory DrawCategory( "Model" );
		for( auto Component : Components )
		{
			auto* ModelComponent = static_cast< ScnModelComponent* >( Component.get() );
			Debug::DrawMatrix( ModelComponent->getParentEntity()->getWorldMatrix() );
			for( BcU32 Idx = 0; Idx < ModelComponent->getNoofNodes(); ++Idx )
			{
				auto ParentIdx = ModelComponent->Model_->pNodePropertyData_[ Idx ].ParentIndex_;
				MaMat4d Transform = ModelComponent->getNodeWorldTransform( Idx );
				MaMat4d ParentTransform = ModelComponent->getParentEntity()->getWorldMatrix();
				if( ParentIdx != BcErrorCode )
				{
					ParentTransform = ModelComponent->getNodeWorldTransform( ParentIdx );
				}

#if 0 // Pushed too many vertices to ImGui. Revisit later.
				auto ModelName = (*ModelComponent->Model_->pNodePropertyData_[ Idx ].Name_);
				if( ModelName.size() > 0 )
				{
					Debug::DrawShadowedText( Transform.translation(), RsColour::WHITE, ModelName.c_str() );
				}
#endif
				Debug::DrawLine( Transform.translation(), ParentTransform.translation(), RsColour::GREEN );
				Debug::DrawMatrix( Transform );
			}
		}
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnModel );

void ScnModel::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pHeader_", &ScnModel::pHeader_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA | bcRFF_TRANSIENT ),
		new ReField( "pNodeTransformData_", &ScnModel::pNodeTransformData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA | bcRFF_TRANSIENT ),
		new ReField( "pNodePropertyData_", &ScnModel::pNodePropertyData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA | bcRFF_TRANSIENT ),
		new ReField( "pVertexBufferData_", &ScnModel::pVertexBufferData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA | bcRFF_TRANSIENT ),
		new ReField( "pIndexBufferData_", &ScnModel::pIndexBufferData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA | bcRFF_TRANSIENT ),
		new ReField( "pVertexElements_", &ScnModel::pVertexElements_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA | bcRFF_TRANSIENT ),
		new ReField( "pMeshData_", &ScnModel::pMeshData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA | bcRFF_TRANSIENT ),
		//TODO: move support. new ReField( "MeshRuntimes_", &ScnModel::MeshRuntimes_, bcRFF_TRANSIENT ),
	};
		
	auto& Class = ReRegisterClass< ScnModel, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnModelImport::StaticGetClass(), 1 ) );
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
						ImGui::ScopedID ScopedID( &MeshRuntime );
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
				Editor::ObjectEditor( ThisFieldEditor, Object, Class, Flags );
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
	
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < pHeader_->NoofPrimitives_; ++PrimitiveIdx )
	{
		ScnModelMeshData* pMeshData = &pMeshData_[ PrimitiveIdx ];
		
		// Setup runtime structure.
		ScnModelMeshRuntime MeshRuntime;
		MeshRuntime.MeshDataIdx_ = PrimitiveIdx;
		MeshRuntime.RenderMeshRef_ = nullptr;
		MeshRuntime.MaterialRef_ = nullptr;
		
		// Get resources
		MeshRuntime.RenderMeshRef_ = static_cast< ScnRenderMesh* >( 
			getPackage()->getCrossRefResource( pMeshData->RenderMeshRef_ ) );
		BcAssertMsg( MeshRuntime.RenderMeshRef_ && MeshRuntime.RenderMeshRef_->isTypeOf< ScnRenderMesh >(), "ScnModel: Render mesh reference is invalid. Packing error." );

		MeshRuntime.MaterialRef_ = static_cast< ScnMaterial* >( 
			getPackage()->getCrossRefResource( pMeshData->MaterialRef_ ) );
		BcAssertMsg( MeshRuntime.MaterialRef_ && MeshRuntime.MaterialRef_->isTypeOf< ScnMaterial >(), "ScnModel: Material reference is invalid. Packing error." );

		// Push into array.
		MeshRuntimes_.emplace_back( std::move( MeshRuntime ) );
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
		for( size_t NodeIdx = 0; NodeIdx < pHeader_->NoofNodes_; ++NodeIdx )
		{
			ScnModelNodePropertyData* pNodePropertyNode = &pNodePropertyData_[ NodeIdx ];
			markupName( pNodePropertyNode->Name_ );
		}
	}
	else if( ChunkID == BcHash( "meshdata" ) )
	{
		pMeshData_ = (ScnModelMeshData*)pData;		
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
		new ReField( "Position_", &ScnModelComponent::Position_, bcRFF_IMPORTER ),
		new ReField( "Scale_", &ScnModelComponent::Scale_, bcRFF_IMPORTER ),
		new ReField( "Rotation_", &ScnModelComponent::Rotation_, bcRFF_IMPORTER ),
		
		new ReField( "Uniforms_", &ScnModelComponent::Uniforms_, bcRFF_TRANSIENT ),

		new ReField( "BaseTransform_", &ScnModelComponent::BaseTransform_ ),
		new ReField( "RenderMask_", &ScnModelComponent::RenderMask_, bcRFF_IMPORTER ),
		new ReField( "RenderPermutations_", &ScnModelComponent::RenderPermutations_, bcRFF_IMPORTER | bcRFF_FLAGS ),
		new ReField( "Passes_", &ScnModelComponent::Passes_, bcRFF_IMPORTER | bcRFF_FLAGS ),

		new ReField( "UploadFence_", &ScnModelComponent::UploadFence_, bcRFF_TRANSIENT ),
		new ReField( "pNodeTransformData_", &ScnModelComponent::pNodeTransformData_, bcRFF_TRANSIENT ),
		new ReField( "AABB_", &ScnModelComponent::AABB_, bcRFF_TRANSIENT ),
	};

	ReRegisterClass< ScnModelComponent, Super >( Fields )
		.addAttribute( new ScnModelProcessor() );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnModelComponent::ScnModelComponent():
	Model_(),
	Layer_( 0 ),
	Position_( 0.0f, 0.0f, 0.0f ),
	Scale_( 1.0f, 1.0f, 1.0f ),
	Rotation_( 0.0f, 0.0f, 0.0f ),
	RenderMask_( 1 ),
	RenderPermutations_( ScnShaderPermutationFlags::RENDER_FORWARD | ScnShaderPermutationFlags::RENDER_DEFERRED ),
	Passes_( RsRenderSortPassFlags::DEPTH | RsRenderSortPassFlags::OPAQUE | RsRenderSortPassFlags::SHADOW ),
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
// findNodeIndexByName
BcU32 ScnModelComponent::findNodeIndexByName( const BcName& Name ) const
{
	BcAssert( isReady() );
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
	BcAssert( isReady() );
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
	BcAssert( isReady() );
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	if( NodeIdx < NoofNodes )
	{
		pNodeTransformData_[ NodeIdx ].LocalTransform_ = LocalTransform;
	}
}

//////////////////////////////////////////////////////////////////////////
// getNodeLocalTransform
const MaMat4d& ScnModelComponent::getNodeLocalTransform( BcU32 NodeIdx ) const
{
	BcAssert( isReady() );
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	BcAssert( pNodeTransformData_ );
	if( NodeIdx < NoofNodes )
	{
		return pNodeTransformData_[ NodeIdx ].LocalTransform_;
	}

	static MaMat4d Default;
	return Default;
}

//////////////////////////////////////////////////////////////////////////
// getNodeWorldTransform
const MaMat4d& ScnModelComponent::getNodeWorldTransform( BcU32 NodeIdx ) const
{
	BcAssert( isReady() );
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	if( NodeIdx < NoofNodes )
	{
		return pNodeTransformData_[ NodeIdx ].WorldTransform_;
	}

	static MaMat4d Default;
	return Default;
}

//////////////////////////////////////////////////////////////////////////
// setUniforms
void ScnModelComponent::setUniforms( const ReClass* UniformClass, const void* UniformData )
{
	BcAssert( isReady() );
#if !PSY_PRODUCTION
	const std::string DebugName = getFullName();
	const char* DebugNameCStr = DebugName.c_str();
#else
	const char* DebugNameCStr = nullptr;
#endif

	auto FoundIt = std::find_if( Uniforms_.begin(), Uniforms_.end(),
		[ UniformClass ]( const ScnModelUniforms& Uniform )
		{
			return Uniform.Class_ == UniformClass;
		} );

	auto Size = UniformClass->getSize();
	if( FoundIt == Uniforms_.end() )
	{
		ScnModelUniforms Uniform;
		Uniform.Class_ = UniformClass;
		Uniform.Data_ = std::move( BcBinaryData( Size ) );
		Uniform.Buffer_ = RsCore::pImpl()->createBuffer( 
			RsBufferDesc(
				RsBindFlags::UNIFORM_BUFFER,
				RsResourceCreationFlags::STREAM,
				Size ), DebugNameCStr );	
		Uniforms_.emplace_back( std::move( Uniform ) );
		FoundIt = Uniforms_.end() - 1;

		// May need to rebind, so reset view data.
		ScnViewProcessor::pImpl()->resetViewRenderData( this );
	}

	// Copy into intermedate, or construct default.
	if( UniformData != nullptr )
	{
		memcpy( FoundIt->Data_.getData< BcU8 >(), UniformData, Size );
	}
	else
	{
		UniformClass->construct< BcU8 >( FoundIt->Data_.getData< BcU8 >() );
	}
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
	BcAssert( isReady() );
	const BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	return NoofNodes;
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
// updateNodes
void ScnModelComponent::updateNodes( const MaMat4d& RootMatrix )
{
	PSY_PROFILE_FUNCTION;
	BcAssert( isReady() );

	setBaseTransform( Position_, Scale_, Rotation_ );

	MaAABB FullAABB;

	// Update nodes.	
	BcU32 NoofNodes = Model_->pHeader_->NoofNodes_;
	for( BcU32 NodeIdx = 0; NodeIdx < NoofNodes; ++NodeIdx )
	{
		const ScnModelNodePropertyData* pNodePropertyData = &Model_->pNodePropertyData_[ NodeIdx ];
		ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ NodeIdx ];

		// Check parent index and process.
		if( pNodePropertyData->ParentIndex_ != BcErrorCode )
		{
			const ScnModelNodeTransformData* pParentScnModelNodeTransformData = &pNodeTransformData_[ pNodePropertyData->ParentIndex_ ];
			
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
		const ScnModelMeshData* pNodeMeshData = &Model_->pMeshData_[ pNodeMeshRuntime->MeshDataIdx_ ];
		TPerComponentMeshData& PerComponentMeshData = PerComponentMeshDataList_[ PrimitiveIdx ];

		// Special case the skinned models for now.
		if( pNodeMeshData->IsSkinned_ == BcFalse )
		{
			const ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pNodeMeshData->NodeIdx_ ];
			
			MaAABB AABB = pNodeMeshData->AABB_.transform( pNodeTransformData->WorldTransform_ );
			FullAABB.expandBy( AABB );
			PerComponentMeshData.AABB_ = AABB;
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
					const ScnModelNodePropertyData* pNodePropertyData = &Model_->pNodePropertyData_[ BoneIndex ];
					if( pNodePropertyData->ParentIndex_ != BcErrorCode )
					{
						const ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ BoneIndex ];
						const ScnModelNodeTransformData* pParentNodeTransformData = &pNodeTransformData_[ pNodePropertyData->ParentIndex_ ];
						MaAABB NewAABB;
						NewAABB.expandBy( pNodeTransformData->WorldTransform_.translation() );
						NewAABB.expandBy( pParentNodeTransformData->WorldTransform_.translation() );
						SkeletalAABB.expandBy( NewAABB );
					}
				}
			}

			if( !SkeletalAABB.isEmpty() )
			{
				// HACK: Expand AABB slightly to cover skin. Should calculate bone sizes and pack them really.
				MaVec3d Centre = SkeletalAABB.centre();
				MaVec3d Dimensions = SkeletalAABB.dimensions() * 0.75f;	// 1.5 x size.
				SkeletalAABB.min( Centre - Dimensions );
				SkeletalAABB.max( Centre + Dimensions );

				FullAABB.expandBy( SkeletalAABB );
				PerComponentMeshData.AABB_ = SkeletalAABB;
			}
		}
	}

	AABB_ = FullAABB;

	// Setup skinning buffers.
	for( BcU32 PrimitiveIdx = 0; PrimitiveIdx < NoofPrimitives; ++PrimitiveIdx )
	{
		ScnModelMeshRuntime* pNodeMeshRuntime = &Model_->MeshRuntimes_[ PrimitiveIdx ];
		ScnModelMeshData* pNodeMeshData = &Model_->pMeshData_[ pNodeMeshRuntime->MeshDataIdx_ ];
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
					ScnModelNodeTransformData* pNodeTransformData = &pNodeTransformData_[ pNodeMeshData->NodeIdx_ ];

					// World matrix.
					ObjectUniformBlock->WorldTransform_ = pNodeTransformData->WorldTransform_;

					// Normal matrix.
					ObjectUniformBlock->NormalTransform_ = pNodeTransformData->WorldTransform_;
					ObjectUniformBlock->NormalTransform_.translation( MaVec3d( 0.0f, 0.0f, 0.0f ) );
#if 0 // Normal when using non-uniform scaling are broken without this. Consider implementing it as optional?
					ObjectUniformBlock->NormalTransform_.inverse();
					ObjectUniformBlock->NormalTransform_.transpose();
#endif

					UploadFence_.decrement();
				} );
		}
	}
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

	// Create automatic uniform buffers from all materials where they haven't
	// been set already.
	ScnModelMeshRuntimeList& MeshRuntimes = Model_->MeshRuntimes_;
	PerComponentMeshDataList_.reserve( MeshRuntimes.size() );
	for( BcU32 Idx = 0; Idx < MeshRuntimes.size(); ++Idx )
	{
		ScnModelMeshRuntime* pMeshRuntime = &MeshRuntimes[ Idx ];
		auto Material = pMeshRuntime->MaterialRef_;

		auto AutomaticUniforms = Material->getAutomaticUniforms();
		for( auto AutomaticUniform : AutomaticUniforms )
		{
			const ReClass* UniformClass = ReManager::GetClass( AutomaticUniform );
			auto FoundIt = std::find_if( Uniforms_.begin(), Uniforms_.end(),
				[ UniformClass ]( const ScnModelUniforms& Uniform )
			{
				return Uniform.Class_ == UniformClass;
			} );

			if( FoundIt == Uniforms_.end() )
			{
				// TODO: Don't use nullptr (default), but get from material?
				setUniforms( UniformClass, nullptr );
			}
		}
	}

	// Create uniform buffers.
	for( auto& Uniform : Uniforms_ )
	{
		auto Data = Uniform.Data_.getData< BcU8 >();
		auto Size = Uniform.Data_.getDataSize();
		if( Uniform.Buffer_ == nullptr )
		{
			Uniform.Buffer_ = RsCore::pImpl()->createBuffer( 
				RsBufferDesc(
					RsBindFlags::UNIFORM_BUFFER,
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
