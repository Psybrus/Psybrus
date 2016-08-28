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

	size_t NumRenderTargets_;
	std::array< RsResourceFormat, 8 > RTVFormats_;
	RsResourceFormat DSVFormat_;
};

//////////////////////////////////////////////////////////////////////////
// RsGraphicsPipelineStateDescD3D12
struct RsGraphicsPipelineStateDescD3D12
{
	RsGraphicsPipelineStateDescD3D12();
	bool operator == ( const RsGraphicsPipelineStateDescD3D12& Other ) const;

	RsTopologyType Topology_;
	const class RsVertexDeclaration* VertexDeclaration_;
	const class RsProgram* Program_;
	const class RsRenderState* RenderState_;
	RsFrameBufferFormatDescD3D12 FrameBufferFormatDesc_;
};

//////////////////////////////////////////////////////////////////////////
// RsComputePipelineStateDescD3D12
struct RsComputePipelineStateDescD3D12
{
	RsComputePipelineStateDescD3D12();
	bool operator == ( const RsComputePipelineStateDescD3D12& Other ) const;
	
	const class RsProgram* Program_;
};

//////////////////////////////////////////////////////////////////////////
// Hash specialisations.
namespace std 
{
	template<>
	struct hash< RsGraphicsPipelineStateDescD3D12 >
	{
		size_t operator()( const RsGraphicsPipelineStateDescD3D12 & PSD ) const;
	};

	template<>
	struct hash< RsComputePipelineStateDescD3D12 >
	{
		size_t operator()( const RsComputePipelineStateDescD3D12 & PSD ) const;
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
	 * @return New, or existing pipeline state.
	 */
	ID3D12PipelineState* getPipelineState( 
		const RsGraphicsPipelineStateDescD3D12& GraphicsPSDesc,
		ID3D12RootSignature* RootSignature,
		const BcChar* DebugName );

	/**
	 * @return New, or existing pipeline state.
	 */
	ID3D12PipelineState* getPipelineState( 
		const RsComputePipelineStateDescD3D12& ComputePSDesc,
		ID3D12RootSignature* RootSignature,
		const BcChar* DebugName );

	/**
	 * Destroy resources.
	 * Call @a DestroyFunc for every value stored, if true is returned then destroy resource, otherwise leave alone.
	 */
	using ShouldDestroyFunc = std::function< bool( const RsGraphicsPipelineStateDescD3D12&, ID3D12PipelineState* ) >;
	void destroyResources( ShouldDestroyFunc DestroyFunc, std::vector< ComPtr< ID3D12Object > >& OutList );

private:
	using GraphicsPSMap = std::unordered_map< RsGraphicsPipelineStateDescD3D12, ComPtr< ID3D12PipelineState > >;
	using ComputePSMap = std::unordered_map< RsComputePipelineStateDescD3D12, ComPtr< ID3D12PipelineState > >;

	ComPtr< ID3D12Device > Device_;
	GraphicsPSMap GraphicsPSMap_;
	ComputePSMap ComputePSMap_;
};

