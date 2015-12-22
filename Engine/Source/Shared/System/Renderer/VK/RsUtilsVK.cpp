#include "System/Renderer/VK/RsUtilsVK.h"

//////////////////////////////////////////////////////////////////////////
// GetTextureFormat
VkFormat RsUtilsVK::GetTextureFormat( RsTextureFormat TextureFormat )
{
	switch( TextureFormat )
	{
	case RsTextureFormat::R8:
		return VK_FORMAT_R8_UNORM;
	case RsTextureFormat::R8G8:
		return VK_FORMAT_R8G8_UNORM;
	case RsTextureFormat::R8G8B8:
		return VK_FORMAT_R8G8B8_UNORM;
	case RsTextureFormat::R8G8B8A8:
		return VK_FORMAT_R8G8B8A8_UNORM;
	case RsTextureFormat::R16F:
		return VK_FORMAT_R16_SFLOAT;
	case RsTextureFormat::R16FG16F:
		return VK_FORMAT_R16G16_SFLOAT;
	case RsTextureFormat::R16FG16FB16F:
		return VK_FORMAT_R16G16B16_SFLOAT;
	case RsTextureFormat::R16FG16FB16FA16F:
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	case RsTextureFormat::R32F:
		return VK_FORMAT_R32_SFLOAT;
	case RsTextureFormat::R32FG32F:
		return VK_FORMAT_R32G32_SFLOAT;
	case RsTextureFormat::R32FG32FB32F:
		return VK_FORMAT_R32G32B32_SFLOAT;
	case RsTextureFormat::R32FG32FB32FA32F:
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	case RsTextureFormat::DXT1:
		return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
	case RsTextureFormat::DXT3:
		return VK_FORMAT_BC2_UNORM_BLOCK;
	case RsTextureFormat::DXT5:
		return VK_FORMAT_BC3_UNORM_BLOCK;
	case RsTextureFormat::ETC1:
		return VK_FORMAT_UNDEFINED;
	case RsTextureFormat::D16:
		return VK_FORMAT_D16_UNORM;
	case RsTextureFormat::D24:
		return VK_FORMAT_X8_D24_UNORM_PACK32;
	case RsTextureFormat::D32:
		return VK_FORMAT_D32_SFLOAT;
	case RsTextureFormat::D24S8:
		return VK_FORMAT_D24_UNORM_S8_UINT;

	default:
		BcBreakpoint;
	}
	return VK_FORMAT_UNDEFINED;
}

RsTextureFormat RsUtilsVK::GetTextureFormat( VkFormat Format )
{
	switch( Format )
	{
	case VK_FORMAT_R8_UNORM:
		return RsTextureFormat::R8;
	case VK_FORMAT_R8G8_UNORM:
		return RsTextureFormat::R8G8;
	case VK_FORMAT_R8G8B8_UNORM:
		return RsTextureFormat::R8G8B8;
	case VK_FORMAT_R8G8B8A8_UNORM:
		return RsTextureFormat::R8G8B8A8;
	case VK_FORMAT_R16_SFLOAT:
		return RsTextureFormat::R16F;
	case VK_FORMAT_R16G16_SFLOAT:
		return RsTextureFormat::R16FG16F;
	case VK_FORMAT_R16G16B16_SFLOAT:
		return RsTextureFormat::R16FG16FB16F;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		return RsTextureFormat::R16FG16FB16FA16F;
	case VK_FORMAT_R32_SFLOAT:
		return RsTextureFormat::R32F;
	case VK_FORMAT_R32G32_SFLOAT:
		return RsTextureFormat::R32FG32F;
	case VK_FORMAT_R32G32B32_SFLOAT:
		return RsTextureFormat::R32FG32FB32F;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		return RsTextureFormat::R32FG32FB32FA32F;
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
		return RsTextureFormat::DXT1;
	case VK_FORMAT_BC2_UNORM_BLOCK:
		return RsTextureFormat::DXT3;
	case VK_FORMAT_BC3_UNORM_BLOCK:
		return RsTextureFormat::DXT5;

	case VK_FORMAT_D16_UNORM:
		return RsTextureFormat::D16;
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		return RsTextureFormat::D24;
	case VK_FORMAT_D32_SFLOAT:
		return RsTextureFormat::D32;
	case VK_FORMAT_D24_UNORM_S8_UINT:
		return RsTextureFormat::D24S8;

	default:
		BcBreakpoint;
	}
	return RsTextureFormat::UNKNOWN;
}

VkPrimitiveTopology RsUtilsVK::GetPrimitiveTopology( RsTopologyType TopologyType )
{
	switch( TopologyType )
	{
	case RsTopologyType::POINTS:
		return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		break;
	case RsTopologyType::LINE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		break;
	case RsTopologyType::LINE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		break;
	case RsTopologyType::LINE_LIST_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		break;
	case RsTopologyType::LINE_STRIP_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		break;
	case RsTopologyType::TRIANGLE_LIST:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		break;
	case RsTopologyType::TRIANGLE_STRIP:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		break;
	case RsTopologyType::TRIANGLE_LIST_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		break;
	case RsTopologyType::TRIANGLE_STRIP_ADJACENCY:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		break;
	case RsTopologyType::TRIANGLE_FAN:
		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		break;
	case RsTopologyType::PATCH_LIST:
		return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		break;
	default:
		BcBreakpoint;
	}
	return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
}

