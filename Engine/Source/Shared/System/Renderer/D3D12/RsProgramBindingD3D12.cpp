#include "System/Renderer/D3D12/RsProgramBindingD3D12.h"
#include "System/Renderer/D3D12/RsDescriptorHeapCacheD3D12.h"
#include "System/Renderer/D3D12/RsProgramD3D12.h"
#include "System/Renderer/D3D12/RsResourceD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsProgramBinding.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramBindingD3D12::RsProgramBindingD3D12( class RsProgramBinding* Parent, ID3D12Device* Device ):
	Parent_( Parent ),
	Device_( Device )
{
	Parent->setHandle( this );
	const auto& Desc = Parent->getDesc();
	RsProgramD3D12* ProgramD3D12 = Parent->getProgram()->getHandle< RsProgramD3D12* >();

	const BcU32 DESCRIPTOR_STAGES = (BcU32)RsShaderType::MAX;

	// Create sampler descriptor heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC D3DDHDesc;
		BcMemZero( &D3DDHDesc, sizeof( D3DDHDesc ) );
		D3DDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		D3DDHDesc.NumDescriptors = static_cast< UINT >( DESCRIPTOR_STAGES * RsDescriptorHeapConstantsD3D12::MAX_SAMPLERS );
		D3DDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT RetVal = Device_->CreateDescriptorHeap( &D3DDHDesc, IID_PPV_ARGS( SamplerDescriptorHeap_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );

		auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3DDHDesc.Type );
		CD3DX12_CPU_DESCRIPTOR_HANDLE BaseDHHandle( SamplerDescriptorHeap_->GetCPUDescriptorHandleForHeapStart() );

		for( INT StageIdx = 0; StageIdx < DESCRIPTOR_STAGES; ++StageIdx )
		{
			auto ShaderType = static_cast< RsShaderType >( StageIdx );
			for( BcU32 Idx = 0; Idx < Desc.SamplerStates_.size(); ++Idx )
			{
				auto& Sampler = Desc.SamplerStates_[ Idx ];
				BcU32 SamplerSlotIdx = ProgramD3D12->getSamplerSlot( ShaderType, Idx );
				CD3DX12_CPU_DESCRIPTOR_HANDLE ShaderDHHandle( BaseDHHandle, StageIdx, DescriptorSize * RsDescriptorHeapConstantsD3D12::MAX_SAMPLERS );
				if( Sampler != nullptr && SamplerSlotIdx != BcErrorCode )
				{
					auto SamplerDesc = getSamplerDesc( Sampler );
					CD3DX12_CPU_DESCRIPTOR_HANDLE SlotDHHandle( ShaderDHHandle, SamplerSlotIdx, DescriptorSize );
					Device_->CreateSampler( &SamplerDesc, SlotDHHandle );
				}
			}
		}
	}

	// Create shader resource descriptor + constant buffer heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC D3DDHDesc;
		BcMemZero( &D3DDHDesc, sizeof( D3DDHDesc ) );
		D3DDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		D3DDHDesc.NumDescriptors = static_cast< UINT >( DESCRIPTOR_STAGES * RsDescriptorHeapConstantsD3D12::MAX_RESOURCES );
		D3DDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT RetVal = Device_->CreateDescriptorHeap( &D3DDHDesc, IID_PPV_ARGS( ShaderResourceDescriptorHeap_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );

		auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3DDHDesc.Type );
		auto BaseDHHandle = ShaderResourceDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

		for( INT StageIdx = 0; StageIdx < DESCRIPTOR_STAGES; ++StageIdx )
		{
			auto ShaderType = static_cast< RsShaderType >( StageIdx );
			CD3DX12_CPU_DESCRIPTOR_HANDLE ShaderDHHandle( BaseDHHandle, StageIdx, DescriptorSize * RsDescriptorHeapConstantsD3D12::MAX_RESOURCES );

			for( INT Idx = 0; Idx < Desc.ShaderResourceSlots_.size(); ++Idx )
			{
				auto& SRVSlot = Desc.ShaderResourceSlots_[ Idx ];
				BcU32 SRVSlotIdx = ProgramD3D12->getSRVSlot( ShaderType, Idx );
				if( SRVSlot.Resource_ != nullptr && SRVSlotIdx != BcErrorCode )
				{
					CD3DX12_CPU_DESCRIPTOR_HANDLE SlotDHHandle( ShaderDHHandle, RsDescriptorHeapConstantsD3D12::SRV_START + SRVSlotIdx, DescriptorSize );
					switch( SRVSlot.Type_ )
					{
						case RsShaderResourceType::INVALID:
							break;
						case RsShaderResourceType::BUFFER:
						{
							BcBreakpoint;
							break;
						}
						case RsShaderResourceType::TEXTURE:
						{
							auto SRVDesc = getDefaultSRVDesc( SRVSlot.Texture_ );
							auto SRVResource = SRVSlot.Resource_->getHandle< RsResourceD3D12* >();
							auto D3DResource = SRVResource->getInternalResource().Get();
							Device_->CreateShaderResourceView( D3DResource, &SRVDesc, SlotDHHandle );
							break;
						}
						default:
							BcBreakpoint;
					}
				}
			}

			for( INT Idx = 0; Idx < Desc.UniformBuffers_.size(); ++Idx )
			{
				auto& UniformBuffer = Desc.UniformBuffers_[ Idx ];
				BcU32 UniformBufferSlotIdx = ProgramD3D12->getCBSlot( ShaderType, Idx );
				if( UniformBuffer != nullptr && UniformBufferSlotIdx != BcErrorCode )
				{
					CD3DX12_CPU_DESCRIPTOR_HANDLE SlotDHHandle( ShaderDHHandle, RsDescriptorHeapConstantsD3D12::CBV_START + UniformBufferSlotIdx, DescriptorSize );
					auto CBVDesc = getDefaultCBVDesc( UniformBuffer, 0 );
					Device_->CreateConstantBufferView( &CBVDesc, SlotDHHandle );
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Dto
RsProgramBindingD3D12::~RsProgramBindingD3D12()
{
}

//////////////////////////////////////////////////////////////////////////
// getSamplerDesc
D3D12_SAMPLER_DESC RsProgramBindingD3D12::getSamplerDesc( class RsSamplerState* SamplerState )
{
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
	OutDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
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
D3D12_SHADER_RESOURCE_VIEW_DESC RsProgramBindingD3D12::getDefaultSRVDesc( class RsTexture* Texture )
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
D3D12_CONSTANT_BUFFER_VIEW_DESC RsProgramBindingD3D12::getDefaultCBVDesc( class RsBuffer* Buffer, size_t Offset )
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC OutDesc;
	BcMemZero( &OutDesc, sizeof( OutDesc ) );

	const auto& BufferDesc = Buffer->getDesc();
	auto Resource = Buffer->getHandle< RsResourceD3D12* >();
	auto D3DResource = Resource->getInternalResource().Get();
	OutDesc.BufferLocation = D3DResource->GetGPUVirtualAddress() + Offset;
	OutDesc.SizeInBytes = static_cast< UINT >( BcPotRoundUp( BufferDesc.SizeBytes_, 256 ) );
	return OutDesc;
}
