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
#include "System/Scene/ScnMaterialFileData.h"

#include "System/Renderer/RsTypes.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( ScnMaterialImport )
	
void ScnMaterialImport::StaticRegisterClass()
{
	/*
	ReField* Fields[] = 
	{
		new ReField( "Source_", &ScnMaterialImport::Source_ ),
	};
	*/
		
	ReRegisterClass< ScnMaterialImport, Super >();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnMaterialImport::ScnMaterialImport()
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
		const Json::Value& Object )
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

		TextureHeader.SamplerName_ = CsResourceImporter::addString( BcName::StripInvalidChars( TextureMembers[ Idx ].c_str() ).c_str() );
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
	CsResourceImporter::addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
	CsResourceImporter::addChunk( BcHash( "stateblock" ), StateBlockStream.pData(), StateBlockStream.dataSize() );
	
	return BcTrue;

}

#endif
