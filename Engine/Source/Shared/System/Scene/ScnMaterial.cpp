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
			case rsRS_COLOR_WRITE_MASK_0:
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
		ReField( "ParameterBufferSize_",		&ScnMaterialComponent::ParameterBufferSize_ ),
		ReField( "WorldTransformParameter_",	&ScnMaterialComponent::WorldTransformParameter_ ),
		ReField( "LightPositionParameter_",		&ScnMaterialComponent::LightPositionParameter_ ),
		ReField( "LightDirectionParameter_",	&ScnMaterialComponent::LightDirectionParameter_ ),
		ReField( "LightAmbientColourParameter_",&ScnMaterialComponent::LightAmbientColourParameter_ ),
		ReField( "LightDiffuseColourParameter_",&ScnMaterialComponent::LightDiffuseColourParameter_ ),
		ReField( "LightAttnParameter_",			&ScnMaterialComponent::LightAttnParameter_ ),
		ReField( "ViewUniformBlockIndex_",		&ScnMaterialComponent::ViewUniformBlockIndex_ ),
		ReField( "BoneUniformBlockIndex_",		&ScnMaterialComponent::BoneUniformBlockIndex_ ),
	};
		
	ReRegisterClass< ScnMaterialComponent, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnMaterialComponent::initialise( ScnMaterialRef Parent, BcU32 PermutationFlags )
{
	Super::initialise();

	BcAssert( Parent.isValid() && Parent->isReady() );
	 
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
		const BcName& SamplerName = (*Iter).first;
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
	WorldTransformParameter_ = findParameter( "uWorldTransform" );
	LightPositionParameter_ = findParameter( "uLightPosition" );
	LightDirectionParameter_ = findParameter( "uLightDirection" );
	LightAmbientColourParameter_ = findParameter( "uLightAmbientColour" );
	LightDiffuseColourParameter_ = findParameter( "uLightDiffuseColour" );
	LightAttnParameter_ = findParameter( "uLightAttn" );

	// Grab uniform blocks.
	ViewUniformBlockIndex_ = findUniformBlock( "ViewUniformBlock" );
	BoneUniformBlockIndex_ = findUniformBlock( "BoneUniformBlock" );
}

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnMaterialComponent::initialise( const Json::Value& Object )
{
	ScnMaterialRef MaterialRef = getPackage()->getPackageCrossRef( Object[ "material" ].asUInt() );
	BcU32 PermutationFlags = 0;
	const BcChar* pPermutation = Object[ "permutation" ].asCString();

	if( BcStrCompare( pPermutation, "2d" ) )
	{
		PermutationFlags = scnSPF_MESH_STATIC_2D;
	}
	else if( BcStrCompare( pPermutation, "3d" ) )
	{
		PermutationFlags = scnSPF_MESH_STATIC_3D;
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

	Parent_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// findParameter
BcU32 ScnMaterialComponent::findParameter( const BcName& ParameterName )
{
	// TODO: Improve this, also store parameter info in parent material to
	//       save memory and move look ups to it's own creation.
	BcU32 Offset = BcErrorCode;
	eRsShaderParameterType Type;
	BcU32 TypeBytes = 0;
	if( pProgram_->findParameterOffset( (*ParameterName).c_str(), Type, Offset, TypeBytes ) )
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
			Type, Offset, TypeBytes
		};
		
		ParameterBindingList_.push_back( Binding );
		return (BcU32)ParameterBindingList_.size() - 1;
	}
	
	//BcPrintf( "ScnMaterialComponent (%s): Can't find parameter \"%s\"\n", (*getName()).c_str(), (*ParameterName).c_str() );
	
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialComponent::setParameter( BcU32 Parameter, BcS32 Value, BcU32 Index )
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
			BcS32* pParameterBuffer = ((BcS32*)pParameterBuffer_) + Binding.Offset_ + ( Index * Binding.TypeBytes_ >> 2 );			
			BcAssert( (void*)pParameterBuffer < (void*)(pParameterBuffer_ + ParameterBufferSize_) );
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
void ScnMaterialComponent::setParameter( BcU32 Parameter, BcBool Value, BcU32 Index )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_BOOL )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcS32* pParameterBuffer = ((BcS32*)pParameterBuffer_) + Binding.Offset_ + ( Index * Binding.TypeBytes_ >> 2 );			
			BcAssert( (void*)pParameterBuffer < (void*)(pParameterBuffer_ + ParameterBufferSize_) );
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
void ScnMaterialComponent::setParameter( BcU32 Parameter, BcF32 Value, BcU32 Index )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_ + ( Index * Binding.TypeBytes_ >> 2 );			
			BcAssert( (void*)pParameterBuffer < (void*)(pParameterBuffer_ + ParameterBufferSize_) );
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
void ScnMaterialComponent::setParameter( BcU32 Parameter, const MaVec2d& Value, BcU32 Index )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC2 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_ + ( Index * Binding.TypeBytes_ >> 2 );			
			BcAssert( (void*)pParameterBuffer < (void*)(pParameterBuffer_ + ParameterBufferSize_) );
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
void ScnMaterialComponent::setParameter( BcU32 Parameter, const MaVec3d& Value, BcU32 Index )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC3 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_ + ( Index * Binding.TypeBytes_ >> 2 );			
			BcAssert( (void*)pParameterBuffer < (void*)(pParameterBuffer_ + ParameterBufferSize_) );
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
void ScnMaterialComponent::setParameter( BcU32 Parameter, const MaVec4d& Value, BcU32 Index )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC4 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_ + ( Index * Binding.TypeBytes_ >> 2 );			
			BcAssert( (void*)pParameterBuffer < (void*)(pParameterBuffer_ + ParameterBufferSize_) );
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
void ScnMaterialComponent::setParameter( BcU32 Parameter, const MaMat4d& Value, BcU32 Index )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_MAT4 )
		{
			BcAssert( Binding.Offset_ <  ( ParameterBufferSize_ >> 2 ) );
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_ + ( Index * Binding.TypeBytes_ >> 2 );
			BcAssert( (void*)pParameterBuffer < (void*)(pParameterBuffer_ + ParameterBufferSize_) );
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
void ScnMaterialComponent::setUniformBlock( BcU32 Index, RsUniformBuffer* UniformBuffer )
{
	auto& UniformBlockBinding = UniformBlockBindingList_[ Index ];
	UniformBlockBinding.UniformBuffer_ = UniformBuffer;
}

//////////////////////////////////////////////////////////////////////////
// setWorldTransform
void ScnMaterialComponent::setWorldTransform( const MaMat4d& Transform )
{
	setParameter( WorldTransformParameter_, Transform );
}

//////////////////////////////////////////////////////////////////////////
// setLightParameters
void ScnMaterialComponent::setLightParameters( BcU32 LightIndex, const MaVec3d& Position, const MaVec3d& Direction, const RsColour& AmbientColour, const RsColour& DiffuseColour, BcF32 AttnC, BcF32 AttnL, BcF32 AttnQ )
{
	// TODO: Perhaps store light values in a matrix to save on setting parameters?
	setParameter( LightPositionParameter_, Position, LightIndex );
	setParameter( LightDirectionParameter_, Direction, LightIndex );
	setParameter( LightAmbientColourParameter_, AmbientColour, LightIndex );
	setParameter( LightDiffuseColourParameter_, DiffuseColour, LightIndex );
	setParameter( LightAttnParameter_, MaVec3d( AttnC, AttnL, AttnQ ), LightIndex );
}

//////////////////////////////////////////////////////////////////////////
// setViewUniformBlock
void ScnMaterialComponent::setViewUniformBlock( RsUniformBuffer* UniformBuffer )
{
	setUniformBlock( ViewUniformBlockIndex_, UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// setBoneUniformBlock
void ScnMaterialComponent::setBoneUniformBlock( RsUniformBuffer* UniformBuffer )
{
	setUniformBlock( BoneUniformBlockIndex_, UniformBuffer );
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
		// Iterate over textures and bind.
		for( BcU32 Idx = 0; Idx < NoofTextures_; ++Idx )
		{
			RsTexture* pTexture = ppTextures_[ Idx ];
			RsTextureParams& TextureParams = pTextureParams_[ Idx ];
			pContext_->setTextureState( Idx, pTexture, TextureParams );
		}

		// Setup states.
		for( BcU32 Idx = 0; Idx < rsRS_MAX; ++Idx )
		{
			pContext_->setRenderState( (eRsRenderState)Idx, pStateBuffer_[ Idx ], BcFalse );
		}
	
		// Set uniform blocks.
		for( BcU32 Idx = 0; Idx < NoofUniformBlocks_; ++Idx )
		{
			BcU32 Index = pUniformBlockIndices_[ Idx ];
			RsUniformBuffer* pUniformBuffer = ppUniformBuffers_[ Idx ];
			pProgram_->setUniformBlock( Index, pUniformBuffer );
		}

		// Bind program.
		pProgram_->bind( pParameterBuffer_ );

		// Flush state in context.
		pContext_->flushState();

		// Done.
		pUpdateFence_->decrement();
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

	// Uniform blocks.
	BcU32 NoofUniformBlocks_;
	BcU32* pUniformBlockIndices_;
	RsUniformBuffer** ppUniformBuffers_;
	
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
	Sort.Blend_ = pStateBuffer_[ rsRS_BLEND_MODE ];
	
	// Allocate a render node.
	ScnMaterialComponentRenderNode* pRenderNode = pFrame->newObject< ScnMaterialComponentRenderNode >();
	
	// Debugging.
	pRenderNode->pParent_ = this;
	
	// Setup program and state.
	pRenderNode->pProgram_ = pProgram_;
	
	// Setup texture binding block.
	pRenderNode->NoofTextures_ = (BcU32)TextureBindingList_.size();
	pRenderNode->ppTextures_ = (RsTexture**)pFrame->allocMem( sizeof( RsTexture* ) * pRenderNode->NoofTextures_ );
	pRenderNode->pTextureParams_ = (RsTextureParams*)pFrame->allocMem( sizeof( RsTextureParams ) * pRenderNode->NoofTextures_ );
	
	for( BcU32 Idx = 0; Idx < pRenderNode->NoofTextures_; ++Idx )
	{
		TTextureBinding& Binding = TextureBindingList_[ Idx ];
		RsTexture*& Texture = pRenderNode->ppTextures_[ Idx ];
		RsTextureParams& TextureParams = pRenderNode->pTextureParams_[ Idx ];
		
		// Set sampler parameter.
		setParameter( Binding.Parameter_, (BcS32)Idx );
		
		// Set texture to bind.
		Texture = Binding.Texture_->getTexture();
		
		// Default texture parameters.
		// TODO: Pull these from the material.
		RsTextureParams DefaultTextureParams = 
		{
			Texture->levels() > 1 ? rsTFM_LINEAR_MIPMAP_LINEAR : rsTFM_LINEAR,
			rsTFM_LINEAR,
			rsTSM_WRAP,
			rsTSM_WRAP
		};

		// Set texture params.
		TextureParams = DefaultTextureParams;
	}

	// Setup uniform blocks.
	pRenderNode->NoofUniformBlocks_ = (BcU32)UniformBlockBindingList_.size();
	pRenderNode->pUniformBlockIndices_ = (BcU32*)pFrame->allocMem( sizeof( BcU32* ) * pRenderNode->NoofUniformBlocks_ );
	pRenderNode->ppUniformBuffers_ = (RsUniformBuffer**)pFrame->allocMem( sizeof( RsUniformBuffer ) * pRenderNode->NoofUniformBlocks_ );

	for( BcU32 Idx = 0; Idx < UniformBlockBindingList_.size(); ++Idx )
	{
		pRenderNode->pUniformBlockIndices_[ Idx ] = UniformBlockBindingList_[ Idx ].Index_;
		pRenderNode->ppUniformBuffers_[ Idx ] = UniformBlockBindingList_[ Idx ].UniformBuffer_;
	}
	
	// Setup parameter buffer.
	pRenderNode->pParameterBuffer_ = (BcU8*)pFrame->allocMem( ParameterBufferSize_ );
	BcMemCopy( pRenderNode->pParameterBuffer_, pParameterBuffer_, ParameterBufferSize_ );

	// Setup state buffer.
	pRenderNode->pStateBuffer_ = (BcU32*)pFrame->allocMem( sizeof( BcU32 ) * rsRS_MAX );
	BcMemCopy( pRenderNode->pStateBuffer_, pStateBuffer_, sizeof( BcU32 ) * rsRS_MAX );
	
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
