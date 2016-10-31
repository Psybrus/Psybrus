#include "System/Renderer/GL/RsUtilsGL.h"

////////////////////////////////////////////////////////////////////////////////
// OSX hack
#ifdef DOMAIN
#undef DOMAIN
#endif

////////////////////////////////////////////////////////////////////////////////
// CompareMode
GLenum RsUtilsGL::GetCompareMode( RsCompareMode V )
{
	switch( V )
	{
	case RsCompareMode::NEVER:
		return GL_NEVER;
	case RsCompareMode::LESS:
		return GL_LESS;
	case RsCompareMode::EQUAL:
		return GL_EQUAL;
	case RsCompareMode::LESSEQUAL:
		return GL_LEQUAL;
	case RsCompareMode::GREATER:
		return GL_GREATER;
	case RsCompareMode::NOTEQUAL:
		return GL_NOTEQUAL;
	case RsCompareMode::GREATEREQUAL:
		return GL_GEQUAL;
	case RsCompareMode::ALWAYS:
		return GL_ALWAYS;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetBlendOp
GLenum RsUtilsGL::GetBlendOp( RsBlendOp V )
{
	switch( V )
	{
	case RsBlendOp::ADD:
		return GL_FUNC_ADD;
	case RsBlendOp::SUBTRACT:
		return GL_FUNC_SUBTRACT;
	case RsBlendOp::REV_SUBTRACT:
		return GL_FUNC_REVERSE_SUBTRACT;
	case RsBlendOp::MINIMUM:
		return GL_MIN;
	case RsBlendOp::MAXIMUM:
		return GL_MAX;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetBlendType
GLenum RsUtilsGL::GetBlendType( RsBlendType V )
{
	switch( V )
	{
	case RsBlendType::ZERO:
		return GL_ZERO;
	case RsBlendType::ONE:
		return GL_ONE;
	case RsBlendType::SRC_COLOUR:
		return GL_SRC_COLOR;
	case RsBlendType::INV_SRC_COLOUR:
		return GL_ONE_MINUS_SRC_COLOR;
	case RsBlendType::SRC_ALPHA:
		return GL_SRC_ALPHA;
	case RsBlendType::INV_SRC_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case RsBlendType::DEST_COLOUR:
		return GL_DST_COLOR;
	case RsBlendType::INV_DEST_COLOUR:
		return GL_ONE_MINUS_DST_COLOR;
	case RsBlendType::DEST_ALPHA:
		return GL_DST_ALPHA;
	case RsBlendType::INV_DEST_ALPHA:
		return GL_ONE_MINUS_DST_ALPHA;
	default:
		BcBreakpoint;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// GetStencilOp
GLenum RsUtilsGL::GetStencilOp( RsStencilOp V )
{
	switch( V )
	{
	case RsStencilOp::KEEP:
		return GL_KEEP;
	case RsStencilOp::ZERO:
		return GL_ZERO;
	case RsStencilOp::REPLACE:
		return GL_REPLACE;
	case RsStencilOp::INCR:
		return GL_INCR;
	case RsStencilOp::INCR_WRAP:
		return GL_INCR_WRAP;
	case RsStencilOp::DECR:
		return GL_DECR;
	case RsStencilOp::DECR_WRAP:
		return GL_DECR_WRAP;
	case RsStencilOp::INVERT:
		return GL_INVERT;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetTextureFiltering
GLenum RsUtilsGL::GetTextureFiltering( RsTextureFilteringMode V )
{
	switch( V )
	{
	case RsTextureFilteringMode::NEAREST:
		return GL_NEAREST;
	case RsTextureFilteringMode::LINEAR:
		return GL_LINEAR;
	case RsTextureFilteringMode::NEAREST_MIPMAP_NEAREST:
		return GL_NEAREST_MIPMAP_NEAREST;
	case RsTextureFilteringMode::LINEAR_MIPMAP_NEAREST:
		return GL_LINEAR_MIPMAP_NEAREST;
	case RsTextureFilteringMode::NEAREST_MIPMAP_LINEAR:
		return GL_NEAREST_MIPMAP_LINEAR;
	case RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR:
		return GL_LINEAR_MIPMAP_LINEAR;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetTextureSampling
GLenum RsUtilsGL::GetTextureSampling( RsTextureSamplingMode V )
{
	switch( V )
	{
	case RsTextureSamplingMode::WRAP:
		return GL_REPEAT;
	case RsTextureSamplingMode::MIRROR:
		return GL_MIRRORED_REPEAT;
	case RsTextureSamplingMode::CLAMP:
		return GL_CLAMP_TO_EDGE;
#if !defined( RENDER_USE_GLES )
	case RsTextureSamplingMode::DECAL:
		return GL_DECAL;
#endif
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetTextureType
GLenum RsUtilsGL::GetTextureType( RsTextureType V )
{
	switch( V )
	{
#if !defined( RENDER_USE_GLES )
	case RsTextureType::TEX1D:
		return GL_TEXTURE_1D;
	case RsTextureType::TEX2D:
		return GL_TEXTURE_2D;
	case RsTextureType::TEX3D:
		return GL_TEXTURE_3D;
	case RsTextureType::TEXCUBE:
		return GL_TEXTURE_CUBE_MAP;
#else
	case RsTextureType::TEX2D:
		return GL_TEXTURE_2D;
	case RsTextureType::TEXCUBE:
		return GL_TEXTURE_CUBE_MAP;
#endif
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetTextureFace
GLenum RsUtilsGL::GetTextureFace( RsTextureFace V )
{
	switch( V )
	{
	case RsTextureFace::POSITIVE_X:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	case RsTextureFace::NEGATIVE_X:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
	case RsTextureFace::POSITIVE_Y:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
	case RsTextureFace::NEGATIVE_Y:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
	case RsTextureFace::POSITIVE_Z:
		return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
	case RsTextureFace::NEGATIVE_Z:
		return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetVertexDataType
GLenum RsUtilsGL::GetVertexDataType( RsVertexDataType V )
{
	switch( V )
	{
	case RsVertexDataType::FLOAT32:
		return GL_FLOAT;
	case RsVertexDataType::FLOAT16:
		return GL_HALF_FLOAT;
	case RsVertexDataType::FIXED:
		return GL_FIXED;
	case RsVertexDataType::BYTE:
	case RsVertexDataType::BYTE_NORM:
		return GL_BYTE;
	case RsVertexDataType::UBYTE:
	case RsVertexDataType::UBYTE_NORM:
		return GL_UNSIGNED_BYTE;
	case RsVertexDataType::SHORT:
	case RsVertexDataType::SHORT_NORM:
		return GL_SHORT;
	case RsVertexDataType::USHORT:
	case RsVertexDataType::USHORT_NORM:
		return GL_UNSIGNED_SHORT;
	case RsVertexDataType::INT:
	case RsVertexDataType::INT_NORM:
		return GL_INT;
	case RsVertexDataType::UINT:
	case RsVertexDataType::UINT_NORM:
		return GL_UNSIGNED_INT;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetVertexDataNormalised
GLboolean RsUtilsGL::GetVertexDataNormalised( RsVertexDataType V )
{
	switch( V )
	{
	case RsVertexDataType::FLOAT32:
	case RsVertexDataType::FLOAT16:
	case RsVertexDataType::FIXED:
	case RsVertexDataType::BYTE:
	case RsVertexDataType::UBYTE:
	case RsVertexDataType::SHORT:
	case RsVertexDataType::USHORT:
	case RsVertexDataType::INT:
		return GL_FALSE;
	case RsVertexDataType::UINT:
	case RsVertexDataType::BYTE_NORM:
	case RsVertexDataType::UBYTE_NORM:
	case RsVertexDataType::SHORT_NORM:
	case RsVertexDataType::USHORT_NORM:
	case RsVertexDataType::INT_NORM:
	case RsVertexDataType::UINT_NORM:
		return GL_TRUE;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetVertexDataSize
GLint RsUtilsGL::GetVertexDataSize( RsVertexDataType V )
{
	switch( V )
	{
	case RsVertexDataType::FLOAT32:
		return 4;
	case RsVertexDataType::FLOAT16:
		return 2;
	case RsVertexDataType::FIXED:
		return 4;
	case RsVertexDataType::BYTE:
	case RsVertexDataType::BYTE_NORM:
	case RsVertexDataType::UBYTE:
	case RsVertexDataType::UBYTE_NORM:
		return 1;
	case RsVertexDataType::SHORT:
	case RsVertexDataType::SHORT_NORM:
	case RsVertexDataType::USHORT:
	case RsVertexDataType::USHORT_NORM:
		return 2;
	case RsVertexDataType::INT:
	case RsVertexDataType::INT_NORM:
	case RsVertexDataType::UINT:
	case RsVertexDataType::UINT_NORM:
		return 4;
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetTopologyType
GLenum RsUtilsGL::GetTopologyType( RsTopologyType V )
{
	switch( V )
	{
	case RsTopologyType::POINTS:
		return GL_POINTS;
	case RsTopologyType::LINE_LIST:
		return GL_LINES;
	case RsTopologyType::LINE_STRIP:
		return GL_LINE_STRIP;
#if !defined( RENDER_USE_GLES )
	case RsTopologyType::LINE_LIST_ADJACENCY:
		return GL_LINES_ADJACENCY;
	case RsTopologyType::LINE_STRIP_ADJACENCY:
		return GL_LINE_STRIP_ADJACENCY;
#endif
	case RsTopologyType::TRIANGLE_LIST:
		return GL_TRIANGLES;
	case RsTopologyType::TRIANGLE_STRIP:
		return GL_TRIANGLE_STRIP;
#if !defined( RENDER_USE_GLES )
	case RsTopologyType::TRIANGLE_LIST_ADJACENCY:
		return GL_TRIANGLES_ADJACENCY;
	case RsTopologyType::TRIANGLE_STRIP_ADJACENCY:
		return GL_TRIANGLE_STRIP_ADJACENCY;
#endif
	case RsTopologyType::TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;
#if !defined( RENDER_USE_GLES )
	case RsTopologyType::PATCH_LIST:
		return GL_PATCHES;
#endif
	default:
		BcBreakpoint;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetBufferType
GLenum RsUtilsGL::GetBufferType( RsBindFlags V )
{
	if( ( V & RsBindFlags::VERTEX_BUFFER ) != RsBindFlags::NONE )
	{
		return GL_ARRAY_BUFFER;
	}
	else if( ( V & RsBindFlags::INDEX_BUFFER ) != RsBindFlags::NONE )
	{
		return GL_ELEMENT_ARRAY_BUFFER;
	}
	else if( ( V & RsBindFlags::UNIFORM_BUFFER ) != RsBindFlags::NONE )
	{
		return GL_UNIFORM_BUFFER;
	}
	else if( ( V & RsBindFlags::STREAM_OUTPUT ) != RsBindFlags::NONE )
	{
		return GL_TRANSFORM_FEEDBACK_BUFFER;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
// GetResourceFormat
RsUtilsGL::ResourceFormat RsUtilsGL::GetResourceFormat( RsResourceFormat V )
{
	auto Unsupported = ResourceFormat( BcFalse, BcFalse, GL_ZERO, GL_ZERO, GL_ZERO );
	
	switch( V )
	{
	case RsResourceFormat::R8_UNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_R8, GL_RED, GL_UNSIGNED_BYTE );
	case RsResourceFormat::R8_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_R8UI, GL_RED, GL_UNSIGNED_BYTE );
	case RsResourceFormat::R8_SNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_R8_SNORM, GL_RED, GL_BYTE );
	case RsResourceFormat::R8_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_R8I, GL_RED, GL_BYTE );

	case RsResourceFormat::R8G8_UNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RG8, GL_RG, GL_UNSIGNED_BYTE );
	case RsResourceFormat::R8G8_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG8UI, GL_RG, GL_UNSIGNED_BYTE );
	case RsResourceFormat::R8G8_SNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RG8_SNORM, GL_RG, GL_BYTE );
	case RsResourceFormat::R8G8_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG8I, GL_RG, GL_BYTE );

	case RsResourceFormat::R8G8B8A8_UNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE );
	case RsResourceFormat::R8G8B8A8_UNORM_SRGB:
		return ResourceFormat( BcFalse, BcFalse, GL_SRGB8_ALPHA8, GL_RGBA, GL_UNSIGNED_BYTE );
	case RsResourceFormat::R8G8B8A8_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA8UI, GL_RGBA, GL_UNSIGNED_BYTE );
	case RsResourceFormat::R8G8B8A8_SNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA8_SNORM, GL_RGBA, GL_BYTE );
	case RsResourceFormat::R8G8B8A8_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA8I, GL_RGBA, GL_BYTE );

	case RsResourceFormat::R16_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_R16F, GL_RED, GL_HALF_FLOAT ); // GL_HALF_FLOAT_OES
	case RsResourceFormat::R16_UNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_R16, GL_RED, GL_UNSIGNED_SHORT );
	case RsResourceFormat::R16_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_R16UI, GL_RED, GL_UNSIGNED_SHORT );
	case RsResourceFormat::R16_SNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_R16_SNORM, GL_RED, GL_SHORT );
	case RsResourceFormat::R16_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_R16I, GL_RED, GL_SHORT );

	case RsResourceFormat::R16G16_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG16F, GL_RG, GL_HALF_FLOAT ); // GL_HALF_FLOAT_OES
	case RsResourceFormat::R16G16_UNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RG16, GL_RG, GL_UNSIGNED_SHORT );
	case RsResourceFormat::R16G16_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG16UI, GL_RG, GL_UNSIGNED_SHORT );
	case RsResourceFormat::R16G16_SNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RG16_SNORM, GL_RG, GL_SHORT );
	case RsResourceFormat::R16G16_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG16I, GL_RG, GL_SHORT );

	case RsResourceFormat::R16G16B16A16_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT ); // GL_HALF_FLOAT_OES
	case RsResourceFormat::R16G16B16A16_UNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA16, GL_RGBA, GL_UNSIGNED_SHORT );
	case RsResourceFormat::R16G16B16A16_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA16UI, GL_RGBA, GL_UNSIGNED_SHORT );
	case RsResourceFormat::R16G16B16A16_SNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA16_SNORM, GL_RGBA, GL_SHORT );
	case RsResourceFormat::R16G16B16A16_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA16I, GL_RGBA, GL_SHORT );

	case RsResourceFormat::R32_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_R32F, GL_RED, GL_FLOAT );
	case RsResourceFormat::R32_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_R32UI, GL_RED, GL_UNSIGNED_INT );
	case RsResourceFormat::R32_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_R32I, GL_RED, GL_INT );

	case RsResourceFormat::R32G32_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG32F, GL_RED, GL_FLOAT );
	case RsResourceFormat::R32G32_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG32UI, GL_RED, GL_UNSIGNED_INT );
	case RsResourceFormat::R32G32_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RG32I, GL_RED, GL_INT );

	case RsResourceFormat::R32G32B32_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGB32F, GL_RED, GL_FLOAT );
	case RsResourceFormat::R32G32B32_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGB32UI, GL_RED, GL_UNSIGNED_INT );
	case RsResourceFormat::R32G32B32_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGB32I, GL_RED, GL_INT );

	case RsResourceFormat::R32G32B32A32_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA32F, GL_RED, GL_FLOAT );
	case RsResourceFormat::R32G32B32A32_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA32UI, GL_RED, GL_UNSIGNED_INT );
	case RsResourceFormat::R32G32B32A32_SINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGBA32I, GL_RED, GL_INT );

	case RsResourceFormat::R10G10B10A2_UNORM:
		return ResourceFormat( BcFalse, BcFalse, GL_RGB10_A2, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2 );
	case RsResourceFormat::R10G10B10A2_UINT:
		return ResourceFormat( BcFalse, BcFalse, GL_RGB10_A2UI, GL_RGBA, GL_UNSIGNED_INT_10_10_10_2 );
	
	case RsResourceFormat::R11G11B10_FLOAT:
		return ResourceFormat( BcFalse, BcFalse, GL_R11F_G11F_B10F, GL_RGB, GL_FLOAT );

	case RsResourceFormat::BC1_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, 0, 0 );
	case RsResourceFormat::BC1_UNORM_SRGB:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, 0, 0 );
	case RsResourceFormat::BC2_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, 0, 0 );
	case RsResourceFormat::BC2_UNORM_SRGB:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, 0, 0 );
	case RsResourceFormat::BC3_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, 0, 0 );
	case RsResourceFormat::BC3_UNORM_SRGB:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, 0, 0 );
	case RsResourceFormat::BC4_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RED_RGTC1, GL_RED, 0 );
	case RsResourceFormat::BC4_SNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_SIGNED_RED_RGTC1, GL_RED, 0 );
	case RsResourceFormat::BC5_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RG_RGTC2, GL_RG, 0 );
	case RsResourceFormat::BC5_SNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_SIGNED_RG_RGTC2, GL_RG, 0 );
	case RsResourceFormat::BC6H_UF16:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, 0, 0 );
	case RsResourceFormat::BC6H_SF16:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, 0, 0 );
	case RsResourceFormat::BC7_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGBA_BPTC_UNORM, 0, 0 );
	case RsResourceFormat::BC7_UNORM_SRGB:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, 0, 0 );
	case RsResourceFormat::ETC1_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_ETC1_RGB8_OES, 0, 0 );
	case RsResourceFormat::ETC2_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGB8_ETC2, 0, 0 );
	case RsResourceFormat::ETC2A_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGBA8_ETC2_EAC, 0, 0 );
	case RsResourceFormat::ETC2A1_UNORM:
		return ResourceFormat( BcTrue, BcFalse, GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2, 0, 0 );
	case RsResourceFormat::D16_UNORM:
		return ResourceFormat( BcFalse, BcTrue, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT );
	case RsResourceFormat::D24_UNORM_S8_UINT:
		return ResourceFormat( BcFalse, BcTrue, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8 ); // GL_UNSIGNED_INT_24_8_OES
	case RsResourceFormat::D32_FLOAT:
		return ResourceFormat( BcFalse, BcTrue, GL_DEPTH_COMPONENT32F, GL_DEPTH_COMPONENT, GL_FLOAT );

	default:
		BcBreakpoint;
	}

	return Unsupported;
}

