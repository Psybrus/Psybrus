/**************************************************************************
*
* File:		RsCore.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsCore.h"

size_t RsCore::JOB_QUEUE_ID = (size_t)-1;

//////////////////////////////////////////////////////////////////////////
// Reflection
REFLECTION_DEFINE_DERIVED( RsCore );

void RsCore::StaticRegisterClass()
{
	ReRegisterAbstractClass< RsCore, Super >();

	ReEnumConstant* RsBlendingModeEnumConstants[] = 
	{
		new ReEnumConstant( "NONE", (BcU32)RsBlendingMode::NONE ),
		new ReEnumConstant( "BLEND", (BcU32)RsBlendingMode::BLEND ),
		new ReEnumConstant( "ADD", (BcU32)RsBlendingMode::ADD ),
		new ReEnumConstant( "SUBTRACT", (BcU32)RsBlendingMode::SUBTRACT ),
		new ReEnumConstant( "INVALID", (BcU32)RsBlendingMode::INVALID ),
	};
	ReRegisterEnum< RsBlendingMode >( RsBlendingModeEnumConstants );

	ReEnumConstant* RsFillModeEnumConstants[] = 
	{
		new ReEnumConstant( "SOLID", (BcU32)RsFillMode::SOLID ),
		new ReEnumConstant( "WIREFRAME", (BcU32)RsFillMode::WIREFRAME ),
		new ReEnumConstant( "INVALID", (BcU32)RsFillMode::INVALID ),
	};
	ReRegisterEnum< RsFillMode >( RsFillModeEnumConstants );

	ReEnumConstant* RsCullModeEnumConstants[] = 
	{
		new ReEnumConstant( "NONE", (BcU32)RsCullMode::NONE ),
		new ReEnumConstant( "CW", (BcU32)RsCullMode::CW ),
		new ReEnumConstant( "CCW", (BcU32)RsCullMode::CCW ),
		new ReEnumConstant( "INVALID", (BcU32)RsCullMode::INVALID ),
	};
	ReRegisterEnum< RsCullMode >( RsCullModeEnumConstants );

	ReEnumConstant* RsBlendTypeEnumConstants[] = 
	{
		new ReEnumConstant( "ZERO", (BcU32)RsBlendType::ZERO ),
		new ReEnumConstant( "ONE", (BcU32)RsBlendType::ONE ),
		new ReEnumConstant( "SRCCOLOUR", (BcU32)RsBlendType::SRC_COLOUR ),
		new ReEnumConstant( "INVSRCCOLOUR", (BcU32)RsBlendType::INV_SRC_COLOUR ),
		new ReEnumConstant( "SRCALPHA", (BcU32)RsBlendType::SRC_ALPHA ),
		new ReEnumConstant( "INVSRCALPHA", (BcU32)RsBlendType::INV_SRC_ALPHA ),
		new ReEnumConstant( "DESTCOLOUR", (BcU32)RsBlendType::DEST_COLOUR ),
		new ReEnumConstant( "INVDESTCOLOUR", (BcU32)RsBlendType::INV_DEST_COLOUR ),
		new ReEnumConstant( "DESTALPHA", (BcU32)RsBlendType::DEST_ALPHA ),
		new ReEnumConstant( "INVDESTALPHA", (BcU32)RsBlendType::INV_DEST_ALPHA ),
		new ReEnumConstant( "INVALID", (BcU32)RsBlendType::INVALID ),
	};
	ReRegisterEnum< RsBlendType >( RsBlendTypeEnumConstants );

	ReEnumConstant* RsBlendOpEnumConstants[] = 
	{
		new ReEnumConstant( "ADD", (BcU32)RsBlendOp::ADD ),
		new ReEnumConstant( "SUBTRACT", (BcU32)RsBlendOp::SUBTRACT ),
		new ReEnumConstant( "REVSUBTRACT", (BcU32)RsBlendOp::REV_SUBTRACT ),
		new ReEnumConstant( "MINIMUM", (BcU32)RsBlendOp::MINIMUM ),
		new ReEnumConstant( "MAXIMUM", (BcU32)RsBlendOp::MAXIMUM ),
		new ReEnumConstant( "INVALID", (BcU32)RsBlendOp::INVALID ),
	};
	ReRegisterEnum< RsBlendOp >( RsBlendOpEnumConstants );

	ReEnumConstant* RsCompareModeEnumConstants[] = 
	{
		new ReEnumConstant( "NEVER", (BcU32)RsCompareMode::NEVER ),
		new ReEnumConstant( "LESS", (BcU32)RsCompareMode::LESS ),
		new ReEnumConstant( "EQUAL", (BcU32)RsCompareMode::EQUAL ),
		new ReEnumConstant( "LESSEQUAL", (BcU32)RsCompareMode::LESSEQUAL ),
		new ReEnumConstant( "GREATER", (BcU32)RsCompareMode::GREATER ),
		new ReEnumConstant( "NOTEQUAL", (BcU32)RsCompareMode::NOTEQUAL ),
		new ReEnumConstant( "GREATEREQUAL", (BcU32)RsCompareMode::GREATEREQUAL ),
		new ReEnumConstant( "ALWAYS", (BcU32)RsCompareMode::ALWAYS ),
		new ReEnumConstant( "INVALID", (BcU32)RsCompareMode::INVALID ),
	};
	ReRegisterEnum< RsCompareMode >( RsCompareModeEnumConstants );

	ReEnumConstant* RsStencilOpEnumConstants[] = 
	{
		new ReEnumConstant( "KEEP", (BcU32)RsStencilOp::KEEP ),
		new ReEnumConstant( "ZERO", (BcU32)RsStencilOp::ZERO ),
		new ReEnumConstant( "REPLACE", (BcU32)RsStencilOp::REPLACE ),
		new ReEnumConstant( "INCR", (BcU32)RsStencilOp::INCR ),
		new ReEnumConstant( "INCR_WRAP", (BcU32)RsStencilOp::INCR_WRAP ),
		new ReEnumConstant( "DECR", (BcU32)RsStencilOp::DECR ),
		new ReEnumConstant( "DECR_WRAP", (BcU32)RsStencilOp::DECR_WRAP ),
		new ReEnumConstant( "INVERT", (BcU32)RsStencilOp::INVERT ),
		new ReEnumConstant( "INVALID", (BcU32)RsStencilOp::INVALID ),
	};
	ReRegisterEnum< RsStencilOp >( RsStencilOpEnumConstants );

	ReEnumConstant* RsTextureFilteringModeEnumConstants[] = 
	{
		new ReEnumConstant( "NEAREST", (BcU32)RsTextureFilteringMode::NEAREST ),
		new ReEnumConstant( "LINEAR", (BcU32)RsTextureFilteringMode::LINEAR ),
		new ReEnumConstant( "NEAREST_MIPMAP_NEAREST", (BcU32)RsTextureFilteringMode::NEAREST_MIPMAP_NEAREST ),
		new ReEnumConstant( "LINEAR_MIPMAP_NEAREST", (BcU32)RsTextureFilteringMode::LINEAR_MIPMAP_NEAREST ),
		new ReEnumConstant( "NEAREST_MIPMAP_LINEAR", (BcU32)RsTextureFilteringMode::NEAREST_MIPMAP_LINEAR ),
		new ReEnumConstant( "LINEAR_MIPMAP_LINEAR", (BcU32)RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR ),
		new ReEnumConstant( "INVALID", (BcU32)RsTextureFilteringMode::INVALID ),
	};
	ReRegisterEnum< RsTextureFilteringMode >( RsTextureFilteringModeEnumConstants );

	ReEnumConstant* RsTextureSamplingModeEnumConstants[] = 
	{
		new ReEnumConstant( "WRAP", (BcU32)RsTextureSamplingMode::WRAP ),
		new ReEnumConstant( "MIRROR", (BcU32)RsTextureSamplingMode::MIRROR ),
		new ReEnumConstant( "CLAMP", (BcU32)RsTextureSamplingMode::CLAMP ),
		new ReEnumConstant( "DECAL", (BcU32)RsTextureSamplingMode::DECAL ),
		new ReEnumConstant( "INVALID", (BcU32)RsTextureSamplingMode::INVALID ),
	};
	ReRegisterEnum< RsTextureSamplingMode >( RsTextureSamplingModeEnumConstants );

	ReEnumConstant* RsVertexDataTypeEnumConstants[] = 
	{
		new ReEnumConstant( "FLOAT32", (BcU32)RsVertexDataType::FLOAT32 ),
		new ReEnumConstant( "FLOAT16", (BcU32)RsVertexDataType::FLOAT16 ),
		new ReEnumConstant( "FIXED", (BcU32)RsVertexDataType::FIXED ),
		new ReEnumConstant( "BYTE", (BcU32)RsVertexDataType::BYTE ),
		new ReEnumConstant( "BYTE_NORM", (BcU32)RsVertexDataType::BYTE_NORM ),
		new ReEnumConstant( "UBYTE_NORM", (BcU32)RsVertexDataType::UBYTE_NORM ),
		new ReEnumConstant( "SHORT", (BcU32)RsVertexDataType::SHORT ),
		new ReEnumConstant( "SHORT_NORM", (BcU32)RsVertexDataType::SHORT_NORM ),
		new ReEnumConstant( "USHORT", (BcU32)RsVertexDataType::USHORT ),
		new ReEnumConstant( "USHORT_NORM", (BcU32)RsVertexDataType::USHORT_NORM ),
		new ReEnumConstant( "INT", (BcU32)RsVertexDataType::INT ),
		new ReEnumConstant( "INT_NORM", (BcU32)RsVertexDataType::INT_NORM ),
		new ReEnumConstant( "UINT", (BcU32)RsVertexDataType::UINT ),
		new ReEnumConstant( "UINT_NORM", (BcU32)RsVertexDataType::UINT_NORM ),
		new ReEnumConstant( "INVALID", (BcU32)RsVertexDataType::INVALID ),
	};
	ReRegisterEnum< RsVertexDataType >( RsVertexDataTypeEnumConstants );

	ReEnumConstant* RsTextureFormatEnumConstants[] = 
	{
		new ReEnumConstant( "R8", (BcU32)RsTextureFormat::R8 ),
		new ReEnumConstant( "R8G8", (BcU32)RsTextureFormat::R8G8 ),
		new ReEnumConstant( "R8G8B8", (BcU32)RsTextureFormat::R8G8B8 ),
		new ReEnumConstant( "R8G8B8A8", (BcU32)RsTextureFormat::R8G8B8A8 ),
		new ReEnumConstant( "R16F", (BcU32)RsTextureFormat::R16F ),
		new ReEnumConstant( "R16FG16F", (BcU32)RsTextureFormat::R16FG16F ),
		new ReEnumConstant( "R16FG16FB16F", (BcU32)RsTextureFormat::R16FG16FB16F ),
		new ReEnumConstant( "R16FG16FB16FA16F", (BcU32)RsTextureFormat::R16FG16FB16FA16F ),
		new ReEnumConstant( "R32F", (BcU32)RsTextureFormat::R32F ),
		new ReEnumConstant( "R32FG32F", (BcU32)RsTextureFormat::R32FG32F ),
		new ReEnumConstant( "R32FG32FB32F", (BcU32)RsTextureFormat::R32FG32FB32F ),
		new ReEnumConstant( "R32FG32FB32FA32F", (BcU32)RsTextureFormat::R32FG32FB32FA32F ),
		new ReEnumConstant( "DXT1", (BcU32)RsTextureFormat::DXT1 ),
		new ReEnumConstant( "DXT3", (BcU32)RsTextureFormat::DXT3 ),
		new ReEnumConstant( "DXT5", (BcU32)RsTextureFormat::DXT5 ),
		new ReEnumConstant( "ETC1", (BcU32)RsTextureFormat::ETC1 ),
		new ReEnumConstant( "D16", (BcU32)RsTextureFormat::D16 ),
		new ReEnumConstant( "D24", (BcU32)RsTextureFormat::D24 ),
		new ReEnumConstant( "D32", (BcU32)RsTextureFormat::D32 ),
		new ReEnumConstant( "D24S8", (BcU32)RsTextureFormat::D24S8 ),
		new ReEnumConstant( "D32F", (BcU32)RsTextureFormat::D32F ),
		new ReEnumConstant( "INVALID", (BcU32)RsTextureFormat::INVALID ),
	};
	ReRegisterEnum< RsTextureFormat >( RsTextureFormatEnumConstants );

	ReEnumConstant* RsShaderTypeEnumConstants[] = 
	{
		new ReEnumConstant( "VERTEX", (BcU32)RsShaderType::VERTEX ),
		new ReEnumConstant( "HULL", (BcU32)RsShaderType::HULL ),
		new ReEnumConstant( "DOMAIN", (BcU32)RsShaderType::DOMAIN ),
		new ReEnumConstant( "GEOMETRY", (BcU32)RsShaderType::GEOMETRY ),
		new ReEnumConstant( "PIXEL", (BcU32)RsShaderType::PIXEL ),
		new ReEnumConstant( "COMPUTE", (BcU32)RsShaderType::COMPUTE ),
	};
	ReRegisterEnum< RsShaderType >( RsShaderTypeEnumConstants );

	ReEnumConstant* RsShaderBackendTypeEnumConstants[] = 
	{
		new ReEnumConstant( "GLSL", (BcU32)RsShaderBackendType::GLSL ),
		new ReEnumConstant( "GLSL_ES", (BcU32)RsShaderBackendType::GLSL_ES ),
		new ReEnumConstant( "D3D9", (BcU32)RsShaderBackendType::D3D9 ),
		new ReEnumConstant( "D3D11", (BcU32)RsShaderBackendType::D3D11 ),
		new ReEnumConstant( "D3D12", (BcU32)RsShaderBackendType::D3D12 ),
		new ReEnumConstant( "MANTLE", (BcU32)RsShaderBackendType::MANTLE ),
	};
	ReRegisterEnum< RsShaderBackendType >( RsShaderBackendTypeEnumConstants );

	ReEnumConstant* RsShaderCodeTypeEnumConstants[] = 
	{
		new ReEnumConstant( "GLSL_140", (BcU32)RsShaderCodeType::GLSL_140 ),
		new ReEnumConstant( "GLSL_150", (BcU32)RsShaderCodeType::GLSL_150 ),
		new ReEnumConstant( "GLSL_330", (BcU32)RsShaderCodeType::GLSL_330 ),
		new ReEnumConstant( "GLSL_400", (BcU32)RsShaderCodeType::GLSL_400 ),
		new ReEnumConstant( "GLSL_410", (BcU32)RsShaderCodeType::GLSL_410 ),
		new ReEnumConstant( "GLSL_420", (BcU32)RsShaderCodeType::GLSL_420 ),
		new ReEnumConstant( "GLSL_430", (BcU32)RsShaderCodeType::GLSL_430 ),
		new ReEnumConstant( "GLSL_440", (BcU32)RsShaderCodeType::GLSL_440 ),
		new ReEnumConstant( "GLSL_450", (BcU32)RsShaderCodeType::GLSL_450 ),
		new ReEnumConstant( "GLSL_ES_100", (BcU32)RsShaderCodeType::GLSL_ES_100 ),
		new ReEnumConstant( "GLSL_ES_300", (BcU32)RsShaderCodeType::GLSL_ES_300 ),
		new ReEnumConstant( "GLSL_ES_310", (BcU32)RsShaderCodeType::GLSL_ES_310 ),
		new ReEnumConstant( "D3D9_3_0", (BcU32)RsShaderCodeType::D3D9_3_0 ),
		new ReEnumConstant( "D3D11_4_0_LEVEL_9_1", (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_1 ),
		new ReEnumConstant( "D3D11_4_0_LEVEL_9_2", (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_2 ),
		new ReEnumConstant( "D3D11_4_0_LEVEL_9_3", (BcU32)RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ),
		new ReEnumConstant( "D3D11_4_0", (BcU32)RsShaderCodeType::D3D11_4_0 ),
		new ReEnumConstant( "D3D11_4_1", (BcU32)RsShaderCodeType::D3D11_4_1 ),
		new ReEnumConstant( "D3D11_5_0", (BcU32)RsShaderCodeType::D3D11_5_0 ),
		new ReEnumConstant( "D3D11_5_1", (BcU32)RsShaderCodeType::D3D11_5_1 ),
	};
	ReRegisterEnum< RsShaderCodeType >( RsShaderCodeTypeEnumConstants );

	ReEnumConstant* RsTextureTypeEnumConstants[] = 
	{
		new ReEnumConstant( "UNKNOWN", (BcU32)RsTextureType::UNKNOWN ),
		new ReEnumConstant( "TEX1D", (BcU32)RsTextureType::TEX1D ),
		new ReEnumConstant( "TEX2D", (BcU32)RsTextureType::TEX2D ),
		new ReEnumConstant( "TEX3D", (BcU32)RsTextureType::TEX3D ),
		new ReEnumConstant( "TEXCUBE", (BcU32)RsTextureType::TEXCUBE ),

		// Additional aliases.
		new ReEnumConstant( "1D", (BcU32)RsTextureType::TEX1D ),
		new ReEnumConstant( "2D", (BcU32)RsTextureType::TEX2D ),
		new ReEnumConstant( "3D", (BcU32)RsTextureType::TEX3D ),
		new ReEnumConstant( "CUBE", (BcU32)RsTextureType::TEXCUBE ),
	};
	ReRegisterEnum< RsTextureType >( RsTextureTypeEnumConstants );

}


//////////////////////////////////////////////////////////////////////////
// Resource deletion.
void RsResourceDeleters::operator()( class RsBuffer* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}

void RsResourceDeleters::operator()( class RsContext* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}

void RsResourceDeleters::operator()( class RsFrameBuffer* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}

void RsResourceDeleters::operator()( class RsProgram* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}

void RsResourceDeleters::operator()( class RsRenderState* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}

void RsResourceDeleters::operator()( class RsSamplerState* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}

void RsResourceDeleters::operator()( class RsShader* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}

void RsResourceDeleters::operator()( class RsTexture* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}


void RsResourceDeleters::operator()( class RsVertexDeclaration* Resource )
{
	RsCore::pImpl()->destroyResource( Resource );
}
