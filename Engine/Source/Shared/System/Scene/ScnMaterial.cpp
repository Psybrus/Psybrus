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

#include "System/Scene/ScnMaterial.h"
#include "System/Scene/ScnEntity.h"
#include "System/Content/CsCore.h"

#include "System/Scene/ScnTextureAtlas.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnMaterial::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	const Json::Value& ImportShader = Object[ "shader" ];
	const Json::Value& ImportTextures = Object[ "textures" ];
	const Json::Value& State = Object[ "state" ];
	
	// Export material.
	BcStream HeaderStream;
	BcStream StateBlockStream;
		
	ScnMaterialHeader Header;
	ScnMaterialTextureHeader TextureHeader;
		
	// Make header.
	Json::Value::Members TextureMembers = ImportTextures.getMemberNames();

	Header.ShaderRef_ = ImportShader.asUInt();	// TODO: Go via addImport. This can then verify for us.
	Header.NoofTextures_ = (BcU32)TextureMembers.size();	
	HeaderStream << Header;

	// Make texture headers.
	for( BcU32 Idx = 0; Idx < TextureMembers.size(); ++Idx )
	{
		const Json::Value& Texture = ImportTextures[ TextureMembers[ Idx ] ];

		TextureHeader.SamplerName_ = Importer.addString( BcName::StripInvalidChars( TextureMembers[ Idx ].c_str() ).c_str() );
		TextureHeader.TextureRef_ = Texture.asUInt(); // TODO: Go via addImport. This can then verify for us.
		HeaderStream << TextureHeader;
	}
	
	// Make state block stream.
	const BcChar* StateNames[] = 
	{
		"depth_write_enable",
		"depth_test_enable",
		"depth_test_compare",
		"stencil_write_mask",
		"stencil_test_enable",
		"stencil_test_func_compare",
		"stencil_test_func_ref",
		"stencil_test_func_mask",
		"stencil_test_op_sfail",
		"stencil_test_op_dpfail",
		"stencil_test_op_dppass",
		"color_write_mask_0",
		"color_write_mask_1",
		"color_write_mask_2",
		"color_write_mask_3",
		"blend_mode",
		"fill_mode",
	};
	
	// This code is bad code. Use the reflection system.
	std::map< std::string, BcU32 > ModeNames;
		
	ModeNames[ "never" ] = (BcU32)RsCompareMode::NEVER;
	ModeNames[ "less" ] = (BcU32)RsCompareMode::LESS;
	ModeNames[ "equal" ] = (BcU32)RsCompareMode::EQUAL;
	ModeNames[ "lessequal" ] = (BcU32)RsCompareMode::LESSEQUAL;
	ModeNames[ "greater" ] = (BcU32)RsCompareMode::GREATER;
	ModeNames[ "notequal" ] = (BcU32)RsCompareMode::NOTEQUAL;
	ModeNames[ "always" ] = (BcU32)RsCompareMode::ALWAYS;
		
	ModeNames[ "none" ] = (BcU32)RsBlendingMode::NONE;
	ModeNames[ "blend" ] = (BcU32)RsBlendingMode::BLEND;
	ModeNames[ "add" ] = (BcU32)RsBlendingMode::ADD;
	ModeNames[ "subtract" ] = (BcU32)RsBlendingMode::SUBTRACT;

	ModeNames[ "keep" ] = (BcU32)RsStencilOp::KEEP;
	ModeNames[ "zero" ] = (BcU32)RsStencilOp::ZERO;
	ModeNames[ "replace" ] = (BcU32)RsStencilOp::REPLACE;
	ModeNames[ "incr" ] = (BcU32)RsStencilOp::INCR;
	ModeNames[ "incr_wrap" ] = (BcU32)RsStencilOp::INCR_WRAP;
	ModeNames[ "decr" ] = (BcU32)RsStencilOp::DECR;
	ModeNames[ "decr_wrap" ] = (BcU32)RsStencilOp::DECR_WRAP;
	ModeNames[ "invert" ] = (BcU32)RsStencilOp::INVERT;

	ModeNames[ "solid" ] = (BcU32)RsFillMode::SOLID;
	ModeNames[ "wireframe" ] = (BcU32)RsFillMode::WIREFRAME;

	for( BcU32 Idx = 0; Idx < (BcU32)RsRenderStateType::MAX; ++Idx )
	{
		bool WriteDefaultState = BcFalse;
		if( State.type() == Json::objectValue )
		{
			const Json::Value& StateValue = State[ StateNames[ Idx ] ];
			
			if( StateValue.type() == Json::realValue )
			{
				BcF32 RealValue = (BcF32)StateValue.asDouble();
				StateBlockStream << BcU32( RealValue );
			}
			else if( StateValue.type() == Json::stringValue )
			{
				BcU32 IntValue = ModeNames[ StateValue.asCString() ];
				StateBlockStream << BcU32( IntValue );
			}
			else if( StateValue.type() == Json::intValue ||
					 StateValue.type() == Json::uintValue )
			{
				BcU32 IntValue = (BcU32)StateValue.asInt();
				StateBlockStream << BcU32( IntValue );
			}
			else
			{
				BcPrintf( "ScnMaterial::import: Unable to parse value for \"%s\"\n", StateNames[ Idx ] );
				WriteDefaultState = BcTrue;
			}
		}
		else
		{
			WriteDefaultState = BcTrue;
		}

		if( WriteDefaultState )
		{
			// Horrible default special case. Should have a table.
			switch( Idx )
			{
			case RsRenderStateType::COLOR_WRITE_MASK_0:
				StateBlockStream << BcU32( 0xf );
				break;

			default:
				StateBlockStream << BcU32( 0 );
				break;
			}
				
		}
	}
	
	// Add chunks.
	Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
	Importer.addChunk( BcHash( "stateblock" ), StateBlockStream.pData(), StateBlockStream.dataSize() );
		
	return BcTrue;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterial );

