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

#ifdef PSY_SERVER
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
		new ReField( "State_", &ScnMaterialImport::State_, bcRFF_IMPORTER ),
	};
		
	ReRegisterClass< ScnMaterialImport, Super >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialImport::ScnMaterialImport()
{
	// Awful default state for now.
	DefaultState_[ (BcU32)RsRenderStateType::DEPTH_WRITE_ENABLE ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::DEPTH_TEST_ENABLE ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::DEPTH_TEST_COMPARE ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_WRITE_MASK ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_TEST_ENABLE ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_COMPARE ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_REF ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_MASK ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_SFAIL ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPFAIL ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPPASS ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::COLOR_WRITE_MASK_0 ] = 0xf;
	DefaultState_[ (BcU32)RsRenderStateType::COLOR_WRITE_MASK_1 ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::COLOR_WRITE_MASK_2 ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::COLOR_WRITE_MASK_3 ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::BLEND_MODE ] = 0;
	DefaultState_[ (BcU32)RsRenderStateType::FILL_MODE ] = 0;
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
		if( State_.find( (RsRenderStateType)Idx ) != State_.end() )
		{
			const std::string& StateValue = State_[ (RsRenderStateType)Idx ];
			
			if( ModeNames.find( StateValue ) != ModeNames.end() )
			{
				BcU32 IntValue = ModeNames[ StateValue ];
				StateBlockStream << BcU32( IntValue );
			}
			else
			{
				BcU32 IntValue = boost::lexical_cast< BcU32 >( StateValue );
				StateBlockStream << BcU32( IntValue );
			}
		}
		else
		{
			StateBlockStream << DefaultState_[ Idx ];
		}
	}
	
	// Add chunks.
	CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
	CsResourceImporter::addChunk( BcHash( "stateblock" ), StateBlockStream.pData(), StateBlockStream.dataSize() );
	
	return BcTrue;

}

#endif