////////////////////////////////////////////////////////////////////////////////
// GetShaderType
GLenum RsUtilsGL::GetShaderType( RsShaderType V )
{
	switch( V )
	{
	case RsShaderType::VERTEX:
		return GL_VERTEX_SHADER;
	case RsShaderType::PIXEL:
		return GL_FRAGMENT_SHADER;
#if !defined( RENDER_USE_GLES )
	case RsShaderType::HULL:
		return GL_TESS_CONTROL_SHADER;
	case RsShaderType::DOMAIN:
		return GL_TESS_EVALUATION_SHADER;
	case RsShaderType::GEOMETRY:
		return GL_GEOMETRY_SHADER;
	case RsShaderType::COMPUTE:
		return GL_COMPUTE_SHADER;
#endif
	default:
		BcBreakpoint;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// GetShaderType
GLenum RsUtilsGL::GetQueryType( RsQueryType Query )
{
	switch( Query )
	{
#if !defined( RENDER_USE_GLES )
 	case RsQueryType::OCCLUSION:
		return GL_SAMPLES_PASSED;
	case RsQueryType::OCCLUSION_BINARY:
		return GL_ANY_SAMPLES_PASSED;
	case RsQueryType::TIMESTAMP:
		return GL_TIMESTAMP;
#endif
	default:
		BcBreakpoint;
	}
	return 0;
}
