#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapSamplerStateDescD3D12
struct RsDescriptorHeapSamplerStateDescD3D12
{
	RsDescriptorHeapSamplerStateDescD3D12();
	bool operator == ( const RsDescriptorHeapSamplerStateDescD3D12& Other ) const;
	std::array< class RsSamplerState*, 32 > SamplerStates_;
};

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapShaderResourceDescD3D12
struct RsDescriptorHeapShaderResourceDescD3D12
{
	RsDescriptorHeapShaderResourceDescD3D12();
	bool operator == ( const RsDescriptorHeapShaderResourceDescD3D12& Other ) const;
	std::array< class RsTexture*, 32 > Textures_;
};

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapConstantBufferDescD3D12
struct RsDescriptorHeapConstantBufferDescD3D12
{
	RsDescriptorHeapConstantBufferDescD3D12();
	bool operator == ( const RsDescriptorHeapConstantBufferDescD3D12& Other ) const;
	std::array< class RsBuffer*, 8 > ConstantBuffers_;
};

//////////////////////////////////////////////////////////////////////////
// Hash specialisations.
namespace std 
{
	template<>
	struct hash< RsDescriptorHeapSamplerStateDescD3D12 >
	{
		size_t operator()( const RsDescriptorHeapSamplerStateDescD3D12 & DHDesc ) const;
	};

	template<>
	struct hash< RsDescriptorHeapShaderResourceDescD3D12 >
	{
		size_t operator()( const RsDescriptorHeapShaderResourceDescD3D12 & DHDesc ) const;
	};

	template<>
	struct hash< RsDescriptorHeapConstantBufferDescD3D12 >
	{
		size_t operator()( const RsDescriptorHeapConstantBufferDescD3D12 & DHDesc ) const;
	};
}

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapCacheD3D12
class RsDescriptorHeapCacheD3D12
{
public:
	RsDescriptorHeapCacheD3D12( ID3D12Device* Device );
	~RsDescriptorHeapCacheD3D12();

	/**
	 * Get descriptor heap.
	 */
	ID3D12DescriptorHeap* getDescriptorHeap( const RsDescriptorHeapSamplerStateDescD3D12& DHDesc );
	ID3D12DescriptorHeap* getDescriptorHeap( const RsDescriptorHeapShaderResourceDescD3D12& DHDesc );
	ID3D12DescriptorHeap* getDescriptorHeap( const RsDescriptorHeapConstantBufferDescD3D12& DHDesc );

private:
	ComPtr< ID3D12Device > Device_;

	std::unordered_map< RsDescriptorHeapSamplerStateDescD3D12, ComPtr< ID3D12DescriptorHeap > > SampleStateHeaps_;
	std::unordered_map< RsDescriptorHeapShaderResourceDescD3D12, ComPtr< ID3D12DescriptorHeap > > ShaderResourceHeaps_;
	std::unordered_map< RsDescriptorHeapConstantBufferDescD3D12, ComPtr< ID3D12DescriptorHeap > > ConstantBufferHeaps_;


};
