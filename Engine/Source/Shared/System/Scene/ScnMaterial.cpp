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
	const Json::Value& ImportShaders = Object[ "shaders" ];
	const Json::Value& ImportTextures = Object[ "textures" ];
				
	// Import shaders.
	std::map< std::string, ScnShaderRef > Shaders;
	Json::Value::Members ShaderMembers = ImportShaders.getMemberNames();
	ScnShaderRef ShaderRef;
	for( BcU32 Idx = 0; Idx < ShaderMembers.size(); ++Idx )
	{
		const Json::Value& Shader = ImportShaders[ ShaderMembers[ Idx ] ];
			
		if( CsCore::pImpl()->importObject( Shader, ShaderRef ) )
		{
			Shaders[ ShaderMembers[ Idx ] ] = ShaderRef;
		}	
	}
		
	// Import textures.
	std::map< std::string, ScnTextureRef > Textures;
	Json::Value::Members TextureMembers = ImportTextures.getMemberNames();
	ScnTextureRef TextureRef;
	for( BcU32 Idx = 0; Idx < TextureMembers.size(); ++Idx )
	{
		const Json::Value& Texture = ImportTextures[ TextureMembers[ Idx ] ];
			
		if( CsCore::pImpl()->importObject( Texture, TextureRef ) )
		{
			Textures[ TextureMembers[ Idx ] ] = TextureRef;
		}	
	}
		
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
	return BcFalse;
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
		
	}
	else if( pChunk->ID_ == BcHash( "body" ) )
	{
		
	}
}

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnMaterialInstance );

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
BcBool ScnMaterialInstance::isReady()
{
	return Parent_.isReady();
}
