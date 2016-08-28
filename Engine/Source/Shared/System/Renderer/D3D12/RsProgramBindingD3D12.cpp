#include "System/Renderer/D3D12/RsProgramBindingD3D12.h"
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
	RsProgram* Program = Parent->getProgram();
	RsProgramD3D12* ProgramD3D12 = Program->getHandle< RsProgramD3D12* >();

	const INT DESCRIPTOR_STAGES = Program->isGraphics() ? 5 : 1;

	// Create sampler descriptor heap.
	{
		D3D12_DESCRIPTOR_HEAP_DESC D3DDHDesc;
		BcMemZero( &D3DDHDesc, sizeof( D3DDHDesc ) );
		D3DDHDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		D3DDHDesc.NumDescriptors = static_cast< UINT >( DESCRIPTOR_STAGES * RsDescriptorHeapConstantsD3D12::MAX_SAMPLERS );
		D3DDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT RetVal = Device_->CreateDescriptorHeap( &D3DDHDesc, IID_PPV_ARGS( SamplerDescriptorHeap_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );

#if !PSY_PRODUCTION
		auto* DebugName = Parent_->getDebugName();
		BcAssert( DebugName != nullptr );
		std::wstring DebugNameW( &DebugName[0], DebugName + BcStrLength( DebugName ) );
		SamplerDescriptorHeap_->SetName( DebugNameW.c_str() );
#endif

	auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3DDHDesc.Type );
		CD3DX12_CPU_DESCRIPTOR_HANDLE BaseDHHandle( SamplerDescriptorHeap_->GetCPUDescriptorHandleForHeapStart() );

		for( INT StageIdx = 0; StageIdx < DESCRIPTOR_STAGES; ++StageIdx )
		{
			auto ShaderType = Program->isGraphics() ? static_cast< RsShaderType >( StageIdx ) : RsShaderType::COMPUTE;
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
		D3DDHDesc.NumDescriptors = 
			static_cast< UINT >( DESCRIPTOR_STAGES * RsDescriptorHeapConstantsD3D12::MAX_SRVS ) +
			static_cast< UINT >( DESCRIPTOR_STAGES * RsDescriptorHeapConstantsD3D12::MAX_CBVS ) +
			static_cast< UINT >( RsDescriptorHeapConstantsD3D12::MAX_UAVS );
		D3DDHDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		HRESULT RetVal = Device_->CreateDescriptorHeap( &D3DDHDesc, IID_PPV_ARGS( ShaderResourceDescriptorHeap_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );

#if !PSY_PRODUCTION
		auto* DebugName = Parent_->getDebugName();
		BcAssert( DebugName != nullptr );
		std::wstring DebugNameW( &DebugName[0], DebugName + BcStrLength( DebugName ) );
		ShaderResourceDescriptorHeap_->SetName( DebugNameW.c_str() );
#endif

		auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3DDHDesc.Type );
		auto BaseDHHandle = ShaderResourceDescriptorHeap_->GetCPUDescriptorHandleForHeapStart();

		for( INT StageIdx = 0; StageIdx < DESCRIPTOR_STAGES; ++StageIdx )
		{
			auto ShaderType = Program->isGraphics() ? static_cast< RsShaderType >( StageIdx ) : RsShaderType::COMPUTE;
			CD3DX12_CPU_DESCRIPTOR_HANDLE ShaderDHHandle( BaseDHHandle, StageIdx, DescriptorSize * RsDescriptorHeapConstantsD3D12::MAX_RESOURCES );

			for( INT Idx = 0; Idx < Desc.ShaderResourceSlots_.size(); ++Idx )
			{
				auto& SRVSlot = Desc.ShaderResourceSlots_[ Idx ];
				BcU32 SRVSlotIdx = ProgramD3D12->getSRVSlot( ShaderType, Idx );
				if( SRVSlot.Resource_ != nullptr && SRVSlotIdx != BcErrorCode )
				{
					BcAssert( SRVSlotIdx < RsDescriptorHeapConstantsD3D12::MAX_SRVS );
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

							ResourceStateTransitions_.emplace_back( std::make_pair( SRVResource, D3D12_RESOURCE_STATE_GENERIC_READ ) );
							break;
						}
						default:
							BcBreakpoint;
					}
				}
			}

			for( INT Idx = 0; Idx < Desc.UniformBuffers_.size(); ++Idx )
			{
				auto& UniformSlot = Desc.UniformBuffers_[ Idx ];
				BcU32 UniformBufferSlotIdx = ProgramD3D12->getCBSlot( ShaderType, Idx );
				if( UniformSlot.Buffer_ != nullptr && UniformBufferSlotIdx != BcErrorCode )
				{
					BcAssert( UniformBufferSlotIdx < RsDescriptorHeapConstantsD3D12::MAX_CBVS );
					CD3DX12_CPU_DESCRIPTOR_HANDLE SlotDHHandle( ShaderDHHandle, RsDescriptorHeapConstantsD3D12::CBV_START + UniformBufferSlotIdx, DescriptorSize );
					auto CBVResource = UniformSlot.Buffer_->getHandle< RsResourceD3D12* >();
					auto CBVDesc = getDefaultCBVDesc( UniformSlot.Buffer_, UniformSlot.Offset_, UniformSlot.Size_ );
					Device_->CreateConstantBufferView( &CBVDesc, SlotDHHandle );
					ResourceStateTransitions_.emplace_back( std::make_pair( CBVResource, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER ) );
				}
			}

			for( INT Idx = 0; Idx < Desc.UnorderedAccessSlots_.size(); ++Idx )
			{
				auto& UAVSlot = Desc.UnorderedAccessSlots_[ Idx ];
				BcU32 UAVSlotIdx = ProgramD3D12->getUAVSlot( ShaderType, Idx );
				if( UAVSlot.Resource_ != nullptr && UAVSlotIdx != BcErrorCode )
				{
					BcAssert( UAVSlotIdx < RsDescriptorHeapConstantsD3D12::MAX_UAVS );
					CD3DX12_CPU_DESCRIPTOR_HANDLE UAVBaseDHHandle( BaseDHHandle, DESCRIPTOR_STAGES, DescriptorSize * RsDescriptorHeapConstantsD3D12::MAX_RESOURCES );
					CD3DX12_CPU_DESCRIPTOR_HANDLE SlotDHHandle( UAVBaseDHHandle, UAVSlotIdx, DescriptorSize );
					auto UAVResource = UAVSlot.Resource_->getHandle< RsResourceD3D12* >();
					auto D3DResource = UAVResource->getInternalResource().Get();
					ResourceStateTransitions_.emplace_back( std::make_pair( UAVResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS ) );
					switch( UAVSlot.Type_ )
					{
						case RsUnorderedAccessType::INVALID:
							break;
						case RsUnorderedAccessType::BUFFER:
						{
							auto UAVDesc = getDefaultUAVDesc( UAVSlot.Buffer_ );
							Device_->CreateUnorderedAccessView( D3DResource, nullptr, &UAVDesc, SlotDHHandle );
							break;
						}
						case RsUnorderedAccessType::TEXTURE:
						{
							auto UAVDesc = getDefaultUAVDesc( UAVSlot.Texture_ );
							Device_->CreateUnorderedAccessView( D3DResource, nullptr, &UAVDesc, SlotDHHandle );
							break;
						}
						default:
							BcBreakpoint;
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Dto
RsProgramBindingD3D12::~RsProgramBindingD3D12()
{
	Parent_->setHandle( 0 );
}

//////////////////////////////////////////////////////////////////////////
// resourceBarrierTransition
void RsProgramBindingD3D12::resourceBarrierTransition( ID3D12GraphicsCommandList* CommandList ) const
{
	for( auto StatePair : ResourceStateTransitions_ )
	{
		StatePair.first->resourceBarrierTransition( CommandList, StatePair.second );
	}
}

//////////////////////////////////////////////////////////////////////////
// gatherOwnedObjects
void RsProgramBindingD3D12::gatherOwnedObjects( std::vector< ComPtr< ID3D12Object > >& OutList )
{
	OutList.emplace_back( SamplerDescriptorHeap_ );
	OutList.emplace_back( ShaderResourceDescriptorHeap_ );
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
		if( Desc.MaxAnisotropy_ > 1 )
		{
			OutDesc.Filter = D3D12_FILTER_ANISOTROPIC;
		}
		else
		{
			OutDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		}
	}
	else
	{
		// Fallback to something sensible.
		OutDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
	}

	OutDesc.AddressU = RsUtilsD3D12::GetTextureAddressMode( Desc.AddressU_ );
	OutDesc.AddressV = RsUtilsD3D12::GetTextureAddressMode( Desc.AddressV_ );
	OutDesc.AddressW = RsUtilsD3D12::GetTextureAddressMode( Desc.AddressW_ );
	OutDesc.MipLODBias = Desc.MipLODBias_;
	OutDesc.MaxAnisotropy = Desc.MaxAnisotropy_;
	OutDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	OutDesc.BorderColor[ 0 ] = Desc.BorderColour_.r();
	OutDesc.BorderColor[ 1 ] = Desc.BorderColour_.g();
	OutDesc.BorderColor[ 2 ] = Desc.BorderColour_.b();
	OutDesc.BorderColor[ 3 ] = Desc.BorderColour_.a();
	OutDesc.MinLOD = Desc.MinLOD_;
	OutDesc.MaxLOD = Desc.MaxLOD_;

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
			OutDesc.Format = RsUtilsD3D12::GetResourceFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
			OutDesc.Texture1D.MipLevels = TextureDesc.Levels_;
			OutDesc.Texture1D.MostDetailedMip = 0;
		}
		break;

	case RsTextureType::TEX2D:
		{
			OutDesc.Format = RsUtilsD3D12::GetResourceFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			OutDesc.Texture2D.MipLevels = TextureDesc.Levels_;
			OutDesc.Texture2D.MostDetailedMip = 0;
		}
		break;

	case RsTextureType::TEX3D:
		{
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			OutDesc.Format = RsUtilsD3D12::GetResourceFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.Texture3D.MipLevels = TextureDesc.Levels_;
			OutDesc.Texture3D.MostDetailedMip = 0;
		}
		break;

	case RsTextureType::TEXCUBE:
		{
			OutDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			OutDesc.Format = RsUtilsD3D12::GetResourceFormat( TextureDesc.Format_ ).SRVFormat_;
			OutDesc.TextureCube.MipLevels = TextureDesc.Levels_;
			OutDesc.TextureCube.MostDetailedMip = 0;
		}
		break;
	}

	return OutDesc;
}

//////////////////////////////////////////////////////////////////////////
// getDefaultCBVDesc
D3D12_CONSTANT_BUFFER_VIEW_DESC RsProgramBindingD3D12::getDefaultCBVDesc( class RsBuffer* Buffer, size_t Offset, size_t Size )
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC OutDesc;
	BcMemZero( &OutDesc, sizeof( OutDesc ) );

	auto Resource = Buffer->getHandle< RsResourceD3D12* >();
	auto D3DResource = Resource->getInternalResource().Get();
	OutDesc.BufferLocation = D3DResource->GetGPUVirtualAddress() + Offset;
	OutDesc.SizeInBytes = static_cast< UINT >( BcPotRoundUp( Size, 256 ) );
	return OutDesc;
}

