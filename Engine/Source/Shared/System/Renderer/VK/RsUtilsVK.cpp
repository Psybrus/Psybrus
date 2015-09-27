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
		return VK_FORMAT_BC1_RGB_UNORM;
	case RsTextureFormat::DXT3:
		return VK_FORMAT_BC2_UNORM;
	case RsTextureFormat::DXT5:
		return VK_FORMAT_BC3_UNORM;
	case RsTextureFormat::ETC1:
		return VK_FORMAT_UNDEFINED;
	case RsTextureFormat::D16:
		return VK_FORMAT_D16_UNORM;
	case RsTextureFormat::D24:
		return VK_FORMAT_D24_UNORM;
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
	case VK_FORMAT_BC1_RGB_UNORM:
		return RsTextureFormat::DXT1;
	case VK_FORMAT_BC2_UNORM:
		return RsTextureFormat::DXT3;
	case VK_FORMAT_BC3_UNORM:
		return RsTextureFormat::DXT5;

	case VK_FORMAT_D16_UNORM:
		return RsTextureFormat::D16;
	case VK_FORMAT_D24_UNORM:
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