void ScnMaterial::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Shader_",				&ScnMaterial::Shader_ ),
		ReField( "TextureMap_",			&ScnMaterial::TextureMap_ ),
	};
		
	ReRegisterClass< ScnMaterial, Super >( Fields );
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

void ScnMaterialComponent::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Parent_",						&ScnMaterialComponent::Parent_ ),
		ReField( "pProgram_",					&ScnMaterialComponent::pProgram_ ),
		ReField( "ViewUniformBlockIndex_",		&ScnMaterialComponent::ViewUniformBlockIndex_ ),
		ReField( "BoneUniformBlockIndex_",		&ScnMaterialComponent::BoneUniformBlockIndex_ ),
		ReField( "ObjectUniformBlockIndex_",	&ScnMaterialComponent::ObjectUniformBlockIndex_ ),
	};
		
	ReRegisterClass< ScnMaterialComponent, Super >( Fields );
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
	pStateBuffer_ = new BcU32[ (BcU32)RsRenderStateType::MAX ];
	BcMemCopy( pStateBuffer_, Parent->pStateBuffer_, sizeof( BcU32 ) * (BcU32)RsRenderStateType::MAX );

	// Build a binding list for textures.
	ScnTextureMap& TextureMap( Parent->TextureMap_ );
	for( ScnTextureMapConstIterator Iter( TextureMap.begin() ); Iter != TextureMap.end(); ++Iter )
	{
		const BcName& SamplerName = (*Iter).first;
		ScnTextureRef Texture = (*Iter).second;

		BcU32 SamplerIdx = findSampler( SamplerName );
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
	delete pStateBuffer_;
	pStateBuffer_ = NULL;
	
	Parent_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// findSampler
BcU32 ScnMaterialComponent::findSampler( const BcName& SamplerName )
{
	// TODO: Improve this, also store parameter info in parent material to
	//       save memory and move look ups to it's own creation.
	BcU32 Handle = pProgram_->findSampler( (*SamplerName).c_str() );
	
	if( Handle != BcErrorCode )
	{
		for( BcU32 Idx = 0; Idx < SamplerBindingList_.size(); ++Idx )
		{
			TSamplerBinding& Binding = SamplerBindingList_[ Idx ];
			
			if( Binding.Handle_ == Handle )
			{
				return Idx;
			}
		}
		
		// If it doesn't exist, add it.
		TSamplerBinding Binding = 
		{
			Handle, nullptr
		};
		
		SamplerBindingList_.push_back( Binding );
		return (BcU32)SamplerBindingList_.size() - 1;
	}
	
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setTexture( BcU32 Idx, ScnTextureRef Texture )
{
	// Find the texture slot to put this in.
	if( Idx < SamplerBindingList_.size() )
	{
		TSamplerBinding& TexBinding( SamplerBindingList_[ Idx ] );
		TexBinding.Texture_ = Texture;
	}
	else
	{
		BcPrintf( "ERROR: Unable to set texture for index %x\n", Idx );
	}
}

//////////////////////////////////////////////////////////////////////////
// findUniformBlock
BcU32 ScnMaterialComponent::findUniformBlock( const BcName& UniformBlockName )
{
	BcU32 Index = pProgram_->findUniformBlockIndex( (*UniformBlockName).c_str() );
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
		TUniformBlockBinding Binding = 
		{
			Index, nullptr
		};
		
		UniformBlockBindingList_.push_back( Binding );
		return (BcU32)UniformBlockBindingList_.size() - 1;
	}
	
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setUniformBlock
void ScnMaterialComponent::setUniformBlock( BcU32 Index, RsBuffer* UniformBuffer )
{
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
		pStateBuffer_[ (BcU32)State ] = Value;
	}
}

//////////////////////////////////////////////////////////////////////////
// getTexture
ScnTextureRef ScnMaterialComponent::getTexture( BcU32 Idx )
{
	if( Idx < SamplerBindingList_.size() )
	{
		return SamplerBindingList_[ Idx ].Texture_;
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
			pProgram_->setSampler( SamplerHandles_[ Idx ], Idx );
			pContext_->setTextureState( Idx, pTexture, TextureParams );
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
			pProgram_->setUniformBlock( Index, pUniformBuffer );
		}

		// Set program.
		pContext_->setProgram( pProgram_ );

		// Done.
		pUpdateFence_->decrement();
	}
	
	RsProgram* pProgram_;

	// Texture binding block.
	BcU32 NoofTextures_;
	BcU32* SamplerHandles_;
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
	Sort.Blend_ = pStateBuffer_[ (BcU32)RsRenderStateType::BLEND_MODE ];
	
	// Allocate a render node.
	ScnMaterialComponentRenderNode* pRenderNode = pFrame->newObject< ScnMaterialComponentRenderNode >();
	
	// Debugging.
	pRenderNode->pParent_ = this;
	
	// Setup program and state.
	pRenderNode->pProgram_ = pProgram_;
	
	// Setup texture binding block.
	pRenderNode->NoofTextures_ = (BcU32)SamplerBindingList_.size();
	pRenderNode->SamplerHandles_ = (BcU32*)pFrame->allocMem( sizeof( BcU32 ) * pRenderNode->NoofTextures_ );
	pRenderNode->ppTextures_ = (RsTexture**)pFrame->allocMem( sizeof( RsTexture* ) * pRenderNode->NoofTextures_ );
	pRenderNode->pTextureParams_ = (RsTextureParams*)pFrame->allocMem( sizeof( RsTextureParams ) * pRenderNode->NoofTextures_ );
	
	for( BcU32 Idx = 0; Idx < pRenderNode->NoofTextures_; ++Idx )
	{
		TSamplerBinding& Binding = SamplerBindingList_[ Idx ];
		RsTexture*& Texture = pRenderNode->ppTextures_[ Idx ];
		RsTextureParams& TextureParams = pRenderNode->pTextureParams_[ Idx ];
		
		// Sampler handles.
		pRenderNode->SamplerHandles_[ Idx ] = Binding.Handle_;

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
	BcMemCopy( pRenderNode->pStateBuffer_, pStateBuffer_, sizeof( BcU32 ) * (BcU32)RsRenderStateType::MAX );
	
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
