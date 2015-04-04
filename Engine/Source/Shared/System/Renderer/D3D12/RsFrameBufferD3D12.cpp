#include "System/Renderer/D3D12/RsFrameBufferD3D12.h"
#include "System/Renderer/D3D12/RsResourceD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBufferD3D12::RsFrameBufferD3D12( class RsFrameBuffer* Parent, ID3D12Device* Device ):
	Parent_( Parent ),
	Device_( Device )
{
	HRESULT RetVal = E_FAIL;
	const auto& ParentDesc = Parent_->getDesc();

	// RTV descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
	RTVHeapDesc.NumDescriptors = static_cast< UINT >( ParentDesc.RenderTargets_.size() );
	RTVHeapDesc.Type = D3D12_RTV_DESCRIPTOR_HEAP;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_NONE;
	RetVal = Device_->CreateDescriptorHeap( &RTVHeapDesc, IID_PPV_ARGS( RTV_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// DSV descriptor heap.
	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc = {};
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DSV_DESCRIPTOR_HEAP;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_NONE;
	RetVal = Device_->CreateDescriptorHeap( &DSVHeapDesc, IID_PPV_ARGS( DSV_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Setup RTVs.
	auto RTVDescriptorHandle = RTV_->GetCPUDescriptorHandleForHeapStart();
	for( INT Idx = 0; Idx < static_cast< INT >( ParentDesc.RenderTargets_.size() ); ++Idx )
	{
		auto ThisDescriptorHandle = RTVDescriptorHandle.MakeOffsetted( Idx );
		auto RTTexture = ParentDesc.RenderTargets_[ Idx ];
		const auto& RTTextureDesc = RTTexture->getDesc();
		auto RTResource = RTTexture->getHandle< RsResourceD3D12* >();
		BcAssert( RTResource );
		D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
		BcMemZero( &RTVDesc, sizeof( RTVDesc ) );

		switch( RTTextureDesc.Type_ )
		{
		case RsTextureType::TEX2D:
			{
				RTVDesc.Format = RsUtilsD3D12::GetTextureFormat( RTTextureDesc.Format_ ).RTVFormat_;
				RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
				RTVDesc.Texture2D.MipSlice = 0;
			}
			break;

		default:
			BcBreakpoint;
			break;
		}	
		Device_->CreateRenderTargetView( RTResource->getInternalResource().Get(), &RTVDesc, ThisDescriptorHandle );
	}

	// Setup DSV.
	auto DSVDescriptorHandle = DSV_->GetCPUDescriptorHandleForHeapStart();
	{
		auto DSTexture = ParentDesc.DepthStencilTarget_;
		const auto& DSTextureDesc = DSTexture->getDesc();
		auto DSResource = DSTexture->getHandle< RsResourceD3D12* >();
		BcAssert( DSResource );
		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc;
		BcMemZero( &DSVDesc, sizeof( DSVDesc ) );

		switch( DSTextureDesc.Type_ )
		{
		case RsTextureType::TEX2D:
			{
				DSVDesc.Format = RsUtilsD3D12::GetTextureFormat( DSTextureDesc.Format_ ).DSVFormat_;
				DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				DSVDesc.Texture2D.MipSlice = 0;
			}
			break;

		default:
			BcBreakpoint;
			break;
		}	
		Device_->CreateDepthStencilView( DSResource->getInternalResource().Get(), &DSVDesc, DSVDescriptorHandle );
	}		


}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsFrameBufferD3D12::~RsFrameBufferD3D12()
{
}

