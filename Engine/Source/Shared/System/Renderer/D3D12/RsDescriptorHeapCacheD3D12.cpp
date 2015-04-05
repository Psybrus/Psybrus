#include "System/Renderer/D3D12/RsDescriptorHeapCacheD3D12.h"
#include "System/Renderer/D3D12/RsResourceD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

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
	Buffers_.fill( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// operator ==
bool RsDescriptorHeapShaderResourceDescD3D12::operator == ( const RsDescriptorHeapShaderResourceDescD3D12& Other ) const
{
	return Textures_ == Other.Textures_ && Buffers_ == Other.Buffers_;
}

//////////////////////////////////////////////////////////////////////////
// Hash specialisations.
namespace std 
{
	size_t hash< RsDescriptorHeapSamplerStateDescD3D12 >::operator()( 
			const RsDescriptorHeapSamplerStateDescD3D12 & DHDesc ) const
	{
		PSY_PROFILE_FUNCTION;
		return BcHash::GenerateCRC32( 0, &DHDesc, sizeof( DHDesc ) );
	}

	size_t hash< RsDescriptorHeapSamplerStateDescArrayD3D12 >::operator()( 
			const RsDescriptorHeapSamplerStateDescArrayD3D12 & DHDesc ) const
	{
		PSY_PROFILE_FUNCTION;
		return BcHash::GenerateCRC32( 0, &DHDesc, sizeof( DHDesc ) );
	}

	size_t hash< RsDescriptorHeapShaderResourceDescD3D12 >::operator()(
			const RsDescriptorHeapShaderResourceDescD3D12 & DHDesc ) const
	{
		PSY_PROFILE_FUNCTION;
		return BcHash::GenerateCRC32( 0, &DHDesc, sizeof( DHDesc ) );
	}

	size_t hash< RsDescriptorHeapShaderResourceDescArrayD3D12 >::operator()(
			const RsDescriptorHeapShaderResourceDescArrayD3D12 & DHDesc ) const
	{
		PSY_PROFILE_FUNCTION;
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
// getSamplersDescriptorHeap
ID3D12DescriptorHeap* RsDescriptorHeapCacheD3D12::getSamplersDescriptorHeap( const RsDescriptorHeapSamplerStateDescArrayD3D12& DHDescs )
{
	PSY_PROFILE_FUNCTION;
	auto FoundIt = SampleStateHeaps_.find( DHDescs );
	if( FoundIt != SampleStateHeaps_.end() )
	{
		return FoundIt->second.Get();
	}

	ComPtr< ID3D12DescriptorHeap > DH;

	D3D12_DESCRIPTOR_HEAP_DESC D3DDHDesc;
	BcMemZero( &D3DDHDesc, sizeof( D3DDHDesc ) );
	D3DDHDesc.Type = D3D12_SAMPLER_DESCRIPTOR_HEAP;
	D3DDHDesc.NumDescriptors = static_cast< UINT >( DHDescs.size() * RsDescriptorHeapSamplerStateDescD3D12::MAX_SAMPLERS );
	D3DDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_SHADER_VISIBLE;
	HRESULT RetVal = Device_->CreateDescriptorHeap( &D3DDHDesc, IID_PPV_ARGS( DH.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3DDHDesc.Type );
	auto BaseDHHandle = DH->GetCPUDescriptorHandleForHeapStart();

	for( INT ShaderIdx = 0; ShaderIdx < DHDescs.size(); ++ShaderIdx )
	{
		const auto& DHDesc = DHDescs[ ShaderIdx ];
		auto ShaderDHHandle = BaseDHHandle.MakeOffsetted( ShaderIdx, DescriptorSize * RsDescriptorHeapSamplerStateDescD3D12::MAX_SAMPLERS );

		for( INT SlotIdx = 0; SlotIdx < DHDesc.SamplerStates_.size(); ++SlotIdx )
		{
			auto SamplerState = DHDesc.SamplerStates_[ SlotIdx ];
			if( SamplerState != nullptr )
			{
				auto SlotDHHandle = ShaderDHHandle.MakeOffsetted( SlotIdx, DescriptorSize );
				const auto SamplerDesc = getSamplerDesc( SamplerState );
				Device_->CreateSampler( &SamplerDesc, SlotDHHandle  );
			}
		}
	}
	SampleStateHeaps_.insert( std::make_pair( DHDescs, DH ) );
	return DH.Get();
}

//////////////////////////////////////////////////////////////////////////
// getShaderResourceDescriptorHeap
ID3D12DescriptorHeap* RsDescriptorHeapCacheD3D12::getShaderResourceDescriptorHeap( const RsDescriptorHeapShaderResourceDescArrayD3D12& DHDescs )
{
	PSY_PROFILE_FUNCTION;
	auto FoundIt = ShaderResourceHeaps_.find( DHDescs );
	if( FoundIt != ShaderResourceHeaps_.end() )
	{
		return FoundIt->second.Get();
	}

	ComPtr< ID3D12DescriptorHeap > DH;

	D3D12_DESCRIPTOR_HEAP_DESC D3DDHDesc;
	BcMemZero( &D3DDHDesc, sizeof( D3DDHDesc ) );
	D3DDHDesc.Type = D3D12_CBV_SRV_UAV_DESCRIPTOR_HEAP;
	D3DDHDesc.NumDescriptors = static_cast< UINT >( DHDescs.size() * RsDescriptorHeapShaderResourceDescD3D12::MAX_RESOURCES );
	D3DDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_SHADER_VISIBLE;
	HRESULT RetVal = Device_->CreateDescriptorHeap( &D3DDHDesc, IID_PPV_ARGS( DH.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3DDHDesc.Type );
	auto BaseDHHandle = DH->GetCPUDescriptorHandleForHeapStart();

	for( INT ShaderIdx = 0; ShaderIdx < DHDescs.size(); ++ShaderIdx )
	{
		const auto& DHDesc = DHDescs[ ShaderIdx ];
		auto ShaderDHHandle = BaseDHHandle.MakeOffsetted( ShaderIdx, DescriptorSize * RsDescriptorHeapShaderResourceDescD3D12::MAX_RESOURCES );

		for( INT SlotIdx = 0; SlotIdx < DHDesc.Textures_.size(); ++SlotIdx )
		{
			auto Texture = DHDesc.Textures_[ SlotIdx ];
			if( Texture != nullptr )
			{
				auto SlotDHHandle = ShaderDHHandle.MakeOffsetted( RsDescriptorHeapShaderResourceDescD3D12::SRV_START + SlotIdx, DescriptorSize );
				auto Resource = Texture->getHandle< RsResourceD3D12* >();
				auto D3DResource = Resource->getInternalResource().Get();
				const auto SRVDesc = getDefaultSRVDesc( Texture );
				Device_->CreateShaderResourceView( D3DResource, &SRVDesc, SlotDHHandle );
			}
		}

		for( INT SlotIdx = 0; SlotIdx < DHDesc.Buffers_.size(); ++SlotIdx )
		{
			auto Buffer = DHDesc.Buffers_[ SlotIdx ];
			if( Buffer != nullptr )
			{
				auto SlotDHHandle = ShaderDHHandle.MakeOffsetted( RsDescriptorHeapShaderResourceDescD3D12::CBV_START + SlotIdx, DescriptorSize );
				const auto CBVDesc = getDefaultCBVDesc( Buffer );
				Device_->CreateConstantBufferView( &CBVDesc, SlotDHHandle );
			}
		}
	}
	ShaderResourceHeaps_.insert( std::make_pair( DHDescs, DH ) );
	return DH.Get();
}

//////////////////////////////////////////////////////////////////////////
// getSamplerDesc
D3D12_SAMPLER_DESC RsDescriptorHeapCacheD3D12::getSamplerDesc( class RsSamplerState* SamplerState )
{
	PSY_PROFILE_FUNCTION;
	D3D12_SAMPLER_DESC OutDesc;
	BcMemZero( &OutDesc, sizeof( OutDesc ) );

	auto Desc = SamplerState->getDesc();

	if( Desc.MinFilter_ == RsTextureFilteringMode::NEAREST &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		OutDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::NEAREST_MIPMAP_LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		OutDesc.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		OutDesc.Filter = D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		OutDesc.Filter = D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::LINEAR )
	{
		OutDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::LINEAR )
	{
		OutDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	}
	else
	{
		// Fallback to something sensible.
		OutDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	}

	OutDesc.AddressU = RsUtilsD3D12::GetTextureAddressMode( Desc.AddressU_ );
	OutDesc.AddressV = RsUtilsD3D12::GetTextureAddressMode( Desc.AddressV_ );
	OutDesc.AddressW = RsUtilsD3D12::GetTextureAddressMode( Desc.AddressW_ );
	OutDesc.MipLODBias = 0.0f;
	OutDesc.MaxAnisotropy = 1;
	OutDesc.ComparisonFunc = D3D12_COMPARISON_NEVER;
	OutDesc.BorderColor[ 0 ] = 0.0f;
	OutDesc.BorderColor[ 1 ] = 0.0f;
	OutDesc.BorderColor[ 2 ] = 0.0f;
	OutDesc.BorderColor[ 3 ] = 0.0f;
	OutDesc.MinLOD = 0;
	OutDesc.MaxLOD = FLT_MAX;

	return OutDesc;
}

//////////////////////////////////////////////////////////////////////////
// getDefaultSRVDesc
D3D12_SHADER_RESOURCE_VIEW_DESC RsDescriptorHeapCacheD3D12::getDefaultSRVDesc( class RsTexture* Texture )
{
	D3D12_SHADER_RESOURCE_VIEW_DESC OutDesc;
	BcMemZero( &OutDesc, sizeof( OutDesc ) );
	OutDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	const auto& TextureDesc = Texture->getDesc();

	switch( TextureDesc.Type_ )
	{
	case RsTextureType::TEX1D:
		{
			OutDesc.Format = RsUtilsD3D12::GetTextureFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			OutDesc.Texture1D.MipLevels = TextureDesc.Levels_;
			OutDesc.Texture1D.MostDetailedMip = 0;
		}
		break;

	case RsTextureType::TEX2D:
		{
			OutDesc.Format = RsUtilsD3D12::GetTextureFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			OutDesc.Texture2D.MipLevels = TextureDesc.Levels_;
			OutDesc.Texture2D.MostDetailedMip = 0;
		}
		break;

	case RsTextureType::TEX3D:
		{
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			OutDesc.Format = RsUtilsD3D12::GetTextureFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.Texture3D.MipLevels = TextureDesc.Levels_;
			OutDesc.Texture3D.MostDetailedMip = 0;
		}
		break;

	case RsTextureType::TEXCUBE:
		{
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			OutDesc.Format = RsUtilsD3D12::GetTextureFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.TextureCube.MipLevels = TextureDesc.Levels_;
			OutDesc.TextureCube.MostDetailedMip = 0;
		}
		break;
	}

	return OutDesc;
}

//////////////////////////////////////////////////////////////////////////
// getDefaultCBVDesc
D3D12_CONSTANT_BUFFER_VIEW_DESC RsDescriptorHeapCacheD3D12::getDefaultCBVDesc( class RsBuffer* Buffer )
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC OutDesc;
	BcMemZero( &OutDesc, sizeof( OutDesc ) );

	const auto& BufferDesc = Buffer->getDesc();
	auto Resource = Buffer->getHandle< RsResourceD3D12* >();
	auto D3DResource = Resource->getInternalResource().Get();
	OutDesc.BufferLocation = D3DResource->GetGPUVirtualAddress();
	OutDesc.SizeInBytes = static_cast< UINT >( BcPotRoundUp( BufferDesc.SizeBytes_, 256 ) );
	return OutDesc;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplers
void RsDescriptorHeapCacheD3D12::destroySamplers( RsSamplerState* SamplerState )
{
	for( auto It = SampleStateHeaps_.begin(); It != SampleStateHeaps_.end(); )
	{
		bool ShouldDestroy = BcFalse;

		for( const auto& SamplerStateDesc : It->first )
		{
			for( const auto& Sampler : SamplerStateDesc.SamplerStates_ )
			{
				if( Sampler == SamplerState )
				{
					ShouldDestroy = BcTrue;
					break;
				}
			}
		}

		if( ShouldDestroy )
		{
			It = SampleStateHeaps_.erase( It );
		}
		else
		{
			++It;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// destroyShaderResources
void RsDescriptorHeapCacheD3D12::destroyShaderResources( RsTexture* Texture )
{
	for( auto It = ShaderResourceHeaps_.begin(); It != ShaderResourceHeaps_.end(); )
	{
		bool ShouldDestroy = BcFalse;

		for( const auto& ShaerResourceDesc : It->first )
		{
			for( const auto& SRTexture : ShaerResourceDesc.Textures_ )
			{
				if( SRTexture == Texture )
				{
					ShouldDestroy = BcTrue;
					break;
				}
			}
		}
		
		if( ShouldDestroy )
		{
			It = ShaderResourceHeaps_.erase( It );
		}
		else
		{
			++It;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// destroyShaderResources
void RsDescriptorHeapCacheD3D12::destroyShaderResources( RsBuffer* Buffer )
{
	for( auto It = ShaderResourceHeaps_.begin(); It != ShaderResourceHeaps_.end(); )
	{
		bool ShouldDestroy = BcFalse;

		for( const auto& ShaerResourceDesc : It->first )
		{
			for( const auto& SRBuffer : ShaerResourceDesc.Buffers_ )
			{
				if( SRBuffer == Buffer )
				{
					ShouldDestroy = BcTrue;
					break;
				}
			}
		}

		if( ShouldDestroy )
		{
			It = ShaderResourceHeaps_.erase( It );
		}
		else
		{
			++It;
		}
	}
}