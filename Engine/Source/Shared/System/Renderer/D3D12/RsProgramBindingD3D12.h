#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramBindingD3D12
class RsProgramBindingD3D12
{
public:
	RsProgramBindingD3D12( class RsProgramBinding* Parent, ID3D12Device* Device );
	~RsProgramBindingD3D12();

	ID3D12DescriptorHeap* getSamplerDescriptorHeap() const { return SamplerDescriptorHeap_.Get(); }
	ID3D12DescriptorHeap* getShaderResourceDescriptorHeap() const { return ShaderResourceDescriptorHeap_.Get(); }

private:
	D3D12_SAMPLER_DESC getSamplerDesc( class RsSamplerState* SamplerState );
	D3D12_SHADER_RESOURCE_VIEW_DESC getDefaultSRVDesc( class RsTexture* Texture );
	D3D12_CONSTANT_BUFFER_VIEW_DESC getDefaultCBVDesc( class RsBuffer* Buffer, size_t Offset );

private:
	class RsProgramBinding* Parent_;
	ComPtr< ID3D12Device > Device_;
	ComPtr< ID3D12DescriptorHeap > SamplerDescriptorHeap_;
	ComPtr< ID3D12DescriptorHeap > ShaderResourceDescriptorHeap_;
};
