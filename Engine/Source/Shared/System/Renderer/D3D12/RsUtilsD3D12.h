#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsVertexDeclaration.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureFormatD3D12
struct RsTextureFormatD3D12
{
	RsTextureFormat TextureFormat_;
	DXGI_FORMAT RTVFormat_;
	DXGI_FORMAT DSVFormat_;
	DXGI_FORMAT SRVFormat_;
};

//////////////////////////////////////////////////////////////////////////
// RsUtilsD3D12
class RsUtilsD3D12
{
public:
	static const RsTextureFormatD3D12& GetTextureFormat( RsTextureFormat TextureFormat );
	static const D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology( RsTopologyType Topology );
	static const D3D12_PRIMITIVE_TOPOLOGY_TYPE GetPrimitiveTopologyType( RsTopologyType Topology );
	static const D3D12_RESOURCE_USAGE GetResourceUsage( RsResourceBindFlags BindFlags );
	static const D3D12_FILL_MODE GetFillMode( RsFillMode FillMode );
	static const D3D12_CULL_MODE GetCullMode( RsCullMode CullMode );
	static const D3D12_COMPARISON_FUNC GetComparisonFunc( RsCompareMode CompareMode );
	static const D3D12_STENCIL_OP GetStencilOp( RsStencilOp StencilOp );
	static const D3D12_TEXTURE_ADDRESS_MODE GetTextureAddressMode( RsTextureSamplingMode TextureAddressMode );
	static const D3D12_BLEND GetBlend( RsBlendType BlendType );
	static const D3D12_BLEND_OP GetBlendOp( RsBlendOp BlendOp );
	static const DXGI_FORMAT GetVertexElementFormat( RsVertexElement Element );
	static LPCSTR GetSemanticName( RsVertexUsage Usage );
};
