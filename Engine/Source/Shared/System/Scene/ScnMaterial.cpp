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
#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#include "json.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnMaterial::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& ImportShader = Object[ "shader" ];
	const Json::Value& ImportTextures = Object[ "textures" ];
				
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
		
		THeader Header;
		TTextureHeader TextureHeader;
		
		// Make header.
		BcStrCopyN( Header.ShaderName_, ShaderRef->getName().c_str(), sizeof( Header.ShaderName_ ) );
		Header.NoofTextures_ = Textures.size();
		
		HeaderStream << Header;

		// Make texture headers.
		for( ScnTextureMapIterator Iter( Textures.begin() ); Iter != Textures.end(); ++Iter )
		{
			BcStrCopyN( TextureHeader.SamplerName_, (*Iter).first.c_str(), sizeof( TextureHeader.SamplerName_ ) );
			BcStrCopyN( TextureHeader.TextureName_, (*Iter).second->getName().c_str(), sizeof( TextureHeader.TextureName_ ) );
			
			HeaderStream << TextureHeader;
		}
		
		// Add chunks.
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		
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
// initialise
//virtual
void ScnMaterial::initialise()
{
	pHeader_ = NULL;
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
	return Shader_.isReady();
}

//////////////////////////////////////////////////////////////////////////
// createInstance
BcBool ScnMaterial::createInstance( const std::string& Name, ScnMaterialInstanceRef& Instance, BcU32 PermutationFlags )
{
	return CsCore::pImpl()->createResource( Name, Instance, this, Shader_->getProgram( PermutationFlags ), TextureMap_ );
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnMaterial::fileReady()
{
	// File is ready, get the header chunk.
	pFile_->getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnMaterial::fileChunkReady( const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
		TTextureHeader* pTextureHeaders = (TTextureHeader*)( pHeader_ + 1 );
	
		// Request resources.
		if( CsCore::pImpl()->requestResource( pHeader_->ShaderName_, Shader_ ) )
		{
			ScnTextureRef Texture;
			for( BcU32 Idx = 0; Idx < pHeader_->NoofTextures_; ++Idx )
			{
				TTextureHeader* pTextureHeader = &pTextureHeaders[ Idx ];
				if( CsCore::pImpl()->requestResource( pTextureHeader->TextureName_, Texture ) )
				{
					TextureMap_[ pTextureHeader->SamplerName_ ] = Texture;
				}
			}			
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterialInstance );

//////////////////////////////////////////////////////////////////////////
// initialise
void ScnMaterialInstance::initialise( ScnMaterialRef Parent, RsProgram* pProgram, const ScnTextureMap& TextureMap )
{
	// Cache parent and program.
	Parent_ = Parent;
	pProgram_ = pProgram;
	
	// Allocate parameter buffer.
	ParameterBufferSize_ = pProgram_->getParameterBufferSize();
	pParameterBuffer_ = new BcU8[ ParameterBufferSize_ ];
	

	// Build a binding list for textures.
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
void ScnMaterialInstance::destroy()
{
	delete pParameterBuffer_;
	pParameterBuffer_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
BcU32 ScnMaterialInstance::findParameter( const std::string& ParameterName )
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
	
	return BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialInstance::setParameter( BcU32 Parameter, BcS32 Value )
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
			BcS32* pParameterBuffer = ((BcS32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer = (BcS32)Value;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialInstance::setParameter( BcU32 Parameter, BcReal Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT )
		{
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer = (BcF32)Value;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialInstance::setParameter( BcU32 Parameter, const BcVec2d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC2 )
		{
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer++ = (BcF32)Value.x();
			*pParameterBuffer = (BcF32)Value.y();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialInstance::setParameter( BcU32 Parameter, const BcVec3d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC3 )
		{
			BcF32* pParameterBuffer = ((BcF32*)pParameterBuffer_) + Binding.Offset_;			
			*pParameterBuffer++ = (BcF32)Value.x();
			*pParameterBuffer++ = (BcF32)Value.y();
			*pParameterBuffer = (BcF32)Value.z();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void ScnMaterialInstance::setParameter( BcU32 Parameter, const BcVec4d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_VEC4 )
		{
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
void ScnMaterialInstance::setParameter( BcU32 Parameter, const BcMat3d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_MAT3 )
		{
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
void ScnMaterialInstance::setParameter( BcU32 Parameter, const BcMat4d& Value )
{
	if( Parameter < ParameterBindingList_.size() )
	{
		TParameterBinding& Binding = ParameterBindingList_[ Parameter ];
		if( Binding.Type_ == rsSPT_FLOAT_MAT4 )
		{
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
void ScnMaterialInstance::setTexture( BcU32 Parameter, ScnTextureRef Texture )
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
// bind
class ScnMaterialInstanceRenderNode: public RsRenderNode
{
public:
	ScnMaterialInstanceRenderNode()
	{
		
	}

	virtual void render()
	{
		// Bind program.
		pProgram_->bind( pParameterBuffer_ );
		
		// Iterate over textures and bind.
		for( BcU32 Idx = 0; Idx < NoofTextures_; ++Idx )
		{
			RsTexture* pTexture = ppTextures_[ Idx ];
			RsTextureParams& TextureParams = pTextureParams_[ Idx ];
			pStateBlock_->setTextureState( Idx, pTexture, TextureParams );
		}
		
		// Bind state block.
		pStateBlock_->bind();
	}
	
	RsProgram* pProgram_;
	RsStateBlock* pStateBlock_;

	// Texture binding block.
	BcU32 NoofTextures_;
	RsTexture** ppTextures_;
	RsTextureParams* pTextureParams_;

	// Parameter buffer.
	BcU8* pParameterBuffer_;
};

void ScnMaterialInstance::bind( RsFrame* pFrame, RsRenderSort Sort )
{
	// Default texture parameters.
	RsTextureParams DefaultTextureParams = 
	{
		rsTFM_LINEAR, rsTFM_LINEAR, rsTSM_WRAP, rsTSM_WRAP
	};
	
	// Allocate a render node.
	ScnMaterialInstanceRenderNode* pRenderNode = pFrame->newObject< ScnMaterialInstanceRenderNode >();
	
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
	
		// Set sampler parameter.
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

	// Add node to frame.
	pRenderNode->Sort_ = Sort;
	pFrame->addRenderNode( pRenderNode );
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnMaterialInstance::isReady()
{
	return Parent_.isReady() && pProgram_->getHandle< BcU64 >() != 0;
}
