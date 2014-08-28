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

#ifdef PSY_SERVER
#include "System/Scene/Import/ScnMaterialImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterial );

void ScnMaterial::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pHeader_", &ScnMaterial::pHeader_, bcRFF_SHALLOW_COPY ),
		new ReField( "Shader_", &ScnMaterial::Shader_ ),
		new ReField( "TextureMap_", &ScnMaterial::TextureMap_ ),
		new ReField( "pStateBuffer_", &ScnMaterial::pStateBuffer_, bcRFF_SHALLOW_COPY ),
	};
		
	auto& Class = ReRegisterClass< ScnMaterial, Super >( Fields );

#ifdef PSY_SERVER
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( ScnMaterialImport::StaticGetClass() ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnMaterial::initialise()
{
	pHeader_ = NULL;
	pStateBuffer_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnMaterial::create()
{
	ScnMaterialTextureHeader* pTextureHeaders = (ScnMaterialTextureHeader*)( pHeader_ + 1 );
		
	// Get resources.
	Shader_ = getPackage()->getPackageCrossRef( pHeader_->ShaderRef_ );
	for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
	{
		ScnMaterialTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
		TextureMap_[ pTextureHeader->SamplerName_ ] = getPackage()->getPackageCrossRef( pTextureHeader->TextureRef_ );
	}
	
	// Mark as ready.
	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnMaterial::destroy()
{
	
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
	else if( ChunkID == BcHash( "stateblock" ) )
	{
		pStateBuffer_ = (BcU32*)pData;

		markCreate(); // All data loaded, time to create.
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterialComponent );
REFLECTION_DEFINE_BASIC( ScnMaterialComponent::TTextureBinding );
REFLECTION_DEFINE_BASIC( ScnMaterialComponent::TUniformBlockBinding );

void ScnMaterialComponent::StaticRegisterClass()
{
	{
		ReField* Fields[] = 
		{
			new ReField( "Parent_", &ScnMaterialComponent::Parent_, bcRFF_SHALLOW_COPY ),
			new ReField( "pProgram_", &ScnMaterialComponent::pProgram_, bcRFF_SHALLOW_COPY ),
			new ReField( "StateBuffer_", &ScnMaterialComponent::StateBuffer_ ),
			new ReField( "TextureBindingList_", &ScnMaterialComponent::TextureBindingList_ ),
			new ReField( "UniformBlockBindingList_", &ScnMaterialComponent::UniformBlockBindingList_ ),
			new ReField( "ViewUniformBlockIndex_", &ScnMaterialComponent::ViewUniformBlockIndex_ ),
			new ReField( "BoneUniformBlockIndex_", &ScnMaterialComponent::BoneUniformBlockIndex_ ),
			new ReField( "ObjectUniformBlockIndex_", &ScnMaterialComponent::ObjectUniformBlockIndex_ ),
		};
		ReRegisterClass< ScnMaterialComponent, Super >( Fields );
	}

	{
		ReField* Fields[] = 
		{
			new ReField( "Parent_", &TTextureBinding::Handle_ ),
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
// initialise
void ScnMaterialComponent::initialise( ScnMaterialRef Parent, ScnShaderPermutationFlags PermutationFlags )
{
	Super::initialise();

	BcAssert( Parent.isValid() && Parent->isReady() );
	
	PermutationFlags_ = PermutationFlags 
		| ScnShaderPermutationFlags::RENDER_FORWARD
		| ScnShaderPermutationFlags::PASS_MAIN;

	// Cache parent and program.
	Parent_ = Parent;
	pProgram_ = Parent->Shader_->getProgram( PermutationFlags_ );
	
	// Allocate state buffer and copy defaults in.
	StateBuffer_.resize( (BcU32)RsRenderStateType::MAX );
	BcMemCopy( &StateBuffer_[ 0 ], Parent->pStateBuffer_, sizeof( BcU32 ) * (BcU32)RsRenderStateType::MAX );

	// Build a binding list for textures.
	ScnTextureMap& TextureMap( Parent->TextureMap_ );
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
	ViewUniformBlockIndex_ = findUniformBlock( "ViewUniformBlock" );
	BoneUniformBlockIndex_ = findUniformBlock( "BoneUniformBlock" );
	ObjectUniformBlockIndex_ = findUniformBlock( "ObjectUniformBlock" );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnMaterialComponent::initialise( const Json::Value& Object )
{
	ScnMaterialRef MaterialRef = getPackage()->getPackageCrossRef( Object[ "material" ].asUInt() );
	ScnShaderPermutationFlags PermutationFlags = ScnShaderPermutationFlags::NONE;
	const BcChar* pPermutation = Object[ "permutation" ].asCString();

	if( BcStrCompare( pPermutation, "2d" ) )
	{
		PermutationFlags = ScnShaderPermutationFlags::MESH_STATIC_2D;
	}
	else if( BcStrCompare( pPermutation, "3d" ) )
	{
		PermutationFlags = ScnShaderPermutationFlags::MESH_STATIC_3D;
	}

	initialise( MaterialRef, PermutationFlags );
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ScnMaterialComponent::destroy()
{
	Parent_ = nullptr;
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
		BcPrintf( "ERROR: Unable to set texture for slot %x\n", Slot );
	}
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
#ifdef PSY_DEBUG
	if( Index == BcErrorCode )
	{
		//pProgram_->logShaders();
		BcBreakpoint;
	}
#endif

	auto& UniformBlockBinding = UniformBlockBindingList_[ Index ];
	UniformBlockBinding.UniformBuffer_ = UniformBuffer;
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
// setState
void ScnMaterialComponent::setState( RsRenderStateType State, BcU32 Value )
{
	if( State < RsRenderStateType::MAX )
	{
		StateBuffer_[ (BcU32)State ] = Value;
	}
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
// getMaterial
ScnMaterialRef ScnMaterialComponent::getMaterial()
{
	return Parent_;
}

//////////////////////////////////////////////////////////////////////////
// bind
class ScnMaterialComponentRenderNode: public RsRenderNode
{
public:
	ScnMaterialComponentRenderNode()
	{
		
	}

	virtual void render()
	{
		// Iterate over textures and bind.
		for( BcU32 Idx = 0; Idx < NoofTextures_; ++Idx )
		{
			RsTexture* pTexture = ppTextures_[ Idx ];
			RsTextureParams& TextureParams = pTextureParams_[ Idx ];
			pContext_->setSamplerState( TextureHandles_[ Idx ], TextureParams );
			pContext_->setTexture( TextureHandles_[ Idx ], pTexture );
		}

		// Setup states.
		for( BcU32 Idx = 0; Idx < (BcU32)RsRenderStateType::MAX; ++Idx )
		{
			pContext_->setRenderState( (RsRenderStateType)Idx, pStateBuffer_[ Idx ], BcFalse );
		}
	
		// Set uniform blocks.
		for( BcU32 Idx = 0; Idx < NoofUniformBlocks_; ++Idx )
		{
			BcU32 Index = pUniformBlockIndices_[ Idx ];
			RsBuffer* pUniformBuffer = ppUniformBuffers_[ Idx ];
			pContext_->setUniformBuffer( Index, pUniformBuffer );
		}

		// Set program.
		pContext_->setProgram( pProgram_ );

		// Done.
		pUpdateFence_->decrement();
	}
	
	RsProgram* pProgram_;

	// Texture binding block.
	BcU32 NoofTextures_;
	BcU32* TextureHandles_;
	RsTexture** ppTextures_;
	RsTextureParams* pTextureParams_;

	// State buffer.
	BcU32* pStateBuffer_;

	// Uniform blocks.
	BcU32 NoofUniformBlocks_;
	BcU32* pUniformBlockIndices_;
	RsBuffer** ppUniformBuffers_;
	
	// Update fence (for marking when in use/not)
	// TODO: Make this a generic feature of the component system?
	SysFence* pUpdateFence_;

	// For debugging.
	ScnMaterialComponent* pParent_;
};

void ScnMaterialComponent::bind( RsFrame* pFrame, RsRenderSort& Sort )
{
	BcAssertMsg( isAttached(), "Material \"%s\" needs to be attached to an entity!", (*getName()).c_str() );

	// Setup sort value with material specifics.
	ScnMaterial* pMaterial_ = Parent_;
	//Sort.MaterialID_ = BcU64( ( BcU32( pMaterial_ ) & 0xffff ) ^ ( BcU32( pMaterial_ ) >> 4 ) & 0xffff );			// revisit once canvas is fixed!
	Sort.Blend_ = StateBuffer_[ (BcU32)RsRenderStateType::BLEND_MODE ];
	
	// Allocate a render node.
	ScnMaterialComponentRenderNode* pRenderNode = pFrame->newObject< ScnMaterialComponentRenderNode >();
	
	// Debugging.
	pRenderNode->pParent_ = this;
	
	// Setup program and state.
	pRenderNode->pProgram_ = pProgram_;
	
	// Setup texture binding block.
	pRenderNode->NoofTextures_ = (BcU32)TextureBindingList_.size();
	pRenderNode->TextureHandles_ = (BcU32*)pFrame->allocMem( sizeof( BcU32 ) * pRenderNode->NoofTextures_ );
	pRenderNode->ppTextures_ = (RsTexture**)pFrame->allocMem( sizeof( RsTexture* ) * pRenderNode->NoofTextures_ );
	pRenderNode->pTextureParams_ = (RsTextureParams*)pFrame->allocMem( sizeof( RsTextureParams ) * pRenderNode->NoofTextures_ );
	
	for( BcU32 Idx = 0; Idx < pRenderNode->NoofTextures_; ++Idx )
	{
		auto& Binding = TextureBindingList_[ Idx ];
		RsTexture*& Texture = pRenderNode->ppTextures_[ Idx ];
		RsTextureParams& TextureParams = pRenderNode->pTextureParams_[ Idx ];
		
		// Sampler handles.
		pRenderNode->TextureHandles_[ Idx ] = Binding.Handle_;

		// Set texture to bind.
		Texture = Binding.Texture_->getTexture();
		
		// Default texture parameters.
		// TODO: Pull these from the material.
		RsTextureParams DefaultTextureParams = 
		{
			Texture->getDesc().Levels_ > 1 ? RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR : RsTextureFilteringMode::LINEAR,
			RsTextureFilteringMode::LINEAR,
			RsTextureSamplingMode::WRAP,
			RsTextureSamplingMode::WRAP
		};

		// Set texture params.
		TextureParams = DefaultTextureParams;
	}

	// Setup uniform blocks.
	pRenderNode->NoofUniformBlocks_ = (BcU32)UniformBlockBindingList_.size();
	pRenderNode->pUniformBlockIndices_ = (BcU32*)pFrame->allocMem( sizeof( BcU32* ) * pRenderNode->NoofUniformBlocks_ );
	pRenderNode->ppUniformBuffers_ = (RsBuffer**)pFrame->allocMem( sizeof( RsBuffer ) * pRenderNode->NoofUniformBlocks_ );

	for( BcU32 Idx = 0; Idx < UniformBlockBindingList_.size(); ++Idx )
	{
		pRenderNode->pUniformBlockIndices_[ Idx ] = UniformBlockBindingList_[ Idx ].Index_;
		pRenderNode->ppUniformBuffers_[ Idx ] = UniformBlockBindingList_[ Idx ].UniformBuffer_;
	}
	
	// Setup state buffer.
	pRenderNode->pStateBuffer_ = (BcU32*)pFrame->allocMem( sizeof( BcU32 ) * (BcU32)RsRenderStateType::MAX );
	BcMemCopy( pRenderNode->pStateBuffer_, &StateBuffer_[ 0 ], sizeof( BcU32 ) * (BcU32)RsRenderStateType::MAX );
	
	// Update fence.
	pRenderNode->pUpdateFence_ = &UpdateFence_;
	UpdateFence_.increment();

	// Add node to frame.
	pRenderNode->Sort_ = Sort;
	pFrame->addRenderNode( pRenderNode );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnMaterialComponent::update( BcF32 Tick )
{
	ScnComponent::update( Tick );
}

//////////////////////////////////////////////////////////////////////////
// onAttach
//virtual
void ScnMaterialComponent::onAttach( ScnEntityWeakRef Parent )
{
	ScnComponent::onAttach( Parent );
}

//////////////////////////////////////////////////////////////////////////
// onDetach
//virtual
void ScnMaterialComponent::onDetach( ScnEntityWeakRef Parent )
{
	UpdateFence_.wait();

	ScnComponent::onDetach( Parent );
}
