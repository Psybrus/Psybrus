/**************************************************************************
*
* File:		ScnMaterial.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Rendering/ScnMaterial.h"
#include "System/Scene/ScnEntity.h"
#include "System/Content/CsCore.h"
#include "System/Os/OsCore.h"

#include "System/Debug/DsCore.h"

#include "System/Renderer/RsRenderNode.h"


#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnMaterialImport.h"
#endif

#include "Editor/Editor.h"

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnMaterial );

void ScnMaterial::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pHeader_", &ScnMaterial::pHeader_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
		new ReField( "Shader_", &ScnMaterial::Shader_, bcRFF_SHALLOW_COPY ),
		new ReField( "TextureMap_", &ScnMaterial::TextureMap_ ),
		new ReField( "RenderStateDesc_", &ScnMaterial::RenderStateDesc_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
	};
	
	auto& Class = ReRegisterClass< ScnMaterial, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnMaterialImport::StaticGetClass(), 0 ) );
#endif

	// Add editor.
	Class.addAttribute( 
		new DsImGuiFieldEditor( 
			[]( DsImGuiFieldEditor* ThisFieldEditor, std::string Name, void* Object, const ReClass* Class, ReFieldFlags Flags )
			{
				auto Material = static_cast< ScnMaterial* >( Object );
				if( ImGui::TreeNode( Material, "%s", Name.c_str() ) )
				{
					ImGui::ScopedID ScopedID( Material );
					for( auto Iter( Material->AutomaticUniformBlocks_.begin() ); Iter != Material->AutomaticUniformBlocks_.end(); ++Iter )
					{
						if( ImGui::TreeNode( Material, "Uniform Block: %s", (*Iter->first).c_str() ) )
						{
							auto Data = Iter->second.getData< BcU8 >();
							auto DataSize = Iter->second.getDataSize();

							ImGui::ScopedID ScopedID2( Data );

							Editor::ObjectEditor(
								ThisFieldEditor, 
								Data,
								ReManager::GetClass( Iter->first ), Flags );

							// Update uniform buffer.
							RsCore::pImpl()->updateBuffer(
								Material->AutomaticUniformBuffers_[ Iter->first ].get(),
								0, Iter->second.getDataSize(),
								RsResourceUpdateFlags::ASYNC,
								[ Data, DataSize ]
								( RsBuffer* Buffer, RsBufferLock Lock )
								{
									memcpy( Lock.Buffer_, Data, DataSize );
								} );
							ImGui::TreePop();
						}
					}
					ImGui::TreePop();
				}
			} ) );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterial::ScnMaterial():
	pHeader_( nullptr ),
	RenderStateDesc_( nullptr )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnMaterial::~ScnMaterial()
{

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnMaterial::create()
{
	ScnMaterialTextureHeader* pTextureHeaders = (ScnMaterialTextureHeader*)( pHeader_ + 1 );
	ScnMaterialUniformBlockName* pAutomaticUniformBlockNames = (ScnMaterialUniformBlockName*)( pTextureHeaders + pHeader_->NoofTextures_ );
		
	// Get resources.
	Shader_ = getPackage()->getCrossRefResource( pHeader_->ShaderRef_ );
	for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
	{
		ScnMaterialTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
		TextureMap_[ pTextureHeader->SamplerName_ ] = getPackage()->getCrossRefResource( pTextureHeader->TextureRef_ );
	}
	
	// Create render state.
	RenderState_ = RsCore::pImpl()->createRenderState( *RenderStateDesc_, getFullName().c_str() );

	// Create sampler states.
	for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
	{
		ScnMaterialTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
		auto SamplerState = RsCore::pImpl()->createSamplerState( pTextureHeader->SamplerStateDesc_, getFullName().c_str() );
		SamplerStateMap_[ pTextureHeader->SamplerName_ ] = std::move( SamplerState );
	}

	// Create uniforms.
	for( BcU32 Idx = 0; Idx < pHeader_->NoofAutomaticUniformBlocks_; ++Idx )
	{
		ScnMaterialUniformBlockName& UniformBlockName = pAutomaticUniformBlockNames[ Idx ];
		markupName( UniformBlockName.Name_ );
		auto Class = ReManager::GetClass( UniformBlockName.Name_ );
		if( Class )
		{
			auto UniformBuffer = createUniformBuffer( Class, getFullName().c_str() );
			auto UniformBlock = BcBinaryData( Class->getSize() );
			auto Data = UniformBlock.getData< BcU8 >();
			auto DataSize = Class->getSize();

			// Construct with default values.
			Class->construct< BcU8 >( UniformBlock.getData< BcU8 >() );

			// Update uniform buffer.
			RsCore::pImpl()->updateBuffer(
				UniformBuffer.get(),
				0, DataSize,
				RsResourceUpdateFlags::ASYNC,
				[ Data, DataSize ]
				( RsBuffer* Buffer, RsBufferLock Lock )
				{
					memcpy( Lock.Buffer_, Data, DataSize );
				} );

			// Move into containers.
			AutomaticUniformBuffers_.insert( std::make_pair( UniformBlockName.Name_, std::move( UniformBuffer ) ) );
			AutomaticUniformBlocks_.insert( std::make_pair( UniformBlockName.Name_, std::move( UniformBlock ) ) );
		}
		else
		{
			PSY_LOG( "ERROR: Uniform block \"%s\" does not exists. Check automatic uniform names on \"%s\"", 
				(*UniformBlockName.Name_).c_str(), getFullName().c_str() );
		}
	}

	// Mark as ready.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnMaterial::destroy()
{
	RenderState_.reset();
	SamplerStateMap_.clear();
}

//////////////////////////////////////////////////////////////////////////
// getTexture
ScnTextureRef ScnMaterial::getTexture( BcName Name )
{
	return TextureMap_[ Name ];
}

//////////////////////////////////////////////////////////////////////////
// hasPermutation
bool ScnMaterial::hasPermutation( ScnShaderPermutationFlags PermutationFlags )
{
	return Shader_->hasPermutation( PermutationFlags );
}

//////////////////////////////////////////////////////////////////////////
// getProgram
RsProgram* ScnMaterial::getProgram( ScnShaderPermutationFlags PermutationFlags )
{
	return Shader_->getProgram( PermutationFlags );
}

//////////////////////////////////////////////////////////////////////////
// getProgramBinding
RsProgramBindingDesc ScnMaterial::getProgramBinding( ScnShaderPermutationFlags PermutationFlags )
{
	auto* Program = Shader_->getProgram( PermutationFlags );
	BcAssert( Program );

	RsProgramBindingDesc ProgramBindingDesc;

	for( auto Iter( TextureMap_.begin() ); Iter != TextureMap_.end(); ++Iter )
	{
		const BcName& SamplerName = (*Iter).first;
		ScnTextureRef Texture = (*Iter).second;
		if( Texture )
		{
			BcU32 Slot = Program->findShaderResourceSlot( (*SamplerName).c_str() );
			if( Slot != BcErrorCode )
			{
				ProgramBindingDesc.setShaderResourceView( Slot, Texture->getTexture() );
			}
		}
	}

	for( auto Iter( SamplerStateMap_.begin() ); Iter != SamplerStateMap_.end(); ++Iter )
	{
		const BcName& SamplerName = (*Iter).first;
		RsSamplerState* Sampler = (*Iter).second.get();

		BcU32 Slot = Program->findSamplerSlot( (*SamplerName).c_str() );
		if( Slot != BcErrorCode )
		{
			ProgramBindingDesc.setSamplerState( Slot, Sampler );
		}
	}

	for( auto Iter( AutomaticUniformBuffers_.begin() ); Iter != AutomaticUniformBuffers_.end(); ++Iter )
	{
		const BcName& UniformBufferName = (*Iter).first;
		RsBuffer* Buffer = (*Iter).second.get();
	
		BcU32 Slot = Program->findUniformBufferSlot( (*UniformBufferName).c_str() );
		if( Slot != BcErrorCode )
		{
			ProgramBindingDesc.setUniformBuffer( Slot, Buffer, 0, Buffer->getDesc().SizeBytes_ );
		}
	}

	return ProgramBindingDesc;
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
RsRenderState* ScnMaterial::getRenderState()
{
	return RenderState_.get();
}

//////////////////////////////////////////////////////////////////////////
// getAutomaticUniforms
std::vector< BcName > ScnMaterial::getAutomaticUniforms() const
{
	std::vector< BcName > Uniforms;
	Uniforms.reserve( AutomaticUniformBlocks_.size() );
	for( auto It : AutomaticUniformBlocks_ )
	{
		Uniforms.emplace_back( It.first );
	}
	return Uniforms;
}

//////////////////////////////////////////////////////////////////////////
// createUniformBuffer
RsBufferUPtr ScnMaterial::createUniformBuffer( const ReClass* UniformBuffer, const BcChar* DebugName ) const
{
#if !PSY_PRODUCTION
	BcAssert( DebugName != nullptr && DebugName[0] != '\0' );
	std::string LongDebugName = ( getFullName() + "/" + DebugName + "/" + *UniformBuffer->getName() );
	const char* DebugNameCStr = LongDebugName.c_str();
#else
	const char* DebugNameCStr = DebugName;
#endif
	auto Buffer = RsCore::pImpl()->createBuffer( 
		RsBufferDesc(
			RsBindFlags::UNIFORM_BUFFER,
			RsResourceCreationFlags::STREAM,
			UniformBuffer->getSize() ), DebugNameCStr );	

	// Put default data into buffer.
	// NOTE: Inefficient.
	RsCore::pImpl()->updateBuffer( Buffer.get(), 0, UniformBuffer->getSize(), RsResourceUpdateFlags::ASYNC,
		[ UniformBuffer ]( RsBuffer* Buffer, const RsBufferLock& BufferLock )
		{
			auto Data = UniformBuffer->create< void >();
			BcMemCopy( BufferLock.Buffer_, Data, UniformBuffer->getSize() );
			UniformBuffer->destroy( Data );
		} );

	return Buffer;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnMaterial::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnMaterial::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		requestChunk( 0 );
		return;
	}
	
	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = (ScnMaterialHeader*)pData;
		ScnMaterialTextureHeader* pTextureHeaders = (ScnMaterialTextureHeader*)( pHeader_ + 1 );

		// Markup names now.
		// TODO: Automate this process with reflection!
		for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
		{
			ScnMaterialTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
			markupName( pTextureHeader->SamplerName_ );
		}

		requestChunk( ++ChunkIdx );
	}
	else if( ChunkID == BcHash( "renderstate" ) )
	{
		RenderStateDesc_ = (const RsRenderStateDesc*)pData;

		markCreate(); // All data loaded, time to create.
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnMaterialComponent );

void ScnMaterialComponent::StaticRegisterClass()
{
	{
		ReField* Fields[] = 
		{
			new ReField( "Material_", &ScnMaterialComponent::Material_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
			new ReField( "PermutationFlags_", &ScnMaterialComponent::PermutationFlags_, bcRFF_IMPORTER ),

			new ReField( "pProgram_", &ScnMaterialComponent::pProgram_, bcRFF_SHALLOW_COPY ),
			new ReField( "TextureMap_", &ScnMaterialComponent::TextureMap_, bcRFF_SHALLOW_COPY ),
			new ReField( "ViewUniformBlockIndex_", &ScnMaterialComponent::ViewUniformBlockIndex_, bcRFF_CONST ),
			new ReField( "BoneUniformBlockIndex_", &ScnMaterialComponent::BoneUniformBlockIndex_, bcRFF_CONST ),
			new ReField( "ObjectUniformBlockIndex_", &ScnMaterialComponent::ObjectUniformBlockIndex_, bcRFF_CONST ),
		};
		ReRegisterClass< ScnMaterialComponent, Super >( Fields );
	}

	{
		ReField* Fields[] = 
		{
			new ReField( "Material_", &TTextureBinding::Handle_ ),
			new ReField( "pProgram_", &TTextureBinding::Texture_, bcRFF_SHALLOW_COPY ),
		};
		ReRegisterClass< TTextureBinding >( Fields );
	}

	{
		ReField* Fields[] = 
		{
			new ReField( "Index_", &TUniformBlockBinding::Index_ ),
			new ReField( "UniformBuffer_", &TUniformBlockBinding::UniformBuffer_, bcRFF_SHALLOW_COPY ),
		};
		ReRegisterClass< TUniformBlockBinding >( Fields );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialComponent::ScnMaterialComponent():
	ScnMaterialComponent( nullptr, ScnShaderPermutationFlags::NONE )
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialComponent::ScnMaterialComponent( ScnMaterialRef Material, ScnShaderPermutationFlags PermutationFlags ):
	Material_( Material ),
	PermutationFlags_( PermutationFlags ),
	pProgram_( nullptr ),
	ViewUniformBlockIndex_(),
	BoneUniformBlockIndex_(),
	ObjectUniformBlockIndex_()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialComponent::ScnMaterialComponent( ScnMaterialComponent* Parent ):
	Material_( Parent->Material_ ),
	PermutationFlags_( Parent->PermutationFlags_ ),
	pProgram_( Parent->pProgram_ ),
	ViewUniformBlockIndex_( Parent->ViewUniformBlockIndex_ ),
	BoneUniformBlockIndex_( Parent->BoneUniformBlockIndex_ ),
	ObjectUniformBlockIndex_( Parent->ObjectUniformBlockIndex_ )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnMaterialComponent::~ScnMaterialComponent()
{

}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnMaterialComponent::initialise()
{
	if( Material_ && pProgram_ == nullptr )
	{
		BcAssertMsg( Material_->isReady(), 
			"Material is not ready for use. Possible cause is trying to use a material with a component from within the same package. "
			"Known issue, can be worked around by moving the material $(ScnMaterial:%s.%s) into another package to allow "
			"it to be fully loaded. This will hopefully be fixed by #118.  "
			"Another possible cause is due to not using the bcRFF_SHALLOW_COPY flag on an imported member field. ",
			(*Material_->getPackageName()).c_str(), (*Material_->getName()).c_str() );
		pProgram_ = Material_->Shader_->getProgram( PermutationFlags_ );
		BcAssert( pProgram_ != nullptr );
		
		// Build a binding list for textures.
		auto& TextureMap( Material_->TextureMap_ );
		for( auto Iter( TextureMap.begin() ); Iter != TextureMap.end(); ++Iter )
		{
			const BcName& SamplerName = (*Iter).first;
			ScnTextureRef Texture = (*Iter).second;

			BcU32 SamplerIdx = findTextureSlot( SamplerName );
			if( SamplerIdx != BcErrorCode )
			{
				setTexture( SamplerIdx, Texture );
			}
		}

		// Build a binding list for samplers.
		auto& SamplerMap( Material_->SamplerStateMap_ );
		for( auto Iter( SamplerMap.begin() ); Iter != SamplerMap.end(); ++Iter )
		{
			const BcName& SamplerName = (*Iter).first;
			RsSamplerState* Sampler = (*Iter).second.get();

			BcU32 SamplerIdx = findTextureSlot( SamplerName );
			if( SamplerIdx != BcErrorCode )
			{
				setSamplerState( SamplerIdx, Sampler );
			}
		}

		// Grab uniform blocks.
		ViewUniformBlockIndex_ = findUniformBlock( "ScnShaderViewUniformBlockData" );
		BoneUniformBlockIndex_ = findUniformBlock( "ScnShaderBoneUniformBlockData" );
		ObjectUniformBlockIndex_ = findUniformBlock( "ScnShaderObjectUniformBlockData" );
	}
	// We've been setup, but we need to set textures & samplers up.
	else if( Material_ && pProgram_ )
	{
		// Build a binding list for textures.
		auto& TextureMap( TextureMap_ );
		for( auto Iter( TextureMap.begin() ); Iter != TextureMap.end(); ++Iter )
		{
			BcU32 SamplerIdx = (*Iter).first;
			ScnTextureRef Texture = (*Iter).second;
			if( SamplerIdx != BcErrorCode )
			{
				setTexture( SamplerIdx, Texture );
			}
		}

		// Build a binding list for samplers.
		auto& SamplerMap( Material_->SamplerStateMap_ );
		for( auto Iter( SamplerMap.begin() ); Iter != SamplerMap.end(); ++Iter )
		{
			const BcName& SamplerName = (*Iter).first;
			RsSamplerState* Sampler = (*Iter).second.get();

			BcU32 SamplerIdx = findTextureSlot( SamplerName );
			if( SamplerIdx != BcErrorCode )
			{
				setSamplerState( SamplerIdx, Sampler );
			}
		}

		// Grab uniform blocks.
		ViewUniformBlockIndex_ = findUniformBlock( "ScnShaderViewUniformBlockData" );
		BoneUniformBlockIndex_ = findUniformBlock( "ScnShaderBoneUniformBlockData" );
		ObjectUniformBlockIndex_ = findUniformBlock( "ScnShaderObjectUniformBlockData" );
	}
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ScnMaterialComponent::destroy()
{
	Material_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// findTextureSlot
BcU32 ScnMaterialComponent::findTextureSlot( const BcName& TextureName )
{
	return pProgram_->findShaderResourceSlot( (*TextureName).c_str() );
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setTexture( BcU32 Slot, ScnTextureRef Texture )
{
	PSY_LOGSCOPEDCATEGORY( ScnMaterialComponent );

	// Find the texture slot to put this in.
	if( Slot != BcErrorCode )
	{
		if( ProgramBindingDesc_.setShaderResourceView( Slot, Texture ? Texture->getTexture() : nullptr ) )
		{
			TextureMap_[ Slot ] = Texture;
			ProgramBinding_.reset();
		}
	}
	else
	{
		PSY_LOG( "ERROR: Unable to set texture in \"%s\"\n", getFullName().c_str() );
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setTexture( const BcName& TextureName, ScnTextureRef Texture )
{
	setTexture( findTextureSlot( TextureName ), Texture );
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void ScnMaterialComponent::setSamplerState( BcU32 Slot, RsSamplerState* Sampler )
{
	if( Slot != BcErrorCode )
	{
		if( ProgramBindingDesc_.setSamplerState( Slot, Sampler ) )
		{
			ProgramBinding_.reset();
		}
	}
	else
	{
		PSY_LOG( "ERROR: Unable to set sampler state in \"%s\"\n", getFullName().c_str() );
	}
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void ScnMaterialComponent::setSamplerState( const BcName& TextureName, RsSamplerState* Sampler )
{
	setSamplerState( findTextureSlot( TextureName ), Sampler );
}

//////////////////////////////////////////////////////////////////////////
// findUniformBlock
BcU32 ScnMaterialComponent::findUniformBlock( const BcName& UniformBlockName )
{
	return pProgram_->findUniformBufferSlot( (*UniformBlockName).c_str() );
}

//////////////////////////////////////////////////////////////////////////
// setUniformBlock
void ScnMaterialComponent::setUniformBlock( BcU32 Slot, RsBuffer* UniformBuffer )
{
	if( Slot != BcErrorCode )
	{
		if( ProgramBindingDesc_.setUniformBuffer( Slot, UniformBuffer, 0, UniformBuffer->getDesc().SizeBytes_ ) )
		{
			ProgramBinding_.reset();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setUniformBlock
void ScnMaterialComponent::setUniformBlock( const BcName& UniformBlockName, RsBuffer* UniformBuffer )
{
	setUniformBlock( findUniformBlock( UniformBlockName ), UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// setViewUniformBlock
void ScnMaterialComponent::setViewUniformBlock( RsBuffer* UniformBuffer )
{
	setUniformBlock( ViewUniformBlockIndex_, UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// setBoneUniformBlock
void ScnMaterialComponent::setBoneUniformBlock( RsBuffer* UniformBuffer )
{
	setUniformBlock( BoneUniformBlockIndex_, UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// setObjectUniformBlock
void ScnMaterialComponent::setObjectUniformBlock( RsBuffer* UniformBuffer )
{
	setUniformBlock( ObjectUniformBlockIndex_, UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// getProgramBinding
RsProgramBinding* ScnMaterialComponent::getProgramBinding()
{
	if( ProgramBinding_ == nullptr )
	{
		OsCore::pImpl()->unsubscribeAll( this );

		// If the texture is client dependent, resubscribe for the binding to be recreated.
		for( auto TextureEntry : TextureMap_ )
		{
			if( TextureEntry.second && TextureEntry.second->isClientDependent() )
			{
				OsCore::pImpl()->subscribe( osEVT_CLIENT_RESIZE, this,
					[ this ]( EvtID, const EvtBaseEvent& )->eEvtReturn
					{
						ProgramBinding_.reset();
						return evtRET_PASS;
					} );
				break;
			}
		}

		ProgramBinding_ = RsCore::pImpl()->createProgramBinding( pProgram_, ProgramBindingDesc_, getFullName().c_str() );
	}
	return ProgramBinding_.get();
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
RsRenderState* ScnMaterialComponent::getRenderState()
{
	return Material_->RenderState_.get();
}

//////////////////////////////////////////////////////////////////////////
// getTexture
ScnTextureRef ScnMaterialComponent::getTexture( BcU32 Slot )
{
	if( TextureMap_.find( Slot ) != TextureMap_.end() )
	{
		return TextureMap_[ Slot ];
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// getTexture
ScnTextureRef ScnMaterialComponent::getTexture( const BcName& TextureName )
{
	return getTexture( findTextureSlot( TextureName ) );
}
 
//////////////////////////////////////////////////////////////////////////
// getMaterial
ScnMaterialRef ScnMaterialComponent::getMaterial()
{
	return Material_;
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnMaterialComponent::onAttach( ScnEntityWeakRef Parent )
{
	Super::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnMaterialComponent::onDetach( ScnEntityWeakRef Parent )
{
	ProgramBinding_.reset();

	Super::onDetach( Parent );
}