VkShaderStageFlagBits RsUtilsVK::GetShaderStage( RsShaderType ShaderType )
{
	switch( ShaderType )
	{
	case RsShaderType::VERTEX:
		return VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case RsShaderType::PIXEL:
		return VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case RsShaderType::HULL:
		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case RsShaderType::DOMAIN:
		return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case RsShaderType::GEOMETRY:
		return VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case RsShaderType::COMPUTE:
		return VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	default:
		BcBreakpoint;
	}
	return (VkShaderStageFlagBits)0;
}

VkFormat RsUtilsVK::GetVertexElementFormat( RsVertexElement Element )
{
	VkFormat Format = VK_FORMAT_UNDEFINED;
	switch( Element.DataType_ )
	{
	case RsVertexDataType::FLOAT32:
		if( Element.Components_ == 1 )
			Format = VK_FORMAT_R32_SFLOAT;
		else if( Element.Components_ == 2 )
			Format = VK_FORMAT_R32G32_SFLOAT;
		else if( Element.Components_ == 3 )
			Format = VK_FORMAT_R32G32B32_SFLOAT;
		else if( Element.Components_ == 4 )
			Format = VK_FORMAT_R32G32B32A32_SFLOAT;
		break;
	case RsVertexDataType::FLOAT16:
		if( Element.Components_ == 1 )
			Format = VK_FORMAT_R16_SFLOAT;
		else if( Element.Components_ == 2 )
			Format = VK_FORMAT_R16G16_SFLOAT;
		else if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_SFLOAT;
		break;
	case RsVertexDataType::BYTE:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R8G8B8A8_SINT;
		break;
	case RsVertexDataType::BYTE_NORM:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R8G8B8A8_SNORM;
		break;
	case RsVertexDataType::UBYTE:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R8G8B8A8_UINT;
		break;
	case RsVertexDataType::UBYTE_NORM:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	case RsVertexDataType::SHORT:
		if( Element.Components_ == 2 )
			Format = VK_FORMAT_R16G16_SINT;
		else if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_SINT;
		break;
	case RsVertexDataType::SHORT_NORM:
		if( Element.Components_ == 2 )
			Format = VK_FORMAT_R16G16_SNORM;
		else if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_SNORM;
		break;
	case RsVertexDataType::USHORT:
		if( Element.Components_ == 2 )
			Format = VK_FORMAT_R16G16_UINT;
		else if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_UINT;
		break;
	case RsVertexDataType::USHORT_NORM:
		if( Element.Components_ == 2 )
			Format = VK_FORMAT_R16G16_UNORM;
		else if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_UNORM;
		break;
	case RsVertexDataType::INT:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_SINT;
		break;
	case RsVertexDataType::INT_NORM:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_SNORM;
		break;
	case RsVertexDataType::UINT:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_UINT;
		break;
	case RsVertexDataType::UINT_NORM:
		if( Element.Components_ == 4 )
			Format = VK_FORMAT_R16G16B16A16_UNORM;
		break;
	}

	return Format;
}

VkBufferUsageFlagBits RsUtilsVK::GetBufferUsageFlags( RsResourceBindFlags BindFlags )
{
	BcU32 Flags = 0;
	if( ( BindFlags & RsResourceBindFlags::VERTEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		Flags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	else if( ( BindFlags & RsResourceBindFlags::INDEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		Flags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}
	else if( ( BindFlags & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE )
	{
		Flags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	else if( ( BindFlags & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		Flags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
	}
	return static_cast< VkBufferUsageFlagBits >( Flags );	
}

VkBlendOp RsUtilsVK::GetBlendOp( RsBlendOp Op )
{
	switch( Op )
	{
	case RsBlendOp::ADD:
		return VK_BLEND_OP_ADD;
	case RsBlendOp::SUBTRACT:
		return VK_BLEND_OP_SUBTRACT;
	case RsBlendOp::REV_SUBTRACT:
		return VK_BLEND_OP_REVERSE_SUBTRACT;
	case RsBlendOp::MINIMUM:
		return VK_BLEND_OP_MIN;
	case RsBlendOp::MAXIMUM:
		return VK_BLEND_OP_MAX;
	default:
		BcBreakpoint;
	}
	return VK_BLEND_OP_ADD;
}

VkBlendFactor RsUtilsVK::GetBlendFactor( RsBlendType Type )
{
	switch( Type )
	{
	case RsBlendType::ZERO:
		return VK_BLEND_FACTOR_ZERO;
		break;
	case RsBlendType::ONE:
		return VK_BLEND_FACTOR_ONE;
		break;
	case RsBlendType::SRC_COLOUR:
		return VK_BLEND_FACTOR_SRC_COLOR;
		break;
	case RsBlendType::INV_SRC_COLOUR:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		break;
	case RsBlendType::SRC_ALPHA:
		return VK_BLEND_FACTOR_SRC_ALPHA;
		break;
	case RsBlendType::INV_SRC_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		break;
	case RsBlendType::DEST_COLOUR:
		return VK_BLEND_FACTOR_DST_COLOR;
		break;
	case RsBlendType::INV_DEST_COLOUR:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		break;
	case RsBlendType::DEST_ALPHA:
		return VK_BLEND_FACTOR_DST_ALPHA;
		break;
	case RsBlendType::INV_DEST_ALPHA:
		return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		break;
	default:
		BcBreakpoint;
	}
	return VK_BLEND_FACTOR_ZERO;
}

VkPolygonMode RsUtilsVK::GetPolygonMode( RsFillMode Mode )
{
	switch( Mode )
	{
	case RsFillMode::SOLID:
		return VK_POLYGON_MODE_FILL;
		break;
	case RsFillMode::WIREFRAME:
		return VK_POLYGON_MODE_LINE;
		break;
	default:
		BcBreakpoint;
	}
	return VK_POLYGON_MODE_FILL;
}
