#include "System/Renderer/D3D12/RsFrameBufferD3D12.h"
#include "System/Renderer/D3D12/RsResourceD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsFrameBufferD3D12::RsFrameBufferD3D12( class RsFrameBuffer* Parent, ID3D12Device* Device ):
	Parent_( Parent ),
	Device_( Device ),
	NumRTVs_( 0 )
{
	createRTVDescriptorHeap();
	createDSVDescriptorHeap();
	setupRTVs();
	setupDSV();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsFrameBufferD3D12::~RsFrameBufferD3D12()
{
}

//////////////////////////////////////////////////////////////////////////
// createRTVDescriptorHeap
void RsFrameBufferD3D12::createRTVDescriptorHeap()
{
	HRESULT RetVal = E_FAIL;
	const auto& ParentDesc = Parent_->getDesc();
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
	RTVHeapDesc.NumDescriptors = NumRTVs_ = static_cast< UINT >( ParentDesc.RenderTargets_.size() );
	RTVHeapDesc.Type = D3D12_RTV_DESCRIPTOR_HEAP;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_NONE;
	RetVal = Device_->CreateDescriptorHeap( &RTVHeapDesc, IID_PPV_ARGS( RTV_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );
}

//////////////////////////////////////////////////////////////////////////
// createDSVDescriptorHeap
void RsFrameBufferD3D12::createDSVDescriptorHeap()
{
	HRESULT RetVal = E_FAIL;
	D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc = {};
	DSVHeapDesc.NumDescriptors = 1;
	DSVHeapDesc.Type = D3D12_DSV_DESCRIPTOR_HEAP;
	DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_NONE;
	RetVal = Device_->CreateDescriptorHeap( &DSVHeapDesc, IID_PPV_ARGS( DSV_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
}

//////////////////////////////////////////////////////////////////////////
// setupRTVs
void RsFrameBufferD3D12::setupRTVs()
{
	const auto& ParentDesc = Parent_->getDesc();
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
}

//////////////////////////////////////////////////////////////////////////
// setupDSV
void RsFrameBufferD3D12::setupDSV()
{
	const auto& ParentDesc = Parent_->getDesc();
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
// clear
void RsFrameBufferD3D12::clear( 
		ID3D12GraphicsCommandList* CommandList, 
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil )
{
	if( EnableClearColour )
	{
		auto BaseRTVDescriptorHandle = RTV_->GetCPUDescriptorHandleForHeapStart();
		for( BcU32 Idx = 0; Idx < NumRTVs_; ++Idx )
		{
			auto ThisRTVDescriptorHandle = BaseRTVDescriptorHandle.MakeOffsetted( Idx );
			FLOAT D3DColour[4] = { Colour.r(), Colour.g(), Colour.b(), Colour.a() };
			CommandList->ClearRenderTargetView( ThisRTVDescriptorHandle, D3DColour, nullptr, 0 );
		}
	}
	if( DSV_ && ( EnableClearDepth || EnableClearStencil ) )
	{
		auto BaseDSVDescriptorHandle = DSV_->GetCPUDescriptorHandleForHeapStart();		
		CommandList->ClearDepthStencilView( 
			BaseDSVDescriptorHandle,
			static_cast< D3D12_CLEAR_FLAG >( 
				( EnableClearDepth ? D3D12_CLEAR_DEPTH : 0 ) | 
				( EnableClearStencil ? D3D12_CLEAR_STENCIL : 0 ) ),
			1.0f, 0, nullptr, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// setRenderTargets
void RsFrameBufferD3D12::setRenderTargets( ID3D12GraphicsCommandList* CommandList )
{
	const auto ParentDesc = Parent_->getDesc();

	// Resource barriers.
	for( size_t Idx = 0; Idx < ParentDesc.RenderTargets_.size(); ++Idx )
	{
		auto RenderTarget = ParentDesc.RenderTargets_[ Idx ];
		auto Resource = RenderTarget->getHandle< RsResourceD3D12* >();
		Resource->resourceBarrierTransition( CommandList, RsResourceBindFlags::RENDER_TARGET );
	}

	if( ParentDesc.DepthStencilTarget_ != nullptr )
	{
		auto DepthStencil = ParentDesc.DepthStencilTarget_;
		auto Resource = DepthStencil->getHandle< RsResourceD3D12* >();
		Resource->resourceBarrierTransition( CommandList, RsResourceBindFlags::DEPTH_STENCIL );
	}

	auto BaseRTVDescriptorHandle = RTV_ ? RTV_->GetCPUDescriptorHandleForHeapStart() : D3D12_CPU_DESCRIPTOR_HANDLE();
	auto BaseDSVDescriptorHandle = DSV_ ? DSV_->GetCPUDescriptorHandleForHeapStart() : D3D12_CPU_DESCRIPTOR_HANDLE();

	CommandList->SetRenderTargets( 
		RTV_ ? &BaseRTVDescriptorHandle : nullptr,
		TRUE,
		NumRTVs_,
		DSV_ ? &BaseDSVDescriptorHandle : nullptr );
}
