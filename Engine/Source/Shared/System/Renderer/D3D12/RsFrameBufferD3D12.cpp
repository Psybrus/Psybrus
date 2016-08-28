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
	Parent->setHandle( this );
	createRTVDescriptorHeap();
	createDSVDescriptorHeap();
	setupRTVs();
	setupDSV();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsFrameBufferD3D12::~RsFrameBufferD3D12()
{
	Parent_->setHandle( 0 );
}

//////////////////////////////////////////////////////////////////////////
// createRTVDescriptorHeap
void RsFrameBufferD3D12::createRTVDescriptorHeap()
{
	HRESULT RetVal = E_FAIL;
	const auto& ParentDesc = Parent_->getDesc();
	D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
	BcMemZero( &RTVHeapDesc, sizeof( RTVHeapDesc ) );
	RTVHeapDesc.NumDescriptors = NumRTVs_ = static_cast< UINT >( ParentDesc.RenderTargets_.size() );
	RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	RetVal = Device_->CreateDescriptorHeap( &RTVHeapDesc, IID_PPV_ARGS( RTV_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

#if !PSY_PRODUCTION
	auto* DebugName = Parent_->getDebugName();
	BcAssert( DebugName != nullptr );
	std::wstring DebugNameW( &DebugName[0], DebugName + BcStrLength( DebugName ) );
	RTV_->SetName( DebugNameW.c_str() );
#endif
}

//////////////////////////////////////////////////////////////////////////
// createDSVDescriptorHeap
void RsFrameBufferD3D12::createDSVDescriptorHeap()
{
	HRESULT RetVal = E_FAIL;
	const auto& ParentDesc = Parent_->getDesc();
	if( ParentDesc.DepthStencilTarget_ != nullptr )
	{
		D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc = {};
		BcMemZero( &DSVHeapDesc, sizeof( DSVHeapDesc ) );
		DSVHeapDesc.NumDescriptors = 1;
		DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		RetVal = Device_->CreateDescriptorHeap( &DSVHeapDesc, IID_PPV_ARGS( DSV_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );
	}

#if !PSY_PRODUCTION
	auto* DebugName = Parent_->getDebugName();
	BcAssert( DebugName != nullptr );
	if( DSV_ )
	{
		std::wstring DebugNameW( &DebugName[0], DebugName + BcStrLength( DebugName ) );
		DSV_->SetName( DebugNameW.c_str() );
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// setupRTVs
void RsFrameBufferD3D12::setupRTVs()
{
	const auto& ParentDesc = Parent_->getDesc();
	auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
	auto RTVDescriptorHandle = RTV_->GetCPUDescriptorHandleForHeapStart();
	for( INT Idx = 0; Idx < static_cast< INT >( ParentDesc.RenderTargets_.size() ); ++Idx )
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE ThisDescriptorHandle( RTVDescriptorHandle, Idx, DescriptorSize );
		auto RTV = ParentDesc.RenderTargets_[ Idx ];
		auto Format = RsResourceFormat::UNKNOWN;
		if( RTV.Texture_!= nullptr )
		{
			const auto& RTTextureDesc = RTV.Texture_->getDesc();
			auto RTResource = RTV.Texture_->getHandle< RsResourceD3D12* >();
			BcAssert( RTResource );
			D3D12_RENDER_TARGET_VIEW_DESC RTVDesc;
			BcMemZero( &RTVDesc, sizeof( RTVDesc ) );
			switch( RTTextureDesc.Type_ )
			{
			case RsTextureType::TEX2D:
				{
					RTVDesc.Format = RsUtilsD3D12::GetResourceFormat( RTTextureDesc.Format_ ).RTVFormat_;
					RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
					RTVDesc.Texture2D.MipSlice = RTV.Level_;
				}
				break;
			case RsTextureType::TEXCUBE:
				{
					RTVDesc.Format = RsUtilsD3D12::GetResourceFormat( RTTextureDesc.Format_ ).RTVFormat_;
					RTVDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
					RTVDesc.Texture2DArray.MipSlice = RTV.Level_;
					RTVDesc.Texture2DArray.FirstArraySlice = UINT( RTV.Face_ );
					RTVDesc.Texture2DArray.ArraySize = 1;
				}
				break;

			default:
				BcBreakpoint;
				break;
			}	
			Device_->CreateRenderTargetView( RTResource->getInternalResource().Get(), &RTVDesc, ThisDescriptorHandle );
			Format = RTTextureDesc.Format_;
		}

		// Append hash.
		FormatHash_ = BcHash::GenerateCRC32( FormatHash_, &Format, sizeof( Format ) );
	}
}

//////////////////////////////////////////////////////////////////////////
// setupDSV
void RsFrameBufferD3D12::setupDSV()
{
	const auto& ParentDesc = Parent_->getDesc();
	auto DSTexture = ParentDesc.DepthStencilTarget_;
	auto Format = RsResourceFormat::UNKNOWN;
	if( DSTexture != nullptr )
	{
		auto DSVDescriptorHandle = DSV_->GetCPUDescriptorHandleForHeapStart();
		const auto& DSTextureDesc = DSTexture->getDesc();
		auto DSResource = DSTexture->getHandle< RsResourceD3D12* >();
		BcAssert( DSResource );
		D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc;
		BcMemZero( &DSVDesc, sizeof( DSVDesc ) );

		switch( DSTextureDesc.Type_ )
		{
		case RsTextureType::TEX2D:
			{
				DSVDesc.Format = RsUtilsD3D12::GetResourceFormat( DSTextureDesc.Format_ ).DSVFormat_;
				DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				DSVDesc.Texture2D.MipSlice = 0;
			}
			break;

		default:
			BcBreakpoint;
			break;
		}
		Device_->CreateDepthStencilView( DSResource->getInternalResource().Get(), &DSVDesc, DSVDescriptorHandle );

		Format = DSTextureDesc.Format_;
	}

	// Append hash.
	FormatHash_ = BcHash::GenerateCRC32( FormatHash_, &Format, sizeof( Format ) );
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
		auto DescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
		auto BaseRTVDescriptorHandle = RTV_->GetCPUDescriptorHandleForHeapStart();
		for( BcU32 Idx = 0; Idx < NumRTVs_; ++Idx )
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE ThisRTVDescriptorHandle( BaseRTVDescriptorHandle, Idx, DescriptorSize );
			FLOAT D3DColour[4] = { Colour.r(), Colour.g(), Colour.b(), Colour.a() };
			CommandList->ClearRenderTargetView( ThisRTVDescriptorHandle, D3DColour, 0, nullptr );
		}
	}
	if( DSV_ && ( EnableClearDepth || EnableClearStencil ) )
	{
		auto BaseDSVDescriptorHandle = DSV_->GetCPUDescriptorHandleForHeapStart();		
		CommandList->ClearDepthStencilView( 
			BaseDSVDescriptorHandle,
			static_cast< D3D12_CLEAR_FLAGS >( 
				( EnableClearDepth ? D3D12_CLEAR_FLAG_DEPTH : 0 ) | 
				( EnableClearStencil ? D3D12_CLEAR_FLAG_STENCIL : 0 ) ),
			1.0f, 0, 0, nullptr );
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
		auto RTV = ParentDesc.RenderTargets_[ Idx ];
		if( RTV.Texture_ != nullptr )
		{
			auto Resource = RTV.Texture_->getHandle< RsResourceD3D12* >();
			BcAssert( Resource );
			Resource->resourceBarrierTransition( CommandList, D3D12_RESOURCE_STATE_RENDER_TARGET );
		}
	}
	if( ParentDesc.DepthStencilTarget_ != nullptr )
	{
		auto DepthStencil = ParentDesc.DepthStencilTarget_;
		auto Resource = DepthStencil->getHandle< RsResourceD3D12* >();
		Resource->resourceBarrierTransition( CommandList, D3D12_RESOURCE_STATE_DEPTH_WRITE );
	}

	auto BaseRTVDescriptorHandle = RTV_ ? RTV_->GetCPUDescriptorHandleForHeapStart() : D3D12_CPU_DESCRIPTOR_HANDLE();
	auto BaseDSVDescriptorHandle = DSV_ ? DSV_->GetCPUDescriptorHandleForHeapStart() : D3D12_CPU_DESCRIPTOR_HANDLE();

	CommandList->OMSetRenderTargets(
		NumRTVs_,
		RTV_ ? &BaseRTVDescriptorHandle : nullptr,
		TRUE,
		DSV_ ? &BaseDSVDescriptorHandle : nullptr );
}

//////////////////////////////////////////////////////////////////////////
// transitionToRead
void RsFrameBufferD3D12::transitionToRead( ID3D12GraphicsCommandList* CommandList )
{
	const auto ParentDesc = Parent_->getDesc();
	
	// Setup usage.
	D3D12_RESOURCE_STATES Usage = 
		static_cast< D3D12_RESOURCE_STATES >( 
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

	// Resource barriers.
	for( size_t Idx = 0; Idx < ParentDesc.RenderTargets_.size(); ++Idx )
	{
		auto RTV = ParentDesc.RenderTargets_[ Idx ];
		if( RTV.Texture_ != nullptr )
		{
			const auto& RenderTargetDesc = RTV.Texture_->getDesc();
			if( ( RenderTargetDesc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
			{ 
				auto Resource = RTV.Texture_->getHandle< RsResourceD3D12* >();
				Resource->resourceBarrierTransition( CommandList, Usage );
			}
		}
	}

	if( ParentDesc.DepthStencilTarget_ != nullptr )
	{
		auto DepthStencil = ParentDesc.DepthStencilTarget_;
		const auto& DepthStencilDesc = DepthStencil->getDesc();
		if( ( DepthStencilDesc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
		{ 
			auto Resource = DepthStencil->getHandle< RsResourceD3D12* >();
			Resource->resourceBarrierTransition( CommandList, Usage );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// gatherOwnedObjects
void RsFrameBufferD3D12::gatherOwnedObjects( std::vector< ComPtr< ID3D12Object > >& OutList )
{
	OutList.emplace_back( RTV_ );
	OutList.emplace_back( DSV_ );
}
