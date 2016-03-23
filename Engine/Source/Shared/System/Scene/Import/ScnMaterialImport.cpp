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

#if PSY_IMPORT_PIPELINE

#include "Base/BcFile.h"
#include "Base/BcStream.h"

#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnMaterialImport )
	
void ScnMaterialImport::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Shader_", &ScnMaterialImport::Shader_, bcRFF_IMPORTER ),
		new ReField( "Textures_", &ScnMaterialImport::Textures_, bcRFF_IMPORTER ),
		new ReField( "Samplers_", &ScnMaterialImport::Samplers_, bcRFF_IMPORTER ),
		new ReField( "RenderState_", &ScnMaterialImport::RenderState_, bcRFF_IMPORTER ),
		new ReField( "AutomaticUniformBlocks_", &ScnMaterialImport::AutomaticUniformBlocks_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnMaterialImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialImport::ScnMaterialImport():
	CsResourceImporter( "<INVALID>", "ScnMaterial" )
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
BcBool ScnMaterialImport::import()
{	
#if PSY_IMPORT_PIPELINE
	// Export material.
	BcStream HeaderStream;
	BcStream StateBlockStream;
	
	ScnMaterialHeader Header;
	ScnMaterialTextureHeader TextureHeader;
	ScnMaterialUniformBlockName UniformBlockName;
	
	// Make header.
	Header.ShaderRef_ = Shader_;
	Header.NoofTextures_ = (BcU32)Samplers_.size();	
	Header.NoofAutomaticUniformBlocks_ = (BcU32)AutomaticUniformBlocks_.size();	
	HeaderStream << Header;

	// Make texture headers.
	for( const auto& Sampler : Samplers_ )
	{
		TextureHeader.SamplerName_ = 
			CsResourceImporter::addString( BcName::StripInvalidChars( Sampler.first.c_str() ).c_str() );
		TextureHeader.SamplerStateDesc_ = Sampler.second;

		// See if we have a texture we can use.
		auto FoundIt = Textures_.find( Sampler.first );
		if( FoundIt != Textures_.end() )
		{
			TextureHeader.TextureRef_ = FoundIt->second;
		}
		else
		{
			TextureHeader.TextureRef_ = 0;
		}

		HeaderStream << TextureHeader;
	}

	// Automatic uniform blocks.
	for( const auto& AutomaticUniformBlock : AutomaticUniformBlocks_ )
	{
		ScnMaterialUniformBlockName UniformBlockName = 
		{
			BcName( CsResourceImporter::addString( (*AutomaticUniformBlock).c_str() ) )
		};
		HeaderStream << UniformBlockName;
	}

	// Add chunks.
	CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
	CsResourceImporter::addChunk( BcHash( "renderstate" ), &RenderState_, sizeof( RenderState_ ) );
	
	return BcTrue;
#else
	return BcFalse;
#endif // PSY_IMPORT_PIPELINE
}

//////////////////////////////////////////////////////////////////////////
// addTexture
void ScnMaterialImport::addTexture( const std::string& Name, CsCrossRefId Texture, RsSamplerStateDesc SamplerState )
{
	Textures_[ Name ] = Texture;
	Samplers_[ Name ] = SamplerState;
}
