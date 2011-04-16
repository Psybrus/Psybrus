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
BcBool ScnMaterial::import( const Json::Value& Object )
{
	const Json::Value& ImportShader = Object[ "shader" ];
	const Json::Value& ImportTextures = Object[ "textures" ];
				
	// Import shader.
	ScnShaderRef ShaderRef;
	if( CsCore::pImpl()->importObject( ImportShader, ShaderRef ) )
	{
		// Import textures.
		ScnTextureMap Textures;
		Json::Value::Members TextureMembers = ImportTextures.getMemberNames();
		ScnTextureRef TextureRef;
		for( BcU32 Idx = 0; Idx < TextureMembers.size(); ++Idx )
		{
			const Json::Value& Texture = ImportTextures[ TextureMembers[ Idx ] ];
			
			if( CsCore::pImpl()->importObject( Texture, TextureRef ) )
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

	// Build a binding list for textures.
	for( ScnTextureMapConstIterator Iter( TextureMap.begin() ); Iter != TextureMap.end(); ++Iter )
	{
		const std::string& SamplerName = (*Iter).first;
		ScnTextureRef Texture = (*Iter).second;
		
		// Find sampler in program.
		TTextureBinding Binding = 
		{
			pProgram_->findParameter( SamplerName ),
			Texture
		};
		
		// Only add if parameter isn't null.
		if( Binding.pParameter_ != NULL )
		{
			TextureBindingList_.push_back( Binding );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// bind
void ScnMaterialInstance::bind()
{
	// Cache state block.
	RsStateBlock* pStateBlock = RsCore::pImpl()->getStateBlock();
	
	// Default parameters.
	RsTextureParams Params = 
	{
		rsTFM_NEAREST, rsTFM_NEAREST, rsTSM_WRAP, rsTSM_WRAP
	};

	// Bind textures to samplers.
	BcU32 SamplerID = 0;
		
	for( TTextureBindingListIterator Iter( TextureBindingList_.begin() ); Iter != TextureBindingList_.end(); ++Iter )
	{
		TTextureBinding& Binding( *Iter );
		
		// Set state block texture.
		pStateBlock->setTextureState( SamplerID, Binding.Texture_->getTexture(), Params );
		
		// Set parameter ID.
		Binding.pParameter_->setInt( SamplerID++ );
	}
	
	// TODO: Handle binding of other parameters.
	
	// Bind state block.
	pStateBlock->bind();
	
	// Bind program.
	pProgram_->bind();
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnMaterialInstance::isReady()
{
	return Parent_.isReady() && pProgram_->getHandle< BcU64 >() != 0;
}
