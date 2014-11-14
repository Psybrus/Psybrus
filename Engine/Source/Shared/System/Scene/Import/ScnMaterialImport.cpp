/**************************************************************************
*
* File:		ScnMaterialImport.cpp
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Scene/Import/ScnMaterialImport.h"
#include "System/Scene/Rendering/ScnMaterialFileData.h"

#include "System/Renderer/RsTypes.h"

#include "Base/BcFile.h"
#include "Base/BcStream.h"

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnMaterialImport )
	
void ScnMaterialImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Shader_", &ScnMaterialImport::Shader_, bcRFF_IMPORTER ),
		new ReField( "Textures_", &ScnMaterialImport::Textures_, bcRFF_IMPORTER ),
		new ReField( "RenderState_", &ScnMaterialImport::RenderState_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnMaterialImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialImport::ScnMaterialImport()
{
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialImport::ScnMaterialImport( ReNoInit )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnMaterialImport::~ScnMaterialImport()
{

}

//////////////////////////////////////////////////////////////////////////
// import
BcBool ScnMaterialImport::import(
		const Json::Value& )
{	
	// Export material.
	BcStream HeaderStream;
	BcStream StateBlockStream;
	
	ScnMaterialHeader Header;
	ScnMaterialTextureHeader TextureHeader;
	
	// Make header.
	Header.ShaderRef_ = Shader_;
	Header.NoofTextures_ = (BcU32)Textures_.size();	
	HeaderStream << Header;

	// Make texture headers.
	for( const auto& Texture : Textures_ )
	{
		TextureHeader.SamplerName_ = 
			CsResourceImporter::addString( BcName::StripInvalidChars( Texture.first.c_str() ).c_str() );
		TextureHeader.TextureRef_ = Texture.second;
		HeaderStream << TextureHeader;
	}
		
	// Add chunks.
	CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
	CsResourceImporter::addChunk( BcHash( "renderstate" ), &RenderState_, sizeof( RenderState_ ) );
	
	return BcTrue;
}
