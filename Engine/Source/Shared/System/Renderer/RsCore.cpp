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

BcU32 RsCore::JOB_QUEUE_ID = BcErrorCode;

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
	ReRegisterEnum< RsShaderType >( RsBlendingModeEnumConstants );

	ReEnumConstant* RsFillModeEnumConstants[] = 
	{
		new ReEnumConstant( "SOLID", (BcU32)RsFillMode::SOLID ),
		new ReEnumConstant( "WIREFRAME", (BcU32)RsFillMode::WIREFRAME ),
		new ReEnumConstant( "INVALID", (BcU32)RsFillMode::INVALID ),
	};
	ReRegisterEnum< RsShaderType >( RsFillModeEnumConstants );

	ReEnumConstant* RsBlendTypeEnumConstants[] = 
	{
		new ReEnumConstant( "ZERO", (BcU32)RsBlendType::ZERO ),
		new ReEnumConstant( "ONE", (BcU32)RsBlendType::ONE ),
		new ReEnumConstant( "SRC_COLOUR", (BcU32)RsBlendType::SRC_COLOUR ),
		new ReEnumConstant( "INV_SRC_COLOUR", (BcU32)RsBlendType::INV_SRC_COLOUR ),
		new ReEnumConstant( "SRC_ALPHA", (BcU32)RsBlendType::SRC_ALPHA ),
		new ReEnumConstant( "INV_SRC_ALPHA", (BcU32)RsBlendType::INV_SRC_ALPHA ),
		new ReEnumConstant( "DEST_COLOUR", (BcU32)RsBlendType::DEST_COLOUR ),
		new ReEnumConstant( "INV_DEST_COLOUR", (BcU32)RsBlendType::INV_DEST_COLOUR ),
		new ReEnumConstant( "DEST_ALPHA", (BcU32)RsBlendType::DEST_ALPHA ),
		new ReEnumConstant( "INV_DEST_ALPHA", (BcU32)RsBlendType::INV_DEST_ALPHA ),
		new ReEnumConstant( "INVALID", (BcU32)RsBlendType::INVALID ),
	};
	ReRegisterEnum< RsShaderType >( RsBlendTypeEnumConstants );

	ReEnumConstant* RsBlendOpEnumConstants[] = 
	{
		new ReEnumConstant( "ADD", (BcU32)RsBlendOp::ADD ),
		new ReEnumConstant( "SUBTRACT", (BcU32)RsBlendOp::SUBTRACT ),
		new ReEnumConstant( "REV_SUBTRACT", (BcU32)RsBlendOp::REV_SUBTRACT ),
		new ReEnumConstant( "MINIMUM", (BcU32)RsBlendOp::MINIMUM ),
		new ReEnumConstant( "MAXIMUM", (BcU32)RsBlendOp::MAXIMUM ),
		new ReEnumConstant( "INVALID", (BcU32)RsBlendOp::INVALID ),
	};
	ReRegisterEnum< RsShaderType >( RsBlendOpEnumConstants );

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
	ReRegisterEnum< RsShaderType >( RsCompareModeEnumConstants );

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
	ReRegisterEnum< RsShaderType >( RsStencilOpEnumConstants );

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
	ReRegisterEnum< RsShaderType >( RsTextureFilteringModeEnumConstants );

	ReEnumConstant* RsTextureSamplingModeEnumConstants[] = 
	{
		new ReEnumConstant( "WRAP", (BcU32)RsTextureSamplingMode::WRAP ),
		new ReEnumConstant( "MIRROR", (BcU32)RsTextureSamplingMode::MIRROR ),
		new ReEnumConstant( "CLAMP", (BcU32)RsTextureSamplingMode::CLAMP ),
		new ReEnumConstant( "DECAL", (BcU32)RsTextureSamplingMode::DECAL ),
		new ReEnumConstant( "INVALID", (BcU32)RsTextureSamplingMode::INVALID ),
	};
	ReRegisterEnum< RsShaderType >( RsTextureSamplingModeEnumConstants );

	ReEnumConstant* RsRenderStateTypeEnumConstants[] = 
	{
		new ReEnumConstant( "DEPTH_WRITE_ENABLE", (BcU32)RsRenderStateType::DEPTH_WRITE_ENABLE ),
		new ReEnumConstant( "DEPTH_TEST_ENABLE", (BcU32)RsRenderStateType::DEPTH_TEST_ENABLE ),
		new ReEnumConstant( "DEPTH_TEST_COMPARE", (BcU32)RsRenderStateType::DEPTH_TEST_COMPARE ),
		new ReEnumConstant( "STENCIL_WRITE_MASK", (BcU32)RsRenderStateType::STENCIL_WRITE_MASK ),
		new ReEnumConstant( "STENCIL_TEST_ENABLE", (BcU32)RsRenderStateType::STENCIL_TEST_ENABLE ),
		new ReEnumConstant( "STENCIL_TEST_FUNC_COMPARE", (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_COMPARE ),
		new ReEnumConstant( "STENCIL_TEST_FUNC_REF", (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_REF ),
		new ReEnumConstant( "STENCIL_TEST_FUNC_MASK", (BcU32)RsRenderStateType::STENCIL_TEST_FUNC_MASK ),
		new ReEnumConstant( "STENCIL_TEST_OP_SFAIL", (BcU32)RsRenderStateType::STENCIL_TEST_OP_SFAIL ),
		new ReEnumConstant( "STENCIL_TEST_OP_DPFAIL", (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPFAIL ),
		new ReEnumConstant( "STENCIL_TEST_OP_DPPASS", (BcU32)RsRenderStateType::STENCIL_TEST_OP_DPPASS ),
		new ReEnumConstant( "COLOR_WRITE_MASK_0", (BcU32)RsRenderStateType::COLOR_WRITE_MASK_0 ),
		new ReEnumConstant( "COLOR_WRITE_MASK_1", (BcU32)RsRenderStateType::COLOR_WRITE_MASK_1 ),
		new ReEnumConstant( "COLOR_WRITE_MASK_2", (BcU32)RsRenderStateType::COLOR_WRITE_MASK_2 ),
		new ReEnumConstant( "COLOR_WRITE_MASK_3", (BcU32)RsRenderStateType::COLOR_WRITE_MASK_3 ),
		new ReEnumConstant( "BLEND_MODE", (BcU32)RsRenderStateType::BLEND_MODE ),
		new ReEnumConstant( "FILL_MODE", (BcU32)RsRenderStateType::FILL_MODE ),
	};
	ReRegisterEnum< RsShaderType >( RsRenderStateTypeEnumConstants );

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
		new ReEnumConstant( "D16", (BcU32)RsTextureFormat::D16 ),
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
		new ReEnumConstant( "D3D11", (BcU32)RsShaderBackendType::D3D11 ),
		new ReEnumConstant( "D3D12", (BcU32)RsShaderBackendType::D3D12 ),
		new ReEnumConstant( "MANTLE", (BcU32)RsShaderBackendType::MANTLE ),
	};
	ReRegisterEnum< RsShaderBackendType >( RsShaderBackendTypeEnumConstants );

	ReEnumConstant* RsShaderCodeTypeEnumConstants[] = 
	{
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
