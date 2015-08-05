#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapSamplerStateDescD3D12
struct RsDescriptorHeapSamplerStateDescD3D12
{
	static const size_t MAX_SAMPLERS = 16;

	RsDescriptorHeapSamplerStateDescD3D12();
	bool operator == ( const RsDescriptorHeapSamplerStateDescD3D12& Other ) const;
	std::array< class RsSamplerState*, MAX_SAMPLERS > SamplerStates_;
};

using RsDescriptorHeapSamplerStateDescArrayD3D12 = std::array< RsDescriptorHeapSamplerStateDescD3D12, (size_t)RsShaderType::MAX >;

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapShaderResourceDescD3D12
struct RsDescriptorHeapShaderResourceDescD3D12
{
	static const size_t MAX_SRVS = 16;
	static const size_t MAX_CBVS = 8;
	static const size_t MAX_RESOURCES = MAX_SRVS + MAX_CBVS;
	static const size_t SRV_START = 0;
	static const size_t CBV_START = SRV_START + MAX_SRVS;

	RsDescriptorHeapShaderResourceDescD3D12();
	bool operator == ( const RsDescriptorHeapShaderResourceDescD3D12& Other ) const;
	std::array< class RsTexture*, MAX_SRVS > Textures_;
	std::array< class RsBuffer*, MAX_CBVS > Buffers_;
};

using RsDescriptorHeapShaderResourceDescArrayD3D12 = std::array< RsDescriptorHeapShaderResourceDescD3D12, (size_t)RsShaderType::MAX >;

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
	struct hash< RsDescriptorHeapSamplerStateDescArrayD3D12 >
	{
		size_t operator()( const RsDescriptorHeapSamplerStateDescArrayD3D12 & DHDesc ) const;
	};

	template<>
	struct hash< RsDescriptorHeapShaderResourceDescD3D12 >
	{
		size_t operator()( const RsDescriptorHeapShaderResourceDescD3D12 & DHDesc ) const;
	};

	template<>
	struct hash< RsDescriptorHeapShaderResourceDescArrayD3D12 >
	{
		size_t operator()( const RsDescriptorHeapShaderResourceDescArrayD3D12 & DHDesc ) const;
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
	 * Get samplers descriptor heap.
	 */
	ID3D12DescriptorHeap* getSamplersDescriptorHeap( 
		const RsDescriptorHeapSamplerStateDescArrayD3D12& DHDescs );

	/**
	 * Get shader resource descriptor heap.
	 */
	ID3D12DescriptorHeap* getShaderResourceDescriptorHeap( 
		const RsDescriptorHeapShaderResourceDescArrayD3D12& DHSRVDescs );

	/**
	 * Get sampler desc.
	 */
	D3D12_SAMPLER_DESC getSamplerDesc( class RsSamplerState* SamplerState );

	/**
	 * Get default shader resource view.
	 */
	D3D12_SHADER_RESOURCE_VIEW_DESC getDefaultSRVDesc( class RsTexture* Texture );

	/**
	 * Get default shader resource view.
	 */
	D3D12_CONSTANT_BUFFER_VIEW_DESC getDefaultCBVDesc( class RsBuffer* Buffer );

	/**
	 * Destroy samplers.
	 */
	void destroySamplers( RsSamplerState* SamplerState );

	/**
	 * Destroy shader resources.
	 */
	void destroyShaderResources( RsTexture* Texture );
	void destroyShaderResources( RsBuffer* Buffer );

private:
	ComPtr< ID3D12Device > Device_;
	using SamplerStateDHs = std::unordered_map< RsDescriptorHeapSamplerStateDescArrayD3D12, ComPtr< ID3D12DescriptorHeap > >;
	using ShaderResourceDHs = std::unordered_map< RsDescriptorHeapShaderResourceDescArrayD3D12, ComPtr< ID3D12DescriptorHeap > >;

	SamplerStateDHs SampleStateHeaps_;
	ShaderResourceDHs ShaderResourceHeaps_;
};
