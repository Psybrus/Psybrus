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

#include "System/Renderer/RsRenderNode.h"

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnMaterialImport.h"
#endif

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
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterial::ScnMaterial():
	pHeader_( nullptr )
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
		
	// Get resources.
	Shader_ = getPackage()->getCrossRefResource( pHeader_->ShaderRef_ );
	for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
	{
		ScnMaterialTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
		TextureMap_[ pTextureHeader->SamplerName_ ] = getPackage()->getCrossRefResource( pTextureHeader->TextureRef_ );
	}
	
	// Create render state.
	RenderState_ = RsCore::pImpl()->createRenderState( *RenderStateDesc_ );

	// Create sampler states.
	SamplerStates_.reserve( pHeader_->NoofTextures_ );
	for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
	{
		ScnMaterialTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
		auto SamplerState = RsCore::pImpl()->createSamplerState( pTextureHeader->SamplerStateDesc_ );
		SamplerStates_.emplace_back( std::move( SamplerState ) );
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
	SamplerStates_.clear();
}

//////////////////////////////////////////////////////////////////////////
// getTexture
ScnTextureRef ScnMaterial::getTexture( BcName Name )
{
	return TextureMap_[ Name ];
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
REFLECTION_DEFINE_BASIC( ScnMaterialComponent::TTextureBinding );
REFLECTION_DEFINE_BASIC( ScnMaterialComponent::TUniformBlockBinding );

void ScnMaterialComponent::StaticRegisterClass()
{
	{
		ReField* Fields[] = 
		{
			new ReField( "Material_", &ScnMaterialComponent::Material_, bcRFF_SHALLOW_COPY | bcRFF_IMPORTER ),
			new ReField( "PermutationFlags_", &ScnMaterialComponent::PermutationFlags_, bcRFF_IMPORTER ),

			new ReField( "pProgram_", &ScnMaterialComponent::pProgram_, bcRFF_SHALLOW_COPY ),
			new ReField( "TextureBindingList_", &ScnMaterialComponent::TextureBindingList_, bcRFF_CONST ),
			new ReField( "UniformBlockBindingList_", &ScnMaterialComponent::UniformBlockBindingList_, bcRFF_CONST ),
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
	TextureBindingList_(),
	UniformBlockBindingList_(),
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
	TextureBindingList_( Parent->TextureBindingList_),
	UniformBlockBindingList_( Parent->UniformBlockBindingList_ ),
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
	PermutationFlags_ = PermutationFlags_ | ScnShaderPermutationFlags::RENDER_FORWARD | ScnShaderPermutationFlags::PASS_MAIN;
	if( Material_ && pProgram_ == nullptr )
	{
		pProgram_ = Material_->Shader_->getProgram( PermutationFlags_ );
		BcAssert( pProgram_ != nullptr );
		
		// Build a binding list for textures.
		ScnTextureMap& TextureMap( Material_->TextureMap_ );
		for( ScnTextureMapConstIterator Iter( TextureMap.begin() ); Iter != TextureMap.end(); ++Iter )
		{
			const BcName& SamplerName = (*Iter).first;
			ScnTextureRef Texture = (*Iter).second;

			BcU32 SamplerIdx = findTextureSlot( SamplerName );
			if( SamplerIdx != BcErrorCode )
			{
				setTexture( SamplerIdx, Texture );
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
	// TODO: Improve this, also store parameter info in parent material to
	//       save memory and move look ups to it's own creation.
	BcU32 Handle = pProgram_->findTextureSlot( (*TextureName).c_str() );
	
	if( Handle != BcErrorCode )
	{
		for( BcU32 Idx = 0; Idx < TextureBindingList_.size(); ++Idx )
		{
			auto& Binding = TextureBindingList_[ Idx ];
			
			if( Binding.Handle_ == Handle )
			{
				return Idx;
			}
		}
		
		// If it doesn't exist, add it.
		TTextureBinding Binding;
		Binding.Handle_ = Handle;
		Binding.Texture_ = nullptr;
		
		TextureBindingList_.push_back( Binding );
		return (BcU32)TextureBindingList_.size() - 1;
	}
	
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setTexture( BcU32 Slot, ScnTextureRef Texture )
{
	// Find the texture slot to put this in.
	if( Slot < TextureBindingList_.size() )
	{
		auto& TexBinding( TextureBindingList_[ Slot ] );
		TexBinding.Texture_ = Texture;
	}
	else
	{
		PSY_LOG( "ERROR: Unable to set texture for slot %x\n", Slot );
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setTexture( const BcName& TextureName, ScnTextureRef Texture )
{
	setTexture( findTextureSlot( TextureName ), Texture );
}

//////////////////////////////////////////////////////////////////////////
// findUniformBlock
BcU32 ScnMaterialComponent::findUniformBlock( const BcName& UniformBlockName )
{
	BcU32 Index = pProgram_->findUniformBufferSlot( (*UniformBlockName).c_str() );
	if( Index != BcErrorCode )
	{
		for( BcU32 Idx = 0; Idx < UniformBlockBindingList_.size(); ++Idx )
		{
			auto& Binding = UniformBlockBindingList_[ Idx ];
			
			if( Binding.Index_ == Index )
			{
				return Idx;
			}
		}
		
		// If it doesn't exist, add it.
		TUniformBlockBinding Binding;
		Binding.Index_ = Index;
		Binding.UniformBuffer_ = nullptr;
		
		UniformBlockBindingList_.push_back( Binding );
		return (BcU32)UniformBlockBindingList_.size() - 1;
	}
	
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setUniformBlock
void ScnMaterialComponent::setUniformBlock( BcU32 Index, RsBuffer* UniformBuffer )
{
	if( Index == BcErrorCode )
	{
		//PSY_LOG( "Error: Attempting to set uniform buffer to invalid slot." );
		return;
	}

	auto& UniformBlockBinding = UniformBlockBindingList_[ Index ];
	UniformBlockBinding.UniformBuffer_ = UniformBuffer;
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
// getTexture
ScnTextureRef ScnMaterialComponent::getTexture( BcU32 Idx )
{
	if( Idx < TextureBindingList_.size() )
	{
		return TextureBindingList_[ Idx ].Texture_;
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
// bind
struct ScnMaterialComponentRenderData
{
	// Texture binding block.
	BcU32 NoofTextures_;
	BcU32* TextureHandles_;
	RsTexture** ppTextures_;
	RsSamplerState** ppSamplerStates_;

	// Uniform blocks.
	BcU32 NoofUniformBlocks_;
	BcU32* pUniformBlockIndices_;
	RsBuffer** ppUniformBuffers_;

	// State.
	RsRenderState* RenderState_;

	// Program.
	RsProgram* pProgram_;
	
	// For debugging.
	ScnMaterialComponent* pMaterial_;
};

void ScnMaterialComponent::bind( RsFrame* pFrame, RsRenderSort& Sort )
{
	BcAssertMsg( isAttached(), "Material \"%s\" needs to be attached to an entity!", (*getName()).c_str() );

	// Setup sort value with material specifics.
	//ScnMaterial* pMaterial_ = Material_;
	//Sort.MaterialID_ = BcU64( ( BcU32( pMaterial_ ) & 0xffff ) ^ ( BcU32( pMaterial_ ) >> 4 ) & 0xffff );			// revisit once canvas is fixed!
	Sort.Blend_ = 0; //StateBuffer_[ (BcU32)RsRenderStateType::BLEND_MODE ];
	
	ScnMaterialComponentRenderData Data;
	Data.pMaterial_ = this;
		
	// Setup texture binding block.
	Data.NoofTextures_ = (BcU32)TextureBindingList_.size();
	Data.TextureHandles_ = (BcU32*)pFrame->allocMem( sizeof( BcU32 ) * Data.NoofTextures_ );
	Data.ppTextures_ = (RsTexture**)pFrame->allocMem( sizeof( RsTexture* ) * Data.NoofTextures_ );
	Data.ppSamplerStates_ = (RsSamplerState**)pFrame->allocMem( sizeof( RsSamplerState* ) * Data.NoofTextures_ );
	
	for( BcU32 Idx = 0; Idx < Data.NoofTextures_; ++Idx )
	{
		auto& Binding = TextureBindingList_[ Idx ];
		RsTexture*& Texture = Data.ppTextures_[ Idx ];
		RsSamplerState*& SamplerState = Data.ppSamplerStates_[ Idx ];
		
		// Sampler handles.
		Data.TextureHandles_[ Idx ] = Binding.Handle_;

		// Set texture to bind.
		if( Binding.Texture_ != nullptr )
		{
			Texture = Binding.Texture_->getTexture();
		}
		else
		{
			Texture = nullptr;
		}

		// Set sampler state.
		SamplerState = Material_->SamplerStates_[ Idx ].get();
	}

	// Setup uniform blocks.
	Data.NoofUniformBlocks_ = (BcU32)UniformBlockBindingList_.size();
	Data.pUniformBlockIndices_ = (BcU32*)pFrame->allocMem( sizeof( BcU32* ) * Data.NoofUniformBlocks_ );
	Data.ppUniformBuffers_ = (RsBuffer**)pFrame->allocMem( sizeof( RsBuffer* ) * Data.NoofUniformBlocks_ );

	for( BcU32 Idx = 0; Idx < UniformBlockBindingList_.size(); ++Idx )
	{
		Data.pUniformBlockIndices_[ Idx ] = UniformBlockBindingList_[ Idx ].Index_;
		Data.ppUniformBuffers_[ Idx ] = UniformBlockBindingList_[ Idx ].UniformBuffer_;
	}
	
	// Setup state buffer.
	Data.RenderState_ = Material_->RenderState_.get();
	
	// Setup program.
	Data.pProgram_ = pProgram_;

	// Add node to frame.
	pFrame->queueRenderNode( Sort,
		[ this, Data ]( RsContext* Context )
		{
			// Iterate over textures and bind.
			for( BcU32 Idx = 0; Idx < Data.NoofTextures_; ++Idx )
			{
				RsTexture* pTexture = Data.ppTextures_[ Idx ];
				RsSamplerState* pSamplerState = Data.ppSamplerStates_[ Idx ];
				Context->setTexture( Data.TextureHandles_[ Idx ], pTexture );
				Context->setSamplerState( Data.TextureHandles_[ Idx ], pSamplerState );
			}
			
			// Set uniform blocks.
			for( BcU32 Idx = 0; Idx < Data.NoofUniformBlocks_; ++Idx )
			{
				BcU32 Index = Data.pUniformBlockIndices_[ Idx ];
				RsBuffer* pUniformBuffer = Data.ppUniformBuffers_[ Idx ];
				if( pUniformBuffer )
				{
					Context->setUniformBuffer( Index, pUniformBuffer );
				}
			}

			// Setup state.
			Context->setRenderState( Data.RenderState_ );

			// Set program.
			Context->setProgram( Data.pProgram_ );
		} );
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
	Super::onDetach( Parent );
}
