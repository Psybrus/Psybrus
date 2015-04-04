#include "System/Renderer/D3D12/RsDescriptorHeapCacheD3D12.h"


#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsDescriptorHeapSamplerStateDescD3D12::RsDescriptorHeapSamplerStateDescD3D12()
{
	SamplerStates_.fill( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// operator ==
bool RsDescriptorHeapSamplerStateDescD3D12::operator == ( const RsDescriptorHeapSamplerStateDescD3D12& Other ) const
{
	return SamplerStates_ == Other.SamplerStates_;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsDescriptorHeapShaderResourceDescD3D12::RsDescriptorHeapShaderResourceDescD3D12()
{
	Textures_.fill( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// operator ==
bool RsDescriptorHeapShaderResourceDescD3D12::operator == ( const RsDescriptorHeapShaderResourceDescD3D12& Other ) const
{
	return Textures_ == Other.Textures_;
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsDescriptorHeapConstantBufferDescD3D12::RsDescriptorHeapConstantBufferDescD3D12()
{
	ConstantBuffers_.fill( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// operator ==
bool RsDescriptorHeapConstantBufferDescD3D12::operator == ( const RsDescriptorHeapConstantBufferDescD3D12& Other ) const
{
	return ConstantBuffers_ == Other.ConstantBuffers_;
}

//////////////////////////////////////////////////////////////////////////
// Hash specialisations.
namespace std 
{
	size_t hash< RsDescriptorHeapSamplerStateDescD3D12 >::operator()( 
			const RsDescriptorHeapSamplerStateDescD3D12 & DHDesc ) const
	{
		return BcHash::GenerateCRC32( 0, &DHDesc, sizeof( DHDesc ) );
	}

	size_t hash< RsDescriptorHeapShaderResourceDescD3D12 >::operator()(
			const RsDescriptorHeapShaderResourceDescD3D12 & DHDesc ) const
	{
		return BcHash::GenerateCRC32( 0, &DHDesc, sizeof( DHDesc ) );
	}

	size_t hash< RsDescriptorHeapConstantBufferDescD3D12 >::operator()( 
			const RsDescriptorHeapConstantBufferDescD3D12 & DHDesc ) const
	{
		return BcHash::GenerateCRC32( 0, &DHDesc, sizeof( DHDesc ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsDescriptorHeapCacheD3D12::RsDescriptorHeapCacheD3D12( ID3D12Device* Device ):
	Device_( Device )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsDescriptorHeapCacheD3D12::~RsDescriptorHeapCacheD3D12()
{
}

//////////////////////////////////////////////////////////////////////////
// getDescriptorHeap
ID3D12DescriptorHeap* RsDescriptorHeapCacheD3D12::getDescriptorHeap( const RsDescriptorHeapSamplerStateDescD3D12& DHDesc )
{
	auto FoundIt = SampleStateHeaps_.find( DHDesc );
	if( FoundIt != SampleStateHeaps_.end() )
	{
		return FoundIt->second.Get();
	}

	ComPtr< ID3D12DescriptorHeap > DH;

	// TODO: CREATE ONE!

	SampleStateHeaps_.insert( std::make_pair( DHDesc, DH ) );
	return DH.Get();
}

//////////////////////////////////////////////////////////////////////////
// getDescriptorHeap
ID3D12DescriptorHeap* RsDescriptorHeapCacheD3D12::getDescriptorHeap( const RsDescriptorHeapShaderResourceDescD3D12& DHDesc )
{
	auto FoundIt = ShaderResourceHeaps_.find( DHDesc );
	if( FoundIt != ShaderResourceHeaps_.end() )
	{
		return FoundIt->second.Get();
	}

	ComPtr< ID3D12DescriptorHeap > DH;

	// TODO: CREATE ONE!

	ShaderResourceHeaps_.insert( std::make_pair( DHDesc, DH ) );
	return DH.Get();
}

//////////////////////////////////////////////////////////////////////////
// getDescriptorHeap
ID3D12DescriptorHeap* RsDescriptorHeapCacheD3D12::getDescriptorHeap( const RsDescriptorHeapConstantBufferDescD3D12& DHDesc )
{
	auto FoundIt = ConstantBufferHeaps_.find( DHDesc );
	if( FoundIt != ConstantBufferHeaps_.end() )
	{
		return FoundIt->second.Get();
	}

	ComPtr< ID3D12DescriptorHeap > DH;

	// TODO: CREATE ONE!

	ConstantBufferHeaps_.insert( std::make_pair( DHDesc, DH ) );
	return DH.Get();
}
