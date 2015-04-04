#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// RsFrameBufferFormatDescD3D12
struct RsFrameBufferFormatDescD3D12
{
	RsFrameBufferFormatDescD3D12();
	bool operator == ( const RsFrameBufferFormatDescD3D12& Other ) const;

	BcU32 NumRenderTargets_;
	std::array< RsTextureFormat, 8 > RTVFormats_;
	RsTextureFormat DSVFormat_;
};

//////////////////////////////////////////////////////////////////////////
// RsGraphicsPipelineStateDescD3D12
struct RsGraphicsPipelineStateDescD3D12
{
	RsGraphicsPipelineStateDescD3D12();
	bool operator == ( const RsGraphicsPipelineStateDescD3D12& Other ) const;
	size_t hash() const;

	RsTopologyType Topology_;
	class RsVertexDeclaration* VertexDeclaration_;
	class RsProgram* Program_;
	class RsRenderState* RenderState_;
	RsFrameBufferFormatDescD3D12 FrameBufferFormatDesc_;
};

namespace std 
{
	template<>
	struct hash< RsGraphicsPipelineStateDescD3D12 >
	{
		size_t operator()( const RsGraphicsPipelineStateDescD3D12 & PSD ) const;
	};
}

//////////////////////////////////////////////////////////////////////////
// RsPipelineStateCacheD3D12
class RsPipelineStateCacheD3D12
{
public:
	RsPipelineStateCacheD3D12( ID3D12Device* Device );
	~RsPipelineStateCacheD3D12();

	/**
	 *	@return New, or existing pipeline state.
	 */
	ID3D12PipelineState* getPipelineState( const RsGraphicsPipelineStateDescD3D12& GraphicsPSDesc );
	
private:
	using GraphicsPSMap = std::unordered_map< RsGraphicsPipelineStateDescD3D12, ComPtr< ID3D12PipelineState > >;

	ComPtr< ID3D12Device > Device_;
	GraphicsPSMap GraphicsPSMap_;
};