//////////////////////////////////////////////////////////////////////////
// getDefaultUAVDesc
D3D12_UNORDERED_ACCESS_VIEW_DESC RsProgramBindingD3D12::getDefaultUAVDesc( class RsBuffer* Buffer )
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC OutDesc;
	BcMemZero( &OutDesc, sizeof( OutDesc ) );

	const auto& BufferDesc = Buffer->getDesc();
	OutDesc.Format = 
		( BufferDesc.StructureStride_ == 0 ? 
			DXGI_FORMAT_R32_TYPELESS : 
			DXGI_FORMAT_UNKNOWN );
	OutDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	OutDesc.Buffer.FirstElement = 0;
	OutDesc.Buffer.NumElements = static_cast< UINT >( ( BufferDesc.SizeBytes_ ) / 
		( BufferDesc.StructureStride_ == 0 ? 
			4 : 
			BufferDesc.StructureStride_ ) );
	OutDesc.Buffer.CounterOffsetInBytes = 0;
	OutDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;
	return OutDesc;
}

//////////////////////////////////////////////////////////////////////////
// getDefaultUAVDesc
D3D12_UNORDERED_ACCESS_VIEW_DESC RsProgramBindingD3D12::getDefaultUAVDesc( class RsTexture* Texture )
{
	D3D12_UNORDERED_ACCESS_VIEW_DESC OutDesc;
	BcMemZero( &OutDesc, sizeof( OutDesc ) );

	const auto& TextureDesc = Texture->getDesc();

	OutDesc.Format = RsUtilsD3D12::GetResourceFormat( TextureDesc.Format_ ).UAVFormat_;

	switch( TextureDesc.Type_ )
	{
		case RsTextureType::TEX1D:
			{
				OutDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
				OutDesc.Texture1D.MipSlice = 0;
			}
			break;

		case RsTextureType::TEX2D:
			{
				OutDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				OutDesc.Texture2D.MipSlice = 0;
				OutDesc.Texture2D.PlaneSlice = 0;
			}
			break;

		case RsTextureType::TEX3D:
			{
				OutDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
				OutDesc.Texture3D.MipSlice = 0;
				OutDesc.Texture3D.FirstWSlice = 0;
				OutDesc.Texture3D.WSize = 0;
			}
			break;
	}

	return OutDesc;
}

