#include "System/Renderer/D3D12/RsUtilsD3D12.h"


//////////////////////////////////////////////////////////////////////////
// Texture formats
namespace 
{
	const RsTextureFormatD3D12 GTextureFormats[] =
	{
		// Unknown.
		{ RsTextureFormat::UNKNOWN,
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN },

		// Colour.
		{ RsTextureFormat::R8,					
			DXGI_FORMAT_R8_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R8_UNORM,				DXGI_FORMAT_R8_UINT },
		{ RsTextureFormat::R8G8,				
			DXGI_FORMAT_R8G8_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R8G8_UNORM,				DXGI_FORMAT_R8G8_UINT },
		{ RsTextureFormat::R8G8B8,				
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN },
		{ RsTextureFormat::R8G8B8A8,			
			DXGI_FORMAT_R8G8B8A8_UNORM,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R8G8B8A8_UNORM,			DXGI_FORMAT_R8G8B8A8_UINT },
		{ RsTextureFormat::R16F,				
			DXGI_FORMAT_R16_FLOAT,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R16_FLOAT,				DXGI_FORMAT_R16_FLOAT },
		{ RsTextureFormat::R16FG16F,			
			DXGI_FORMAT_R16G16_FLOAT,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R16G16_FLOAT,			DXGI_FORMAT_R16G16_FLOAT },
		{ RsTextureFormat::R16FG16FB16F,	
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN },
		{ RsTextureFormat::R16FG16FB16FA16F,	
			DXGI_FORMAT_R16G16B16A16_FLOAT,	DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R16G16B16A16_FLOAT,		DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ RsTextureFormat::R32F,				
			DXGI_FORMAT_R32_FLOAT,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R32_FLOAT,				DXGI_FORMAT_R32_FLOAT },
		{ RsTextureFormat::R32FG32F,			
			DXGI_FORMAT_R32G32_FLOAT,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R32G32_FLOAT,			DXGI_FORMAT_R32G32_FLOAT },
		{ RsTextureFormat::R32FG32FB32F,		
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN},
		{ RsTextureFormat::R32FG32FB32FA32F,	
			DXGI_FORMAT_R32G32B32A32_FLOAT,	DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R32G32B32A32_FLOAT,		DXGI_FORMAT_R32G32B32A32_FLOAT },
		{ RsTextureFormat::R10G10B10A2,	
			DXGI_FORMAT_R10G10B10A2_UNORM,	DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R10G10B10A2_UNORM,		DXGI_FORMAT_R10G10B10A2_UNORM },
		{ RsTextureFormat::R11G11B10F,	
			DXGI_FORMAT_R11G11B10_FLOAT,	DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R11G11B10_FLOAT,		DXGI_FORMAT_R11G11B10_FLOAT },
		{ RsTextureFormat::BC1,				
			DXGI_FORMAT_BC1_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC1_UNORM,				DXGI_FORMAT_BC1_UNORM },
		{ RsTextureFormat::BC2,				
			DXGI_FORMAT_BC2_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC2_UNORM,				DXGI_FORMAT_BC2_UNORM },
		{ RsTextureFormat::BC3,				
			DXGI_FORMAT_BC3_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC3_UNORM,				DXGI_FORMAT_BC3_UNORM },
		{ RsTextureFormat::BC4,				
			DXGI_FORMAT_BC4_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC4_UNORM,				DXGI_FORMAT_BC4_UNORM },
		{ RsTextureFormat::BC5,				
			DXGI_FORMAT_BC5_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC5_UNORM,				DXGI_FORMAT_BC5_UNORM },
		{ RsTextureFormat::BC6H,				
			DXGI_FORMAT_BC6H_UF16,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC6H_UF16,				DXGI_FORMAT_BC6H_UF16 },
		{ RsTextureFormat::BC7,				
			DXGI_FORMAT_BC7_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC7_UNORM,				DXGI_FORMAT_BC7_UNORM },
		{ RsTextureFormat::ETC1,
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,				DXGI_FORMAT_UNKNOWN },

		// Depth.
		{ RsTextureFormat::D16,
			DXGI_FORMAT_R16_TYPELESS,		DXGI_FORMAT_D16_UNORM,			DXGI_FORMAT_R16_UNORM,				DXGI_FORMAT_R16_UNORM },
		{ RsTextureFormat::D24, 
			DXGI_FORMAT_R24G8_TYPELESS,		DXGI_FORMAT_D24_UNORM_S8_UINT,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
		{ RsTextureFormat::D32,
			DXGI_FORMAT_R32_TYPELESS,		DXGI_FORMAT_D32_FLOAT,			DXGI_FORMAT_D32_FLOAT,				DXGI_FORMAT_R32_FLOAT },
		{ RsTextureFormat::D24S8, 
			DXGI_FORMAT_R24G8_TYPELESS,		DXGI_FORMAT_D24_UNORM_S8_UINT,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
	};

	static const D3D12_PRIMITIVE_TOPOLOGY GTopology[] =
	{
		D3D_PRIMITIVE_TOPOLOGY_POINTLIST,					// RsTopologyType::POINTLIST = 0,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST,					// RsTopologyType::LINE_LIST,
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,					// RsTopologyType::LINE_STRIP,
		D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,				// RsTopologyType::LINE_LIST_ADJACENCY,
		D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,				// RsTopologyType::LINE_STRIP_ADJACENCY,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,				// RsTopologyType::TRIANGLE_LIST,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,				// RsTopologyType::TRIANGLE_STRIP,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,			// RsTopologyType::TRIANGLE_LIST_ADJACENCY,
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,			// RsTopologyType::TRIANGLE_STRIP_ADJACENCY,
		D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,					// RsTopologyType::TRIANGLE_FAN,
		D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,	// RsTopologyType::PATCHES,
	};

	static const D3D12_PRIMITIVE_TOPOLOGY_TYPE GTopologyType[] =
	{
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,				// RsTopologyType::POINTLIST = 0,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,					// RsTopologyType::LINE_LIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,					// RsTopologyType::LINE_STRIP,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,					// RsTopologyType::LINE_LIST_ADJACENCY,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,					// RsTopologyType::LINE_STRIP_ADJACENCY,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,				// RsTopologyType::TRIANGLE_LIST,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,				// RsTopologyType::TRIANGLE_STRIP,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,				// RsTopologyType::TRIANGLE_LIST_ADJACENCY,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,				// RsTopologyType::TRIANGLE_STRIP_ADJACENCY,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,				// RsTopologyType::TRIANGLE_FAN,
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH,				// RsTopologyType::PATCHES,
	};

	static const D3D12_FILL_MODE GFillMode[] =
	{
		D3D12_FILL_MODE_SOLID,
		D3D12_FILL_MODE_WIREFRAME,
	};

	static const D3D12_CULL_MODE GCullMode[] =
	{
		D3D12_CULL_MODE_NONE,
		D3D12_CULL_MODE_BACK,
		D3D12_CULL_MODE_FRONT,
	};

	static const D3D12_COMPARISON_FUNC GComparisonFunc[] =
	{
		D3D12_COMPARISON_FUNC_NEVER,			// RsCompareMode::NEVER,
		D3D12_COMPARISON_FUNC_LESS,				// RsCompareMode::LESS,
		D3D12_COMPARISON_FUNC_EQUAL,			// RsCompareMode::EQUAL,
		D3D12_COMPARISON_FUNC_LESS_EQUAL,		// RsCompareMode::LESSEQUAL,
		D3D12_COMPARISON_FUNC_GREATER,			// RsCompareMode::GREATER,
		D3D12_COMPARISON_FUNC_NOT_EQUAL,		// RsCompareMode::NOTEQUAL,
		D3D12_COMPARISON_FUNC_GREATER_EQUAL,	// RsCompareMode::GREATEREQUAL,
		D3D12_COMPARISON_FUNC_ALWAYS,			// RsCompareMode::ALWAYS,
	};

	static const D3D12_STENCIL_OP GStencilOp[] =
	{
		D3D12_STENCIL_OP_KEEP,			// RsStencilOp::KEEP,
		D3D12_STENCIL_OP_ZERO,			// RsStencilOp::ZERO,
		D3D12_STENCIL_OP_REPLACE,		// RsStencilOp::REPLACE,
		D3D12_STENCIL_OP_INCR_SAT,		// RsStencilOp::INCR,
		D3D12_STENCIL_OP_INCR,			// RsStencilOp::INCR_WRAP,
		D3D12_STENCIL_OP_DECR_SAT,		// RsStencilOp::DECR,
		D3D12_STENCIL_OP_DECR,			// RsStencilOp::DECR_WRAP,
		D3D12_STENCIL_OP_INVERT,		// RsStencilOp::INVERT,
	};

	static const D3D12_TEXTURE_ADDRESS_MODE GTextureAddressMode[] =
	{
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,		// RsTextureSamplingMode::WRAP
		D3D12_TEXTURE_ADDRESS_MODE_MIRROR,	// RsTextureSamplingMode::MIRROR
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,	// RsTextureSamplingMode::CLAMP
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,	// RsTextureSamplingMode::DECAL
	};

	static const D3D12_BLEND GBlend[] =
	{
		D3D12_BLEND_ZERO,
		D3D12_BLEND_ONE,
		D3D12_BLEND_SRC_COLOR,
		D3D12_BLEND_INV_SRC_COLOR,
		D3D12_BLEND_SRC_ALPHA,
		D3D12_BLEND_INV_SRC_ALPHA,
		D3D12_BLEND_DEST_COLOR,
		D3D12_BLEND_INV_DEST_COLOR,
		D3D12_BLEND_DEST_ALPHA,
		D3D12_BLEND_INV_DEST_ALPHA,
	};

	static const D3D12_BLEND_OP GBlendOp[] =
	{
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_OP_SUBTRACT,
		D3D12_BLEND_OP_REV_SUBTRACT,
		D3D12_BLEND_OP_MIN,
		D3D12_BLEND_OP_MAX,
	};

	static const LPCSTR GSemanticName[] =
	{
		"POSITION",					// RsVertexUsage::POSITION,
		"BLENDWEIGHTS",				// RsVertexUsage::BLENDWEIGHTS,
		"BLENDINDICES",				// RsVertexUsage::BLENDINDICES,
		"NORMAL",					// RsVertexUsage::NORMAL,
		"PSIZE",					// RsVertexUsage::PSIZE,
		"TEXCOORD",					// RsVertexUsage::TEXCOORD,
		"TANGENT",					// RsVertexUsage::TANGENT,
		"BINORMAL",					// RsVertexUsage::BINORMAL,
		"TESSFACTOR",				// RsVertexUsage::TESSFACTOR,
		"POSITIONT",				// RsVertexUsage::POSITIONT,
		"COLOR",					// RsVertexUsage::COLOUR,
		"FOG",						// RsVertexUsage::FOG,
		"DEPTH",					// RsVertexUsage::DEPTH,
		"SAMPLE",					// RsVertexUsage::SAMPLE,
	};
}

//////////////////////////////////////////////////////////////////////////
// GetTextureFormat
//static
const RsTextureFormatD3D12& RsUtilsD3D12::GetTextureFormat( RsTextureFormat TextureFormat )
{
	const auto& RetVal = GTextureFormats[ (size_t)TextureFormat ];
	BcAssert( RetVal.TextureFormat_ == TextureFormat );
	return RetVal;
};

//////////////////////////////////////////////////////////////////////////
// GetPrimitiveTopology
//static
const D3D12_PRIMITIVE_TOPOLOGY RsUtilsD3D12::GetPrimitiveTopology( RsTopologyType Topology )
{
	return GTopology[ (size_t)Topology ];
}

//////////////////////////////////////////////////////////////////////////
// GetPrimitiveTopologyType
//static
const D3D12_PRIMITIVE_TOPOLOGY_TYPE RsUtilsD3D12::GetPrimitiveTopologyType( RsTopologyType Topology )
{
	return GTopologyType[ (size_t)Topology ];
}

//////////////////////////////////////////////////////////////////////////
// GetResourceUsage
//static 
const D3D12_RESOURCE_STATES RsUtilsD3D12::GetResourceUsage( RsResourceBindFlags BindFlags )
{
	BcU32 AllowedUsage = 0;
	if( ( BindFlags & RsResourceBindFlags::VERTEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_GENERIC_READ |
			D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( BindFlags & RsResourceBindFlags::INDEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_GENERIC_READ |
			D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( BindFlags & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_GENERIC_READ |
			D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( BindFlags & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE |
			D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE |
			D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( BindFlags & RsResourceBindFlags::STREAM_OUTPUT ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_STREAM_OUT;
	}
	if( ( BindFlags & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_RENDER_TARGET;
	}
	if( ( BindFlags & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_DEPTH_READ | 
			D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
	if( ( BindFlags & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_UNORDERED_ACCESS |
			D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( BindFlags & RsResourceBindFlags::PRESENT ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_STATE_PRESENT;
	}

	return static_cast< D3D12_RESOURCE_STATES >( AllowedUsage );
}

//////////////////////////////////////////////////////////////////////////
// GetFillMode
//static
const D3D12_FILL_MODE RsUtilsD3D12::GetFillMode( RsFillMode FillMode )
{
	return GFillMode[ (size_t)FillMode ];
}

//////////////////////////////////////////////////////////////////////////
// GetCullMode
//static
const D3D12_CULL_MODE RsUtilsD3D12::GetCullMode( RsCullMode CullMode )
{
	return GCullMode[ (size_t)CullMode ];
}

//////////////////////////////////////////////////////////////////////////
// GetComparisonFunc
//static
const D3D12_COMPARISON_FUNC RsUtilsD3D12::GetComparisonFunc( RsCompareMode CompareMode )
{
	return GComparisonFunc[ (size_t)CompareMode ];
}

//////////////////////////////////////////////////////////////////////////
// GetStencilOp
//static
const D3D12_STENCIL_OP RsUtilsD3D12::GetStencilOp( RsStencilOp StencilOp )
{
	return GStencilOp[ (size_t)StencilOp ];
}

//////////////////////////////////////////////////////////////////////////
// GetTextureAddressMode
//static
const D3D12_TEXTURE_ADDRESS_MODE RsUtilsD3D12::GetTextureAddressMode( RsTextureSamplingMode TextureAddressMode )
{
	return GTextureAddressMode[ (size_t)TextureAddressMode ];
}

//////////////////////////////////////////////////////////////////////////
// GetBlend
//static
const D3D12_BLEND RsUtilsD3D12::GetBlend( RsBlendType BlendType )
{
	return GBlend[ (size_t)BlendType ];
}

//////////////////////////////////////////////////////////////////////////
// GetVertexElementFormat
//static
const D3D12_BLEND_OP RsUtilsD3D12::GetBlendOp( RsBlendOp BlendOp )
{
	return GBlendOp[ (size_t)BlendOp ];
}

//////////////////////////////////////////////////////////////////////////
// GetVertexElementFormat
//static 
const DXGI_FORMAT RsUtilsD3D12::GetVertexElementFormat( RsVertexElement Element )
{
	DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
	switch( Element.DataType_ )
	{
	case RsVertexDataType::FLOAT32:
		if( Element.Components_ == 1 )
			Format = DXGI_FORMAT_R32_FLOAT;
		else if( Element.Components_ == 2 )
			Format = DXGI_FORMAT_R32G32_FLOAT;
		else if( Element.Components_ == 3 )
			Format = DXGI_FORMAT_R32G32B32_FLOAT;
		else if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case RsVertexDataType::FLOAT16:
		if( Element.Components_ == 1 )
			Format = DXGI_FORMAT_R16_FLOAT;
		else if( Element.Components_ == 2 )
			Format = DXGI_FORMAT_R16G16_FLOAT;
		else if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	case RsVertexDataType::BYTE:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R8G8B8A8_SINT;
		break;
	case RsVertexDataType::BYTE_NORM:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R8G8B8A8_SNORM;
		break;
	case RsVertexDataType::UBYTE:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R8G8B8A8_UINT;
		break;
	case RsVertexDataType::UBYTE_NORM:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case RsVertexDataType::SHORT:
		if( Element.Components_ == 2 )
			Format = DXGI_FORMAT_R16G16_SINT;
		else if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_SINT;
		break;
	case RsVertexDataType::SHORT_NORM:
		if( Element.Components_ == 2 )
			Format = DXGI_FORMAT_R16G16_SNORM;
		else if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_SNORM;
		break;
	case RsVertexDataType::USHORT:
		if( Element.Components_ == 2 )
			Format = DXGI_FORMAT_R16G16_UINT;
		else if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_UINT;
		break;
	case RsVertexDataType::USHORT_NORM:
		if( Element.Components_ == 2 )
			Format = DXGI_FORMAT_R16G16_UNORM;
		else if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_UNORM;
		break;
	case RsVertexDataType::INT:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_SINT;
		break;
	case RsVertexDataType::INT_NORM:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_SNORM;
		break;
	case RsVertexDataType::UINT:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_UINT;
		break;
	case RsVertexDataType::UINT_NORM:
		if( Element.Components_ == 4 )
			Format = DXGI_FORMAT_R16G16B16A16_UNORM;
		break;
	}

	return Format;
}

//////////////////////////////////////////////////////////////////////////
// GetVertexElementFormat
//static 
LPCSTR RsUtilsD3D12::GetSemanticName( RsVertexUsage Usage )
{
	return GSemanticName[ (size_t)Usage ];
}

//////////////////////////////////////////////////////////////////////////
// GetQueryHeapType
//static
D3D12_QUERY_HEAP_TYPE RsUtilsD3D12::GetQueryHeapType( RsQueryType QueryType )
{
	switch( QueryType )
	{
	case RsQueryType::OCCLUSION:
		return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	case RsQueryType::OCCLUSION_BINARY:
		return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	case RsQueryType::TIMESTAMP:
		return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	default:
		BcBreakpoint;
		break;
	}
	return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
}

//////////////////////////////////////////////////////////////////////////
// GetQueryType
//static
D3D12_QUERY_TYPE RsUtilsD3D12::GetQueryType( RsQueryType QueryType )
{
	switch( QueryType )
	{
	case RsQueryType::OCCLUSION:
		return D3D12_QUERY_TYPE_OCCLUSION;
	case RsQueryType::OCCLUSION_BINARY:
		return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
	case RsQueryType::TIMESTAMP:
		return D3D12_QUERY_TYPE_TIMESTAMP;
	default:
		BcBreakpoint;
		break;
	}
	return D3D12_QUERY_TYPE_OCCLUSION;
}
