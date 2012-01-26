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

#include "ScnMaterial.h"
#include "ScnEntity.h"
#include "CsCore.h"

#include "ScnTextureAtlas.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnMaterial::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& ImportShader = Object[ "shader" ];
	const Json::Value& ImportTextures = Object[ "textures" ];
	const Json::Value& State = Object[ "state" ];
	
	// Import shader.
	ScnShaderRef ShaderRef;
	if( CsCore::pImpl()->importObject( ImportShader, ShaderRef, DependancyList ) )
	{
		// Import textures.
		ScnTextureMap Textures;
		Json::Value::Members TextureMembers = ImportTextures.getMemberNames();
		ScnTextureRef TextureRef;
		for( BcU32 Idx = 0; Idx < TextureMembers.size(); ++Idx )
		{
			const Json::Value& Texture = ImportTextures[ TextureMembers[ Idx ] ];
			
			if( CsCore::pImpl()->importObject( Texture, TextureRef, DependancyList ) )
			{
				Textures[ TextureMembers[ Idx ] ] = TextureRef;
			}	
			else
			{
				BcPrintf( "ScnMaterial: Failed to import texture.\n" );
			}
		}
		
		// Export material.
		BcStream HeaderStream;
		BcStream StateBlockStream;
		
		THeader Header;
		TTextureHeader TextureHeader;
		
		// Make header.
		BcStrCopyN( Header.ShaderName_, (*ShaderRef->getName()).c_str(), sizeof( Header.ShaderName_ ) );
		Header.NoofTextures_ = Textures.size();
		
		HeaderStream << Header;

		// Make texture headers.
		for( ScnTextureMapIterator Iter( Textures.begin() ); Iter != Textures.end(); ++Iter )
		{
			BcStrCopyN( TextureHeader.SamplerName_, (*Iter).first.c_str(), sizeof( TextureHeader.SamplerName_ ) );
			BcStrCopyN( TextureHeader.TextureName_, (*(*Iter).second->getName()).c_str(), sizeof( TextureHeader.TextureName_ ) );
			BcStrCopyN( TextureHeader.TextureType_, (*(*Iter).second->getType()).c_str(), sizeof( TextureHeader.TextureType_ ) );
			
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
					StateBlockStream << BcU32( IntValue );
				}
				else
				{
					BcU32 IntValue = StateValue.asUInt();
					StateBlockStream << BcU32( IntValue );
				}
			}
			else
			{
				StateBlockStream << BcU32( 0 );
			}
		}
		
		// Add chunks.
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "stateblock" ), StateBlockStream.pData(), StateBlockStream.dataSize() );
		
		return BcTrue;
	}
	else
	{
		BcPrintf( "ScnMaterial: Failed to import shader.\n" );
	}		
	
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterial );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnMaterial::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	Super::StaticPropertyTable( PropertyTable );

	PropertyTable.beginCatagory( "ScnMaterial" )
		.field( "shader",		csPVT_RESOURCE,		csPCT_VALUE,	"ScnShader" )
		.field( "textures",		csPVT_RESOURCE,		csPCT_MAP,		"ScnTexture" )
		.field( "state",		csPVT_STRING,		csPCT_MAP )
	.endCatagory();
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
// fileReady
void ScnMaterial::fileReady()
{
	// File is ready, get the header chunk.
	getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnMaterial::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	// If we have no render core get chunk 0 so we keep getting entered into.
	if( RsCore::pImpl() == NULL )
	{
		getChunk( 0 );
		return;
	}
	
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
		TTextureHeader* pTextureHeaders = (TTextureHeader*)( pHeader_ + 1 );
		
		// Request resources.
		if( CsCore::pImpl()->requestResource( pHeader_->ShaderName_, Shader_ ) )
		{
			// HACK: We should be able to handle resource subtypes without this.
			ScnTextureRef Texture;
			CsResourceRef<>& InternalHandle = *( reinterpret_cast< CsResourceRef<>* >( &Texture ) );
			for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
			{
				TTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
				
				if( CsCore::pImpl()->internalRequestResource( pTextureHeader->TextureName_, pTextureHeader->TextureType_, InternalHandle ) )
				{
					TextureMap_[ pTextureHeader->SamplerName_ ] = Texture;
				}
			}
		}
		
		getChunk( ++ChunkIdx );
	}
	else if( pChunk->ID_ == BcHash( "stateblock" ) )
	{
		pStateBuffer_ = (BcU32*)pData;
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterialComponent );

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void ScnMaterialComponent::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	PropertyTable.beginCatagory( "ScnMaterialComponent" )
		//.field( "source",					csPVT_FILE,			csPCT_LIST )
	.endCatagory();
}

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
	if( pProgram_->findParameterOffset( ParameterName, Type, Offset ) )
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
	}
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
			pStateBlock_->setTextureState( Idx, pTexture, TextureParams );
		}

		// Setup states.
		for( BcU32 Idx = 0; Idx < rsRS_MAX; ++Idx )
		{
			pStateBlock_->setRenderState( (eRsRenderState)Idx, pStateBuffer_[ Idx ], BcFalse );
		}
		
		// Bind state block.
		pStateBlock_->bind();

		// Bind program.
		pProgram_->bind( pParameterBuffer_ );
	}
	
	RsProgram* pProgram_;
	RsStateBlock* pStateBlock_;

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

void ScnMaterialComponent::bind( RsFrame* pFrame, RsRenderSort Sort )
{
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
	pRenderNode->pStateBlock_ = RsCore::pImpl()->getStateBlock();
	
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
