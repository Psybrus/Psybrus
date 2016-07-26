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
		new ReEnumConstant( "NONE", RsBlendingMode::NONE ),
		new ReEnumConstant( "BLEND", RsBlendingMode::BLEND ),
		new ReEnumConstant( "ADD", RsBlendingMode::ADD ),
		new ReEnumConstant( "SUBTRACT", RsBlendingMode::SUBTRACT ),
		new ReEnumConstant( "INVALID", RsBlendingMode::INVALID ),
	};
	ReRegisterEnum< RsBlendingMode >( RsBlendingModeEnumConstants );

	ReEnumConstant* RsFillModeEnumConstants[] = 
	{
		new ReEnumConstant( "SOLID", RsFillMode::SOLID ),
		new ReEnumConstant( "WIREFRAME", RsFillMode::WIREFRAME ),
		new ReEnumConstant( "INVALID", RsFillMode::INVALID ),
	};
	ReRegisterEnum< RsFillMode >( RsFillModeEnumConstants );

	ReEnumConstant* RsCullModeEnumConstants[] = 
	{
		new ReEnumConstant( "NONE", RsCullMode::NONE ),
		new ReEnumConstant( "CW", RsCullMode::CW ),
		new ReEnumConstant( "CCW", RsCullMode::CCW ),
		new ReEnumConstant( "INVALID", RsCullMode::INVALID ),
	};
	ReRegisterEnum< RsCullMode >( RsCullModeEnumConstants );

	ReEnumConstant* RsBlendTypeEnumConstants[] = 
	{
		new ReEnumConstant( "ZERO", RsBlendType::ZERO ),
		new ReEnumConstant( "ONE", RsBlendType::ONE ),
		new ReEnumConstant( "SRCCOLOUR", RsBlendType::SRC_COLOUR ),
		new ReEnumConstant( "INVSRCCOLOUR", RsBlendType::INV_SRC_COLOUR ),
		new ReEnumConstant( "SRCALPHA", RsBlendType::SRC_ALPHA ),
		new ReEnumConstant( "INVSRCALPHA", RsBlendType::INV_SRC_ALPHA ),
		new ReEnumConstant( "DESTCOLOUR", RsBlendType::DEST_COLOUR ),
		new ReEnumConstant( "INVDESTCOLOUR", RsBlendType::INV_DEST_COLOUR ),
		new ReEnumConstant( "DESTALPHA", RsBlendType::DEST_ALPHA ),
		new ReEnumConstant( "INVDESTALPHA", RsBlendType::INV_DEST_ALPHA ),
		new ReEnumConstant( "INVALID", RsBlendType::INVALID ),
	};
	ReRegisterEnum< RsBlendType >( RsBlendTypeEnumConstants );

	ReEnumConstant* RsBlendOpEnumConstants[] = 
	{
		new ReEnumConstant( "ADD", RsBlendOp::ADD ),
		new ReEnumConstant( "SUBTRACT", RsBlendOp::SUBTRACT ),
		new ReEnumConstant( "REVSUBTRACT", RsBlendOp::REV_SUBTRACT ),
		new ReEnumConstant( "MINIMUM", RsBlendOp::MINIMUM ),
		new ReEnumConstant( "MAXIMUM", RsBlendOp::MAXIMUM ),
		new ReEnumConstant( "INVALID", RsBlendOp::INVALID ),
	};
	ReRegisterEnum< RsBlendOp >( RsBlendOpEnumConstants );

	ReEnumConstant* RsCompareModeEnumConstants[] = 
	{
		new ReEnumConstant( "NEVER", RsCompareMode::NEVER ),
		new ReEnumConstant( "LESS", RsCompareMode::LESS ),
		new ReEnumConstant( "EQUAL", RsCompareMode::EQUAL ),
		new ReEnumConstant( "LESSEQUAL", RsCompareMode::LESSEQUAL ),
		new ReEnumConstant( "GREATER", RsCompareMode::GREATER ),
		new ReEnumConstant( "NOTEQUAL", RsCompareMode::NOTEQUAL ),
		new ReEnumConstant( "GREATEREQUAL", RsCompareMode::GREATEREQUAL ),
		new ReEnumConstant( "ALWAYS", RsCompareMode::ALWAYS ),
		new ReEnumConstant( "INVALID", RsCompareMode::INVALID ),
	};
	ReRegisterEnum< RsCompareMode >( RsCompareModeEnumConstants );

	ReEnumConstant* RsStencilOpEnumConstants[] = 
	{
		new ReEnumConstant( "KEEP", RsStencilOp::KEEP ),
		new ReEnumConstant( "ZERO", RsStencilOp::ZERO ),
		new ReEnumConstant( "REPLACE", RsStencilOp::REPLACE ),
		new ReEnumConstant( "INCR", RsStencilOp::INCR ),
		new ReEnumConstant( "INCR_WRAP", RsStencilOp::INCR_WRAP ),
		new ReEnumConstant( "DECR", RsStencilOp::DECR ),
		new ReEnumConstant( "DECR_WRAP", RsStencilOp::DECR_WRAP ),
		new ReEnumConstant( "INVERT", RsStencilOp::INVERT ),
		new ReEnumConstant( "INVALID", RsStencilOp::INVALID ),
	};
	ReRegisterEnum< RsStencilOp >( RsStencilOpEnumConstants );

	ReEnumConstant* RsTextureFilteringModeEnumConstants[] = 
	{
		new ReEnumConstant( "NEAREST", RsTextureFilteringMode::NEAREST ),
		new ReEnumConstant( "LINEAR", RsTextureFilteringMode::LINEAR ),
		new ReEnumConstant( "NEAREST_MIPMAP_NEAREST", RsTextureFilteringMode::NEAREST_MIPMAP_NEAREST ),
		new ReEnumConstant( "LINEAR_MIPMAP_NEAREST", RsTextureFilteringMode::LINEAR_MIPMAP_NEAREST ),
		new ReEnumConstant( "NEAREST_MIPMAP_LINEAR", RsTextureFilteringMode::NEAREST_MIPMAP_LINEAR ),
		new ReEnumConstant( "LINEAR_MIPMAP_LINEAR", RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR ),
		new ReEnumConstant( "INVALID", RsTextureFilteringMode::INVALID ),
	};
	ReRegisterEnum< RsTextureFilteringMode >( RsTextureFilteringModeEnumConstants );

	ReEnumConstant* RsTextureSamplingModeEnumConstants[] = 
	{
		new ReEnumConstant( "WRAP", RsTextureSamplingMode::WRAP ),
		new ReEnumConstant( "MIRROR", RsTextureSamplingMode::MIRROR ),
		new ReEnumConstant( "CLAMP", RsTextureSamplingMode::CLAMP ),
		new ReEnumConstant( "DECAL", RsTextureSamplingMode::DECAL ),
		new ReEnumConstant( "INVALID", RsTextureSamplingMode::INVALID ),
	};
	ReRegisterEnum< RsTextureSamplingMode >( RsTextureSamplingModeEnumConstants );

	ReEnumConstant* RsVertexDataTypeEnumConstants[] = 
	{
		new ReEnumConstant( "FLOAT32", RsVertexDataType::FLOAT32 ),
		new ReEnumConstant( "FLOAT16", RsVertexDataType::FLOAT16 ),
		new ReEnumConstant( "FIXED", RsVertexDataType::FIXED ),
		new ReEnumConstant( "BYTE", RsVertexDataType::BYTE ),
		new ReEnumConstant( "BYTE_NORM", RsVertexDataType::BYTE_NORM ),
		new ReEnumConstant( "UBYTE_NORM", RsVertexDataType::UBYTE_NORM ),
		new ReEnumConstant( "SHORT", RsVertexDataType::SHORT ),
		new ReEnumConstant( "SHORT_NORM", RsVertexDataType::SHORT_NORM ),
		new ReEnumConstant( "USHORT", RsVertexDataType::USHORT ),
		new ReEnumConstant( "USHORT_NORM", RsVertexDataType::USHORT_NORM ),
		new ReEnumConstant( "INT", RsVertexDataType::INT ),
		new ReEnumConstant( "INT_NORM", RsVertexDataType::INT_NORM ),
		new ReEnumConstant( "UINT", RsVertexDataType::UINT ),
		new ReEnumConstant( "UINT_NORM", RsVertexDataType::UINT_NORM ),
		new ReEnumConstant( "INVALID", RsVertexDataType::INVALID ),
	};
	ReRegisterEnum< RsVertexDataType >( RsVertexDataTypeEnumConstants );

	ReEnumConstant* RsTextureFormatEnumConstants[] = 
	{
		new ReEnumConstant( "R8", RsTextureFormat::R8 ),
		new ReEnumConstant( "R8G8", RsTextureFormat::R8G8 ),
		new ReEnumConstant( "R8G8B8", RsTextureFormat::R8G8B8 ),
		new ReEnumConstant( "R8G8B8A8", RsTextureFormat::R8G8B8A8 ),
		new ReEnumConstant( "R16F", RsTextureFormat::R16F ),
		new ReEnumConstant( "R16FG16F", RsTextureFormat::R16FG16F ),
		new ReEnumConstant( "R16FG16FB16F", RsTextureFormat::R16FG16FB16F ),
		new ReEnumConstant( "R16FG16FB16FA16F", RsTextureFormat::R16FG16FB16FA16F ),
		new ReEnumConstant( "R32F", RsTextureFormat::R32F ),
		new ReEnumConstant( "R32FG32F", RsTextureFormat::R32FG32F ),
		new ReEnumConstant( "R32FG32FB32F", RsTextureFormat::R32FG32FB32F ),
		new ReEnumConstant( "R32FG32FB32FA32F", RsTextureFormat::R32FG32FB32FA32F ),
		new ReEnumConstant( "R10G10B10A2", RsTextureFormat::R10G10B10A2 ),
		new ReEnumConstant( "R11G11B10F", RsTextureFormat::R11G11B10F ),
		new ReEnumConstant( "BC1", RsTextureFormat::BC1 ),
		new ReEnumConstant( "BC2", RsTextureFormat::BC2 ),
		new ReEnumConstant( "BC3", RsTextureFormat::BC3 ),
		new ReEnumConstant( "BC4", RsTextureFormat::BC4 ),
		new ReEnumConstant( "BC5", RsTextureFormat::BC5 ),
		new ReEnumConstant( "BC6H", RsTextureFormat::BC6H ),
		new ReEnumConstant( "BC7", RsTextureFormat::BC7 ),
		new ReEnumConstant( "ETC1", RsTextureFormat::ETC1 ),
		new ReEnumConstant( "D16", RsTextureFormat::D16 ),
		new ReEnumConstant( "D24", RsTextureFormat::D24 ),
		new ReEnumConstant( "D32", RsTextureFormat::D32 ),
		new ReEnumConstant( "D24S8", RsTextureFormat::D24S8 ),
		new ReEnumConstant( "INVALID", RsTextureFormat::INVALID ),

		// Legacy.
		new ReEnumConstant( "DXT1", RsTextureFormat::BC1 ),
		new ReEnumConstant( "DXT3", RsTextureFormat::BC2 ),
		new ReEnumConstant( "DXT5", RsTextureFormat::BC3 ),
	};
	ReRegisterEnum< RsTextureFormat >( RsTextureFormatEnumConstants );

	ReEnumConstant* RsShaderTypeEnumConstants[] = 
	{
		new ReEnumConstant( "VERTEX", RsShaderType::VERTEX ),
		new ReEnumConstant( "HULL", RsShaderType::HULL ),
		new ReEnumConstant( "DOMAIN", RsShaderType::DOMAIN ),
		new ReEnumConstant( "GEOMETRY", RsShaderType::GEOMETRY ),
		new ReEnumConstant( "PIXEL", RsShaderType::PIXEL ),
		new ReEnumConstant( "COMPUTE", RsShaderType::COMPUTE ),
	};
	ReRegisterEnum< RsShaderType >( RsShaderTypeEnumConstants );

	ReEnumConstant* RsShaderBackendTypeEnumConstants[] = 
	{
		new ReEnumConstant( "GLSL", RsShaderBackendType::GLSL ),
		new ReEnumConstant( "GLSL_ES", RsShaderBackendType::GLSL_ES ),
		new ReEnumConstant( "D3D11", RsShaderBackendType::D3D11 ),
		new ReEnumConstant( "D3D12", RsShaderBackendType::D3D12 ),
		new ReEnumConstant( "SPIRV", RsShaderBackendType::SPIRV ),
	};
	ReRegisterEnum< RsShaderBackendType >( RsShaderBackendTypeEnumConstants );

	ReEnumConstant* RsShaderCodeTypeEnumConstants[] = 
	{
		new ReEnumConstant( "GLSL_140", RsShaderCodeType::GLSL_140 ),
		new ReEnumConstant( "GLSL_150", RsShaderCodeType::GLSL_150 ),
		new ReEnumConstant( "GLSL_330", RsShaderCodeType::GLSL_330 ),
		new ReEnumConstant( "GLSL_400", RsShaderCodeType::GLSL_400 ),
		new ReEnumConstant( "GLSL_410", RsShaderCodeType::GLSL_410 ),
		new ReEnumConstant( "GLSL_420", RsShaderCodeType::GLSL_420 ),
		new ReEnumConstant( "GLSL_430", RsShaderCodeType::GLSL_430 ),
		new ReEnumConstant( "GLSL_440", RsShaderCodeType::GLSL_440 ),
		new ReEnumConstant( "GLSL_450", RsShaderCodeType::GLSL_450 ),
		new ReEnumConstant( "GLSL_ES_100", RsShaderCodeType::GLSL_ES_100 ),
		new ReEnumConstant( "GLSL_ES_300", RsShaderCodeType::GLSL_ES_300 ),
		new ReEnumConstant( "GLSL_ES_310", RsShaderCodeType::GLSL_ES_310 ),
		new ReEnumConstant( "D3D11_4_0_LEVEL_9_1", RsShaderCodeType::D3D11_4_0_LEVEL_9_1 ),
		new ReEnumConstant( "D3D11_4_0_LEVEL_9_2", RsShaderCodeType::D3D11_4_0_LEVEL_9_2 ),
		new ReEnumConstant( "D3D11_4_0_LEVEL_9_3", RsShaderCodeType::D3D11_4_0_LEVEL_9_3 ),
		new ReEnumConstant( "D3D11_4_0", RsShaderCodeType::D3D11_4_0 ),
		new ReEnumConstant( "D3D11_4_1", RsShaderCodeType::D3D11_4_1 ),
		new ReEnumConstant( "D3D11_5_0", RsShaderCodeType::D3D11_5_0 ),
		new ReEnumConstant( "D3D11_5_1", RsShaderCodeType::D3D11_5_1 ),
		new ReEnumConstant( "SPIRV", RsShaderCodeType::SPIRV ),
	};
	ReRegisterEnum< RsShaderCodeType >( RsShaderCodeTypeEnumConstants );

	ReEnumConstant* RsTextureTypeEnumConstants[] = 
	{
		new ReEnumConstant( "UNKNOWN", RsTextureType::UNKNOWN ),
		new ReEnumConstant( "TEX1D", RsTextureType::TEX1D ),
		new ReEnumConstant( "TEX2D", RsTextureType::TEX2D ),
		new ReEnumConstant( "TEX3D", RsTextureType::TEX3D ),
		new ReEnumConstant( "TEXCUBE", RsTextureType::TEXCUBE ),

		// Additional aliases.
		new ReEnumConstant( "1D", RsTextureType::TEX1D ),
		new ReEnumConstant( "2D", RsTextureType::TEX2D ),
		new ReEnumConstant( "3D", RsTextureType::TEX3D ),
		new ReEnumConstant( "CUBE", RsTextureType::TEXCUBE ),
	};
	ReRegisterEnum< RsTextureType >( RsTextureTypeEnumConstants );

	ReEnumConstant* RsRenderSortPassTypeEnumConstants[] = 
	{
		new ReEnumConstant( "SHADOW", RsRenderSortPassType::SHADOW ),
		new ReEnumConstant( "DEPTH", RsRenderSortPassType::DEPTH ),
		new ReEnumConstant( "OPAQUE", RsRenderSortPassType::OPAQUE),
		new ReEnumConstant( "TRANSPARENT", RsRenderSortPassType::TRANSPARENT ),
		new ReEnumConstant( "OVERLAY", RsRenderSortPassType::OVERLAY ),
	};
	ReRegisterEnum< RsRenderSortPassType >( RsRenderSortPassTypeEnumConstants );

	ReEnumConstant* RsRenderSortPassFlagsEnumConstants[] = 
	{
		new ReEnumConstant( "NONE", RsRenderSortPassFlags::NONE ),
		new ReEnumConstant( "SHADOW", RsRenderSortPassFlags::SHADOW ),
		new ReEnumConstant( "DEPTH", RsRenderSortPassFlags::DEPTH ),
		new ReEnumConstant( "OPAQUE", RsRenderSortPassFlags::OPAQUE ),
		new ReEnumConstant( "TRANSPARENT", RsRenderSortPassFlags::TRANSPARENT ),
		new ReEnumConstant( "OVERLAY", RsRenderSortPassFlags::OVERLAY ),
		new ReEnumConstant( "ALL", RsRenderSortPassFlags::ALL ),
	};
	ReRegisterEnum< RsRenderSortPassFlags >( RsRenderSortPassFlagsEnumConstants );
}
