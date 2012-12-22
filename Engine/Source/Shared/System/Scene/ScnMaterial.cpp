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
	Header.NoofTextures_ = TextureMembers.size();	
	HeaderStream << Header;

	// Make texture headers.
	for( BcU32 Idx = 0; Idx < TextureMembers.size(); ++Idx )
	{
		const Json::Value& Texture = ImportTextures[ TextureMembers[ Idx ] ];

		TextureHeader.SamplerName_ = Importer.addString( TextureMembers[ Idx ].c_str() );
		TextureHeader.TextureRef_ = Texture.asUInt(); // TODO: Go via addImport. This can then verify for us.
		HeaderStream << TextureHeader;
	}
	
	// Make state block stream.
	const BcChar* StateNames[] = 
	{
		"depth_write_enable",
		"depth_test_enable",
		"depth_test_compare",
		"depth_bias",
		"alpha_test_enable",
		"alpha_test_compare",
		"alpha_test_threshold",
		"stencil_write_mask",
		"stencil_test_enable",
		"stencil_test_func_compare",
		"stencil_test_func_ref",
		"stencil_test_func_mask",
		"stencil_test_op_sfail",
		"stencil_test_op_dpfail",
		"stencil_test_op_dppass",
		"color_write_red_enable",
		"color_write_green_enable",
		"color_write_blue_enable",
		"color_write_alpha_enable",
		"blend_mode"
	};
		
	std::map< std::string, BcU32 > ModeNames;
		
	ModeNames[ "never" ] = rsCM_NEVER;
	ModeNames[ "less" ] = rsCM_LESS;
	ModeNames[ "equal" ] = rsCM_EQUAL;
	ModeNames[ "lessequal" ] = rsCM_LESSEQUAL;
	ModeNames[ "greater" ] = rsCM_GREATER;
	ModeNames[ "notequal" ] = rsCM_NOTEQUAL;
	ModeNames[ "always" ] = rsCM_ALWAYS;
		
	ModeNames[ "none" ] = rsBM_NONE;
	ModeNames[ "blend" ] = rsBM_BLEND;
	ModeNames[ "add" ] = rsBM_ADD;
	ModeNames[ "subtract" ] = rsBM_SUBTRACT;

	ModeNames[ "keep" ] = rsSO_KEEP;
	ModeNames[ "zero" ] = rsSO_ZERO;
	ModeNames[ "replace" ] = rsSO_REPLACE;
	ModeNames[ "incr" ] = rsSO_INCR;
	ModeNames[ "incr_wrap" ] = rsSO_INCR_WRAP;
	ModeNames[ "decr" ] = rsSO_DECR;
	ModeNames[ "decr_wrap" ] = rsSO_DECR_WRAP;
	ModeNames[ "invert" ] = rsSO_INVERT;

	for( BcU32 Idx = 0; Idx < rsRS_MAX; ++Idx )
	{
		if( State.type() == Json::objectValue )
		{
			const Json::Value& StateValue = State[ StateNames[ Idx ] ];
			
			if( StateValue.type() == Json::realValue )
			{
				BcReal RealValue = (BcReal)StateValue.asDouble();
				StateBlockStream << BcU32( RealValue );
			}
			else if( StateValue.type() == Json::stringValue )
			{
				BcU32 IntValue = ModeNames[ StateValue.asCString() ];
				if( BcStrCompare( StateValue.asCString(), "subtract" ) )
				{
					int a = 0; ++a;
				}
				StateBlockStream << BcU32( IntValue );
			}
			else
			{
				BcU32 IntValue = (BcU32)StateValue.asInt();
				StateBlockStream << BcU32( IntValue );
			}
		}
		else
		{
			// Horrible default special case. Should have a table.
			switch( Idx )
			{
			case rsRS_COLOR_WRITE_RED_ENABLE:
			case rsRS_COLOR_WRITE_GREEN_ENABLE:
			case rsRS_COLOR_WRITE_BLUE_ENABLE:
			case rsRS_COLOR_WRITE_ALPHA_ENABLE:
				StateBlockStream << BcU32( 1 );
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
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnMaterial::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnMaterial::isReady()
{
	for( ScnTextureMapIterator Iter( TextureMap_.begin() ); Iter != TextureMap_.end(); ++Iter )
	{
		if( (*Iter).second->isReady() == BcFalse )
		{
			return BcFalse;
		}
	}
	
	return Shader_.isReady() && pStateBuffer_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// getTexture
ScnTextureRef ScnMaterial::getTexture( BcName Name )
{
	return TextureMap_[ *Name ];
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
		
		// Get resources.
		Shader_ = getPackage()->getPackageCrossRef( pHeader_->ShaderRef_ );
		for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
		{
			ScnMaterialTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
			TextureMap_[ getString( pTextureHeader->SamplerName_ ) ] = getPackage()->getPackageCrossRef( pTextureHeader->TextureRef_ );
		}
		
		requestChunk( ++ChunkIdx );
	}
	else if( ChunkID == BcHash( "stateblock" ) )
	{
		pStateBuffer_ = (BcU32*)pData;
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterialComponent );

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnMaterialComponent::initialise( ScnMaterialRef Parent, BcU32 PermutationFlags )
{
	BcAssert( Parent.isReady() );

	// Cache parent and program.
	Parent_ = Parent;
	pProgram_ = Parent->Shader_->getProgram( PermutationFlags );
	
	// Allocate parameter buffer.
	ParameterBufferSize_ = pProgram_->getParameterBufferSize();
	if( ParameterBufferSize_ > 0 )
	{
		pParameterBuffer_ = new BcU8[ ParameterBufferSize_ ];
		BcMemSet( pParameterBuffer_, 0, ParameterBufferSize_ );
	}
	else
	{
		pParameterBuffer_ = NULL;
	}
	
	// Allocate state buffer and copy defaults in.
	pStateBuffer_ = new BcU32[ rsRS_MAX ];
	BcMemCopy( pStateBuffer_, Parent->pStateBuffer_, sizeof( BcU32 ) * rsRS_MAX );

	// Build a binding list for textures.
	ScnTextureMap& TextureMap( Parent->TextureMap_ );
	for( ScnTextureMapConstIterator Iter( TextureMap.begin() ); Iter != TextureMap.end(); ++Iter )
	{
		const std::string& SamplerName = (*Iter).first;
		ScnTextureRef Texture = (*Iter).second;
		
		// Find sampler in program.
		TTextureBinding Binding = 
		{
			findParameter( SamplerName ),
			Texture
		};
		
		// Only add if parameter is valid.
		if( Binding.Parameter_ != BcErrorCode )
		{
			TextureBindingList_.push_back( Binding );
		}
	}

	// Grab common parameters.
	ClipTransformParameter_ = findParameter( "uClipTransform" );
	ViewTransformParameter_ = findParameter( "uViewTransform" );
	InverseViewTransformParameter_ = findParameter( "uInverseViewTransform" );
	WorldTransformParameter_ = findParameter( "uWorldTransform" );
	EyePositionParameter_ = findParameter( "uEyePosition" );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnMaterialComponent::initialise( const Json::Value& Object )
{
	ScnMaterialRef MaterialRef;
	MaterialRef = getPackage()->getPackageCrossRef( Object[ "material" ].asUInt() );
	BcU32 PermutationFlags = 0;
	const BcChar* pPermutation = Object[ "permutation" ].asCString();

	if( BcStrCompare( pPermutation, "2d" ) )
	{
		PermutationFlags = scnSPF_2D;
	}
	else if( BcStrCompare( pPermutation, "3d" ) )
	{
		PermutationFlags = scnSPF_3D;
	}

	initialise( MaterialRef, PermutationFlags );
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ScnMaterialComponent::destroy()
{
	delete pStateBuffer_;
	pStateBuffer_ = NULL;
	
	delete pParameterBuffer_;
	pParameterBuffer_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
BcU32 ScnMaterialComponent::findParameter( const BcName& ParameterName )
{
	// TODO: Improve this, also store parameter info in parent material to
	//       save memory and move look ups to it's own creation.
	BcU32 Offset = BcErrorCode;
	eRsShaderParameterType Type;
	if( pProgram_->findParameterOffset( (*ParameterName).c_str(), Type, Offset ) )
	{
		for( BcU32 Idx = 0; Idx < ParameterBindingList_.size(); ++Idx )
		{
			TParameterBinding& Binding = ParameterBindingList_[ Idx ];
			
			if( Binding.Type_ == Type && Binding.Offset_ == Offset )
			{
				return Idx;
			}
		}
		
		// If it doesn't exist, add it.
		TParameterBinding Binding = 
		{
			Type, Offset
		};
		
		ParameterBindingList_.push_back( Binding );
		return ParameterBindingList_.size() - 1;
	}
	
	//BcPrintf( "ScnMaterialComponent (%s): Can't find parameter \"%s\"\n", (*getName()).c_str(), (*ParameterName).c_str() );
	
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, BcS32 Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_INT ||
		    Binding.Type_ == rsSPT_SAMPLER_1D ||
			Binding.Type_ == rsSPT_SAMPLER_2D ||
			Binding.Type_ == rsSPT_SAMPLER_3D ||
			Binding.Type_ == rsSPT_SAMPLER_CUBE ||
			Binding.Type_ == rsSPT_SAMPLER_1D_SHADOW ||
			Binding.Type_ == rsSPT_SAMPLER_2D_SHADOW )	   
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcS32* pParameterBuffer = ((BcS32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer = Value;
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not an int.\n", (*getName()).c_str(), Parameter );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, BcBool Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_BOOL )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcS32* pParameterBuffer = ((BcS32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer = (BcU32)Value;
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a bool.\n", (*getName()).c_str(), Parameter );
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, BcReal Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer = (BcF32)Value;
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a real.\n", (*getName()).c_str(), Parameter );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, const BcVec2d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC2 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer++ = (BcF32)Value.x();
			*pParameterBuffer = (BcF32)Value.y();
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a vec2.\n", (*getName()).c_str(), Parameter );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, const BcVec3d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC3 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer++ = (BcF32)Value.x();
			*pParameterBuffer++ = (BcF32)Value.y();
			*pParameterBuffer = (BcF32)Value.z();
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a vec3.\n", (*getName()).c_str(), Parameter );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, const BcVec4d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC4 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer++ = (BcF32)Value.x();
			*pParameterBuffer++ = (BcF32)Value.y();
			*pParameterBuffer++ = (BcF32)Value.z();
			*pParameterBuffer = (BcF32)Value.w();
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a vec4.\n", (*getName()).c_str(), Parameter );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, const BcMat3d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_MAT3 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer++ = (BcF32)Value[0][0];
			*pParameterBuffer++ = (BcF32)Value[0][1];
			*pParameterBuffer++ = (BcF32)Value[0][2];
			*pParameterBuffer++ = (BcF32)Value[1][0];
			*pParameterBuffer++ = (BcF32)Value[1][1];
			*pParameterBuffer++ = (BcF32)Value[1][2];
			*pParameterBuffer++ = (BcF32)Value[2][0];
			*pParameterBuffer++ = (BcF32)Value[2][1];
			*pParameterBuffer = (BcF32)Value[2][2];
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a mat3.\n", (*getName()).c_str(), Parameter );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, const BcMat4d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_MAT4 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer++ = (BcF32)Value[0][0];
			*pParameterBuffer++ = (BcF32)Value[0][1];
			*pParameterBuffer++ = (BcF32)Value[0][2];
			*pParameterBuffer++ = (BcF32)Value[0][3];
			*pParameterBuffer++ = (BcF32)Value[1][0];
			*pParameterBuffer++ = (BcF32)Value[1][1];
			*pParameterBuffer++ = (BcF32)Value[1][2];
			*pParameterBuffer++ = (BcF32)Value[1][3];
			*pParameterBuffer++ = (BcF32)Value[2][0];
			*pParameterBuffer++ = (BcF32)Value[2][1];
			*pParameterBuffer++ = (BcF32)Value[2][2];
			*pParameterBuffer++ = (BcF32)Value[2][3];
			*pParameterBuffer++ = (BcF32)Value[3][0];
			*pParameterBuffer++ = (BcF32)Value[3][1];
			*pParameterBuffer++ = (BcF32)Value[3][2];
			*pParameterBuffer = (BcF32)Value[3][3];
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a mat4.\n", (*getName()).c_str(), Parameter );
		}
	}
	
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setTexture( BcU32 Parameter, ScnTextureRef Texture )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_SAMPLER_1D ||
		    Binding.Type_ == rsSPT_SAMPLER_2D ||
		    Binding.Type_ == rsSPT_SAMPLER_3D ||
		    Binding.Type_ == rsSPT_SAMPLER_CUBE ||
		    Binding.Type_ == rsSPT_SAMPLER_1D_SHADOW ||
		    Binding.Type_ == rsSPT_SAMPLER_2D_SHADOW )	   
		{
			// Find the texture slot to put this in.
			for( TTextureBindingListIterator Iter( TextureBindingList_.begin() ); Iter != TextureBindingList_.end(); ++Iter )
			{
				TTextureBinding& TexBinding( *Iter );
				
				if( TexBinding.Parameter_ == Parameter )
				{
					TexBinding.Texture_ = Texture;
					break;
				}
			}
		}
		else
		{
			BcPrintf( "ScnMaterialComponent: \"%s\"'s Parameter %u is not a texture.\n", (*getName()).c_str(), Parameter );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setClipTransform
void ScnMaterialComponent::setClipTransform( const BcMat4d& Transform )
{
	setParameter( ClipTransformParameter_, Transform );
}

//////////////////////////////////////////////////////////////////////////
// setViewTransform
void ScnMaterialComponent::setViewTransform( const BcMat4d& Transform )
{
	setParameter( ViewTransformParameter_, Transform );

	if( InverseViewTransformParameter_ != BcErrorCode )
	{
		BcMat4d InverseViewTransform = Transform;
		InverseViewTransform.inverse();
		setParameter( InverseViewTransformParameter_, InverseViewTransform );
	}
}

//////////////////////////////////////////////////////////////////////////
// setEyePosition
void ScnMaterialComponent::setEyePosition( const BcVec3d& EyePosition )
{
	setParameter( EyePositionParameter_, EyePosition );
}

//////////////////////////////////////////////////////////////////////////
// setWorldTransform
void ScnMaterialComponent::setWorldTransform( const BcMat4d& Transform )
{
	setParameter( WorldTransformParameter_, Transform );
}

//////////////////////////////////////////////////////////////////////////
// setState
void ScnMaterialComponent::setState( eRsRenderState State, BcU32 Value )
{
	if( State < rsRS_MAX )
	{
		pStateBuffer_[ State ] = Value;
	}
}

//////////////////////////////////////////////////////////////////////////
// getTexture
ScnTextureRef ScnMaterialComponent::getTexture( BcU32 Parameter )
{

	for( TTextureBindingListIterator Iter( TextureBindingList_.begin() ); Iter != TextureBindingList_.end(); ++Iter )
	{
		TTextureBinding& TexBinding( *Iter );
		
		if( TexBinding.Parameter_ == Parameter )
		{
			return TexBinding.Texture_;
		}
	}
	
	return NULL;
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
		// TODO: Some kind of object to batch this crap up properly.
		RsStateBlock* pStateBlock = RsCore::pImpl()->getStateBlock();

		// Iterate over textures and bind.
		for( BcU32 Idx = 0; Idx < NoofTextures_; ++Idx )
		{
			RsTexture* pTexture = ppTextures_[ Idx ];
			RsTextureParams& TextureParams = pTextureParams_[ Idx ];
			pStateBlock->setTextureState( Idx, pTexture, TextureParams );
		}

		// Setup states.
		for( BcU32 Idx = 0; Idx < rsRS_MAX; ++Idx )
		{
			pStateBlock->setRenderState( (eRsRenderState)Idx, pStateBuffer_[ Idx ], BcFalse );
		}

		// Bind state block.
		pStateBlock->bind();
		
		// Bind program.
		pProgram_->bind( pParameterBuffer_ );
	}
	
	RsProgram* pProgram_;

	// Texture binding block.
	BcU32 NoofTextures_;
	RsTexture** ppTextures_;
	RsTextureParams* pTextureParams_;

	// Parameter buffer.
	BcU8* pParameterBuffer_;
	
	// State buffer.
	BcU32* pStateBuffer_;
	
	// For debugging.
	ScnMaterialComponent* pParent_;
};

void ScnMaterialComponent::bind( RsFrame* pFrame, RsRenderSort& Sort )
{
	// Setup sort value with material specifics.
	ScnMaterial* pMaterial_ = Parent_;
	//Sort.MaterialID_ = BcU64( ( BcU32( pMaterial_ ) & 0xffff ) ^ ( BcU32( pMaterial_ ) >> 4 ) & 0xffff );			// revisit once canvas is fixed!
	Sort.Blend_ = pStateBuffer_[ rsRS_BLEND_MODE ];

	// Default texture parameters.
	RsTextureParams DefaultTextureParams = 
	{
		rsTFM_LINEAR, rsTFM_LINEAR, rsTSM_WRAP, rsTSM_WRAP
	};
	
	// Allocate a render node.
	ScnMaterialComponentRenderNode* pRenderNode = pFrame->newObject< ScnMaterialComponentRenderNode >();
	
	// Debugging.
	pRenderNode->pParent_ = this;
	
	// Setup program and state.
	pRenderNode->pProgram_ = pProgram_;
	
	// Setup texture binding block.
	pRenderNode->NoofTextures_ = TextureBindingList_.size();
	pRenderNode->ppTextures_ = (RsTexture**)pFrame->allocMem( sizeof( RsTexture* ) * pRenderNode->NoofTextures_ );
	pRenderNode->pTextureParams_ = (RsTextureParams*)pFrame->allocMem( sizeof( RsTextureParams ) * pRenderNode->NoofTextures_ );
	
	for( BcU32 Idx = 0; Idx < pRenderNode->NoofTextures_; ++Idx )
	{
		TTextureBinding& Binding = TextureBindingList_[ Idx ];
		RsTexture*& Texture = pRenderNode->ppTextures_[ Idx ];
		RsTextureParams& TextureParams = pRenderNode->pTextureParams_[ Idx ];
	
		// Set sampler parameter.	BcU8		B_;
		
		setParameter( Binding.Parameter_, (BcS32)Idx );
		
		// Set texture to bind.
		Texture = Binding.Texture_->getTexture();
		
		// Set texture params.
		// TODO: Pull these from material instance.
		TextureParams = DefaultTextureParams;
	}
	
	// Setup parameter buffer.
	pRenderNode->pParameterBuffer_ = (BcU8*)pFrame->allocMem( ParameterBufferSize_ );
	BcMemCopy( pRenderNode->pParameterBuffer_, pParameterBuffer_, ParameterBufferSize_ );

	// Setup state buffer.
	pRenderNode->pStateBuffer_ = (BcU32*)pFrame->allocMem( sizeof( BcU32 ) * rsRS_MAX );
	BcMemCopy( pRenderNode->pStateBuffer_, pStateBuffer_, sizeof( BcU32 ) * rsRS_MAX );
	
	// Add node to frame.
	pRenderNode->Sort_ = Sort;
	pFrame->addRenderNode( pRenderNode );
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnMaterialComponent::isReady()
{
	for( BcU32 Idx = 0; Idx < TextureBindingList_.size(); ++Idx )
	{
		if( TextureBindingList_[ Idx ].Texture_->isReady() == BcFalse )
		{
			return BcFalse;
		}
	}
	
	return Parent_.isReady() && pProgram_->getHandle< BcU64 >() != 0;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void ScnMaterialComponent::update( BcReal Tick )
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
	ScnComponent::onDetach( Parent );
}
