#include "System/Renderer/D3D12/RsUtilsD3D12.h"


//////////////////////////////////////////////////////////////////////////
// Texture formats
namespace 
{
	const RsTextureFormatD3D12 GTextureFormats[] =
	{
		// Colour.
		{ RsTextureFormat::R8,					
			DXGI_FORMAT_R8_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R8_UNORM },
		{ RsTextureFormat::R8G8,				
			DXGI_FORMAT_R8G8_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R8G8_UNORM },
		{ RsTextureFormat::R8G8B8,				
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN },
		{ RsTextureFormat::R8G8B8A8,			
			DXGI_FORMAT_R8G8B8A8_UNORM,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R8G8B8A8_UNORM },
		{ RsTextureFormat::R16F,				
			DXGI_FORMAT_R16_FLOAT,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R16_FLOAT },
		{ RsTextureFormat::R16FG16F,			
			DXGI_FORMAT_R16G16_FLOAT,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R16G16_FLOAT },
		{ RsTextureFormat::R16FG16FB16F,	
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN },
		{ RsTextureFormat::R16FG16FB16FA16F,	
			DXGI_FORMAT_R16G16B16A16_FLOAT,	DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R16G16B16A16_FLOAT },
		{ RsTextureFormat::R32F,				
			DXGI_FORMAT_R32_FLOAT,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R32_FLOAT },
		{ RsTextureFormat::R32FG32F,			
			DXGI_FORMAT_R32G32_FLOAT,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R32G32_FLOAT },
		{ RsTextureFormat::R32FG32FB32F,		
			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN },
		{ RsTextureFormat::R32FG32FB32FA32F,	
			DXGI_FORMAT_R32G32B32A32_FLOAT,	DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_R32G32B32A32_FLOAT },
		{ RsTextureFormat::DXT1,				
			DXGI_FORMAT_BC1_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC1_UNORM },
		{ RsTextureFormat::DXT3,				
			DXGI_FORMAT_BC2_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC2_UNORM },
		{ RsTextureFormat::DXT5,				
			DXGI_FORMAT_BC3_UNORM,			DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_BC3_UNORM },

		// Depth.
		{ RsTextureFormat::D16,
			DXGI_FORMAT_R16_TYPELESS,		DXGI_FORMAT_D16_UNORM,			DXGI_FORMAT_R16_UNORM },
		{ RsTextureFormat::D24, 
			DXGI_FORMAT_R24G8_TYPELESS,		DXGI_FORMAT_D24_UNORM_S8_UINT,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
		{ RsTextureFormat::D32,
			DXGI_FORMAT_R32_TYPELESS,		DXGI_FORMAT_UNKNOWN,			DXGI_FORMAT_UNKNOWN },
		{ RsTextureFormat::D24S8, 
			DXGI_FORMAT_R24G8_TYPELESS,		DXGI_FORMAT_D24_UNORM_S8_UINT,	DXGI_FORMAT_R24_UNORM_X8_TYPELESS },
		{ RsTextureFormat::D32F,
			DXGI_FORMAT_R32_TYPELESS,		DXGI_FORMAT_D32_FLOAT,			DXGI_FORMAT_R32_FLOAT },
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
const D3D12_RESOURCE_USAGE RsUtilsD3D12::GetResourceUsage( RsResourceBindFlags BindFlags )
{
	BcU32 AllowedUsage = 0;
	if( ( BindFlags & RsResourceBindFlags::VERTEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_GENERIC_READ;
	}
	if( ( BindFlags & RsResourceBindFlags::INDEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_GENERIC_READ;
	}
	if( ( BindFlags & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_GENERIC_READ;
	}
	if( ( BindFlags & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_GENERIC_READ;/*
			D3D12_RESOURCE_USAGE_NON_PIXEL_SHADER_RESOURCE | 
			D3D12_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE*/
	}
	if( ( BindFlags & RsResourceBindFlags::STREAM_OUTPUT ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_STREAM_OUT;
	}
	if( ( BindFlags & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_RENDER_TARGET;
	}
	if( ( BindFlags & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_DEPTH;
	}
	if( ( BindFlags & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_UNORDERED_ACCESS;
	}
	if( ( BindFlags & RsResourceBindFlags::PRESENT ) != RsResourceBindFlags::NONE )
	{
		AllowedUsage |= 
			D3D12_RESOURCE_USAGE_PRESENT;
	}

	return static_cast< D3D12_RESOURCE_USAGE >( AllowedUsage );
}
