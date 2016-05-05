#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapConstantsD3D12
struct RsDescriptorHeapConstantsD3D12
{
	static const size_t MAX_SAMPLERS = 8;
	static const size_t MAX_SRVS = 8;
	static const size_t MAX_CBVS = 8;
	static const size_t MAX_UAVS = 8;
	static const size_t MAX_RESOURCES = MAX_SRVS + MAX_CBVS;
	static const size_t SRV_START = 0;
	static const size_t CBV_START = SRV_START + MAX_SRVS;
};

//////////////////////////////////////////////////////////////////////////
// RsProgramBindingD3D12
class RsProgramBindingD3D12
{
public:
	RsProgramBindingD3D12( class RsProgramBinding* Parent, ID3D12Device* Device );
	~RsProgramBindingD3D12();

	ID3D12DescriptorHeap* getSamplerDescriptorHeap() const { return SamplerDescriptorHeap_.Get(); }
	ID3D12DescriptorHeap* getShaderResourceDescriptorHeap() const { return ShaderResourceDescriptorHeap_.Get(); }

	void resourceBarrierTransition( ID3D12GraphicsCommandList* CommandList ) const;

	/**
	 * Gather objects we own.
	 */
	void gatherOwnedObjects( std::vector< ComPtr< ID3D12Object > >& OutList );

private:
	D3D12_SAMPLER_DESC getSamplerDesc( class RsSamplerState* SamplerState );
	D3D12_SHADER_RESOURCE_VIEW_DESC getDefaultSRVDesc( class RsTexture* Texture );
	D3D12_CONSTANT_BUFFER_VIEW_DESC getDefaultCBVDesc( class RsBuffer* Buffer, size_t Offset, size_t Size );

	D3D12_UNORDERED_ACCESS_VIEW_DESC getDefaultUAVDesc( class RsBuffer* Buffer );
	D3D12_UNORDERED_ACCESS_VIEW_DESC getDefaultUAVDesc( class RsTexture* Texture );

private:
	class RsProgramBinding* Parent_;
	ComPtr< ID3D12Device > Device_;
	ComPtr< ID3D12DescriptorHeap > SamplerDescriptorHeap_;
	ComPtr< ID3D12DescriptorHeap > ShaderResourceDescriptorHeap_;

	typedef std::vector< std::pair< class RsResourceD3D12*, D3D12_RESOURCE_STATES > > ResourceStateTransitions;
	ResourceStateTransitions ResourceStateTransitions_;
};
