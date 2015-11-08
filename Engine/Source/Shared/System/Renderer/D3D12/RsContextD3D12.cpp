/**************************************************************************
*
* File:		RsContextD3D12.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/D3D12/RsContextD3D12.h"
#include "System/Renderer/D3D12/RsFrameBufferD3D12.h"
#include "System/Renderer/D3D12/RsProgramD3D12.h"
#include "System/Renderer/D3D12/RsResourceD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"

#include "System/Renderer/D3D12/Shaders/DefaultVS.h"
#include "System/Renderer/D3D12/Shaders/DefaultPS.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsVertexDeclaration.h"
#include "System/Renderer/RsViewport.h"

#include "System/Os/OsClient.h"
#include "System/Os/OsClientWindows.h"

#include "Base/BcMath.h"
#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Direct3D 12 libraries.
#pragma comment (lib, "D3D12.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment( lib, "D3DCompiler.lib" )

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextD3D12::RsContextD3D12( OsClient* pClient, RsContextD3D12* pParent ):
	RsContext( pParent ),
	Parent_( pParent ),
	Client_( pClient ),
	InsideBeginEnd_( 0 ),
	Width_( 0 ),
	Height_( 0 ),
	Adapter_(),
	CommandQueueDesc_(),
	CommandQueue_(),
	CommandListDatas_(),
	CurrentCommandListData_( -1 ),
	WaitOnCommandListEvent_( 0 ),
	FrameCounter_( 0 ),
	FlushCounter_( 1 ),
	NumSwapBuffers_( 3 ),
	CurrentSwapBuffer_( 0 ),
	DefaultRootSignature_(),
	DefaultPSO_(),
	PSOCache_(),
	GraphicsPSODesc_(),
	Viewports_(),
	ScissorRects_(),
	FrameBuffer_( nullptr ),
	VertexBufferViews_(),
	IndexBufferView_(),
	DHCache_(),
	SamplerStateDescs_(),
	ShaderResourceDescs_(),
	BackBufferFB_(),
	BackBufferRT_(),
	BackBufferDS_( nullptr ),
	OwningThread_( BcErrorCode ),
	ScreenshotRequested_( BcFalse )
{
	BcMemZero( &VertexBufferViews_, sizeof( VertexBufferViews_ ) );
	BcMemZero( &IndexBufferView_, sizeof( IndexBufferView_ ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsContextD3D12::~RsContextD3D12()
{

}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextD3D12::getClient() const
{
	return Client_;
}

//////////////////////////////////////////////////////////////////////////
// getFeatures
//virtual
const RsFeatures& RsContextD3D12::getFeatures() const
{
	return Features_;
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextD3D12::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	switch( CodeType )
	{
	case RsShaderCodeType::D3D11_4_0_LEVEL_9_1:
		return ( FeatureLevel_ >= D3D_FEATURE_LEVEL_9_1 );
		break;

	case RsShaderCodeType::D3D11_4_0_LEVEL_9_2:
		return ( FeatureLevel_ >= D3D_FEATURE_LEVEL_9_2 );
		break;

	case RsShaderCodeType::D3D11_4_0_LEVEL_9_3:
		return ( FeatureLevel_ >= D3D_FEATURE_LEVEL_9_3 );
		break;
		
	case RsShaderCodeType::D3D11_4_0:
		return ( FeatureLevel_ >= D3D_FEATURE_LEVEL_10_0 );
		break;

	case RsShaderCodeType::D3D11_4_1:
		return ( FeatureLevel_ >= D3D_FEATURE_LEVEL_10_1 );
		break;

	case RsShaderCodeType::D3D11_5_0:
		return ( FeatureLevel_ >= D3D_FEATURE_LEVEL_11_0 );
		break;

	case RsShaderCodeType::D3D11_5_1:
		return ( FeatureLevel_ >= D3D_FEATURE_LEVEL_11_1 );
		break;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
// maxShaderCodeType
//virtual
RsShaderCodeType RsContextD3D12::maxShaderCodeType( RsShaderCodeType CodeType ) const
{
	switch( FeatureLevel_ )
	{
	case D3D_FEATURE_LEVEL_9_1:
		return RsShaderCodeType::D3D11_4_0_LEVEL_9_1;
		break;

	case D3D_FEATURE_LEVEL_9_2:
		return RsShaderCodeType::D3D11_4_0_LEVEL_9_2;
		break;

	case D3D_FEATURE_LEVEL_9_3:
		return RsShaderCodeType::D3D11_4_0_LEVEL_9_3;
		break;

	case D3D_FEATURE_LEVEL_10_0:
		return RsShaderCodeType::D3D11_4_0;
		break;

	case D3D_FEATURE_LEVEL_10_1:
		return RsShaderCodeType::D3D11_4_1;
		break;

	case D3D_FEATURE_LEVEL_11_0:
		return RsShaderCodeType::D3D11_5_0;
		break;

	case D3D_FEATURE_LEVEL_11_1:
		return RsShaderCodeType::D3D11_5_1;
		break;
	}
	
	return RsShaderCodeType::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 RsContextD3D12::getWidth() const
{
	BcAssert( InsideBeginEnd_ == 1 );
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextD3D12::getHeight() const
{
	BcAssert( InsideBeginEnd_ == 1 );
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// beginFrame
void RsContextD3D12::beginFrame( BcU32 Width, BcU32 Height )
{
	PSY_PROFILE_FUNCTION;
	BcAssert( InsideBeginEnd_ == 0 );
	++InsideBeginEnd_;

	// Recreate as needed.
	recreateBackBuffers( Width, Height );

	// Grab current swap buffer index.
	CurrentSwapBuffer_ = SwapChain_->GetCurrentBackBufferIndex();

	// Bind up default states.
	setDefaultState();
	setFrameBuffer( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextD3D12::endFrame()
{
	PSY_PROFILE_FUNCTION;
	BcAssert( InsideBeginEnd_ == 1 );
	--InsideBeginEnd_;

	// Transition back buffer to present.
	{
		auto CommandList = getCurrentCommandList();
		RsResourceD3D12* BackBufferResource = BackBufferRT_[ CurrentSwapBuffer_ ]->getHandle< RsResourceD3D12* >();
		BackBufferResource->resourceBarrierTransition( CommandList, D3D12_RESOURCE_STATE_PRESENT );
	}

	// Flush command list (also waits for completion).
	flushCommandList( 
		[ this ]()
		{
			PSY_PROFILER_SECTION( PresentRoot, "DXGI::Present" );

			// Do present.
			HRESULT RetVal = SwapChain_->Present( 0, 0 );
			BcAssert( SUCCEEDED( RetVal ) );
			++FrameCounter_;
		} );
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextD3D12::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextD3D12::setViewport( const class RsViewport& Viewport )
{
	D3D12_VIEWPORT D3DViewport;
	D3DViewport.Width = (FLOAT)Viewport.width();
	D3DViewport.Height = (FLOAT)Viewport.height();
	D3DViewport.TopLeftX = (FLOAT)Viewport.x();
	D3DViewport.TopLeftY = (FLOAT)Viewport.y();
	D3DViewport.MinDepth = 0.0f;
	D3DViewport.MaxDepth = 1.0f;

	D3D12_RECT D3DScissorRect;
	D3DScissorRect.left = 0;
	D3DScissorRect.top = 0;
	D3DScissorRect.right = Viewport.width();
	D3DScissorRect.bottom = Viewport.height();

	Viewports_.fill( D3DViewport );
	ScissorRects_.fill( D3DScissorRect );
}

//////////////////////////////////////////////////////////////////////////
// setScissorRect
void RsContextD3D12::setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height )
{
	D3D12_RECT D3DScissorRect;
	D3DScissorRect.left = X;
	D3DScissorRect.top = Y;
	D3DScissorRect.right = X + Width;
	D3DScissorRect.bottom = Y + Height;
	ScissorRects_.fill( D3DScissorRect );
}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextD3D12::create()
{
	PSY_LOGSCOPEDCATEGORY( "RsContextD3D12" );
	OsClientWindows* Client = dynamic_cast< OsClientWindows* >( Client_ );
	BcAssertMsg( Client != nullptr, "Windows client is not being used!" );
	HRESULT RetVal = E_FAIL;

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

#ifdef _DEBUG
	// Enable the D3D12 debug layer.
	{
		ComPtr< ID3D12Debug > DebugController;
		if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &DebugController ) ) ) )
		{
			DebugController->EnableDebugLayer();
		}
	}
#endif

	// Create DXGI factory.
	RetVal = ::CreateDXGIFactory1( IID_PPV_ARGS( &Factory_ ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Enum adapters.
	for( UINT AdapterIdx = 0; ; ++AdapterIdx )
	{
		ComPtr< IDXGIAdapter > Adapter;
		RetVal = Factory_->EnumAdapters( AdapterIdx, Adapter.ReleaseAndGetAddressOf() );
		if( SUCCEEDED( RetVal ) )
		{
			DXGI_ADAPTER_DESC AdapterDesc;
			Adapter->GetDesc( &AdapterDesc );

			std::wstring DescriptionW( AdapterDesc.Description );
			std::string Description( DescriptionW.begin(), DescriptionW.end() );
			PSY_LOG( "Selecting adapter %s", Description.c_str() );
			PSY_LOG( "- Dedicated Video: %uMb", (int)( AdapterDesc.DedicatedVideoMemory / ( 1024 * 1024 ) ) );
			PSY_LOG( "- Dedicated System: %uMb", (int)( AdapterDesc.DedicatedSystemMemory / ( 1024 * 1024 ) ) );
			PSY_LOG( "- Shared System %uMb", (int)( AdapterDesc.SharedSystemMemory / ( 1024 * 1024 ) ) );
			Adapter_ = Adapter;
			break;
		}
		else
		{
			break;
		}
	}

	// Create default device.
	const D3D_FEATURE_LEVEL Features[] = 
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	for( BcU32 Idx = 0; Idx < BcArraySize( Features ); ++Idx )
	{
		FeatureLevel_ = Features[ Idx ];
		RetVal = D3D12CreateDevice(
			Adapter_.Get(),
			FeatureLevel_, 
			IID_PPV_ARGS( &Device_ ) );
		if( SUCCEEDED( RetVal ) )
		{
			PSY_LOG( "Created feature level %x", Features[ Idx ] );
			break;
		}
	}

	if( Device_ == nullptr )
	{
		BcAssertMsg( BcFalse, "Unable to create device." );
		return;
	}

	Features_.MRT_ = true;
	Features_.DepthTextures_ = true;
	Features_.NPOTTextures_ = true;
	Features_.SeparateBlendState_ = true;
	Features_.AnisotropicFiltering_ = true;
	Features_.AntialiasedLines_ = true;
	Features_.Texture1D_ = true;
	Features_.Texture2D_ = true;
	Features_.Texture3D_ = true;
	Features_.TextureCube_ = true;

	for( int Format = 0; Format < (int)RsTextureFormat::MAX; ++Format )
	{
		auto DXFormat = RsUtilsD3D12::GetTextureFormat( (RsTextureFormat)Format );
		if( DXFormat.SRVFormat_ != DXGI_FORMAT_UNKNOWN )
		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = { DXFormat.SRVFormat_ };
			if( SUCCEEDED( Device_->CheckFeatureSupport( D3D12_FEATURE_FORMAT_SUPPORT,  &FormatSupport, sizeof( FormatSupport ) ) ) )
			{
				if( FormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_TEXTURE2D )
				{
					Features_.TextureFormat_[ Format ] = true;
				}
				if( FormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET )
				{
					Features_.RenderTargetFormat_[ Format ] = true;
				}
			}
		}
		if( DXFormat.RTVFormat_ != DXGI_FORMAT_UNKNOWN )
		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = { DXFormat.RTVFormat_ };
			if( SUCCEEDED( Device_->CheckFeatureSupport( D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof( FormatSupport ) ) ) )
			{
				if( FormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_RENDER_TARGET )
				{
					Features_.RenderTargetFormat_[ Format ] = true;
				}
			}
		}
		if( DXFormat.DSVFormat_ != DXGI_FORMAT_UNKNOWN )
		{
			D3D12_FEATURE_DATA_FORMAT_SUPPORT FormatSupport = { DXFormat.DSVFormat_ };
			if( SUCCEEDED( Device_->CheckFeatureSupport( D3D12_FEATURE_FORMAT_SUPPORT, &FormatSupport, sizeof( FormatSupport ) ) ) )
			{
				if( FormatSupport.Support1 & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL )
				{
					Features_.DepthStencilTargetFormat_[ Format ] = true;
				}
			}
		}
	}

	ComPtr< ID3D12InfoQueue > InfoQueue;
	RetVal = Device_.CopyTo( InfoQueue.GetAddressOf() );
	if ( SUCCEEDED( RetVal ) )
	{
		InfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_CORRUPTION, true );
		InfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_ERROR, true );
		InfoQueue->SetBreakOnSeverity( D3D12_MESSAGE_SEVERITY_WARNING, false );
	}

	// Create command queue.
	BcMemZero( &CommandQueueDesc_, sizeof( CommandQueueDesc_ ) );
	CommandQueueDesc_.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc_.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	RetVal = Device_->CreateCommandQueue( &CommandQueueDesc_, IID_PPV_ARGS( &CommandQueue_ ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Setup swap chain desc.
	BcMemZero( &SwapChainDesc_, sizeof( SwapChainDesc_ ) );
	SwapChainDesc_.BufferCount = NumSwapBuffers_;
	SwapChainDesc_.BufferDesc.Width = Client->getWidth();
	SwapChainDesc_.BufferDesc.Height = Client->getHeight();
	SwapChainDesc_.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc_.OutputWindow = (HWND)Client->getWindowHandle();
	SwapChainDesc_.SampleDesc.Count = 1;
	SwapChainDesc_.Windowed = TRUE;
	SwapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr< IDXGISwapChain > SwapChain;
	RetVal = Factory_->CreateSwapChain( CommandQueue_.Get(), &SwapChainDesc_, &SwapChain );
	BcAssert( SUCCEEDED( RetVal ) );

	RetVal = SwapChain.As( &SwapChain_ );
	BcAssert( SUCCEEDED( RetVal ) );

	// Create present fence.
	FrameCounter_ = 0;

	// Create pipeline state cache.
	PSOCache_.reset( new RsPipelineStateCacheD3D12( Device_.Get() ) );

	// Create descriptor heap cache.
	DHCache_.reset( new RsDescriptorHeapCacheD3D12( Device_.Get() ) );

	// Create default root signature.
	createDefaultRootSignature();

	// Create default pipeline state.
	createDefaultPSO();

	// Create command list data.
	// TODO: Use more than 1. For now this is so we don't need to fence/flush to handle resource deletion.
	createCommandListData( 1 );

	// Flush command list.
	flushCommandList( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// update
void RsContextD3D12::update()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
void RsContextD3D12::destroy()
{
	// Destroy backbuffer.
	for( auto BackBufferFB : BackBufferFB_ )
	{
		destroyFrameBuffer( BackBufferFB );
	}

	for( auto BackBufferRT : BackBufferRT_ )
	{
		destroyTexture( BackBufferRT );
	}

	if( BackBufferDS_ != nullptr )
	{
		destroyTexture( BackBufferDS_ );
	}

	// Cleanup everything.
	for( auto& CommandListData : CommandListDatas_ )
	{
		CommandListData.CommandAllocator_.Reset();
		CommandListData.CommandList_.Reset();
		CommandListData.UploadAllocator_.reset();
	}
	DefaultRootSignature_.Reset();
	DefaultPSO_.Reset();
	DHCache_.reset();
	PSOCache_.reset();
	CommandQueue_.Reset();
	Device_.Reset();
	SwapChain_.Reset();
	Adapter_.Reset();
	Factory_.Reset();
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextD3D12::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextD3D12::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextD3D12::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D12::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	GraphicsPSODesc_.RenderState_ = RenderState;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextD3D12::setSamplerState( BcU32 Handle, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	
	// Bind for each shader based on specified handle.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
	{
		BcU32 SlotIdx = ( Handle >> ( Idx * RsProgramD3D12::BitsPerShader ) ) & RsProgramD3D12::MaxBindPoints;

		if( SlotIdx != RsProgramD3D12::MaxBindPoints )
		{
			SamplerStateDescs_[ Idx ].SamplerStates_[ SlotIdx ] = SamplerState;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextD3D12::setTexture( BcU32 Handle, RsTexture* Texture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	
	// Bind for each shader based on specified handle.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
	{
		BcU32 SlotIdx = ( Handle >> ( Idx * RsProgramD3D12::BitsPerShader ) ) & RsProgramD3D12::MaxBindPoints;

		if( SlotIdx != RsProgramD3D12::MaxBindPoints )
		{
			ShaderResourceDescs_[ Idx ].Textures_[ SlotIdx ] = Texture;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextD3D12::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	GraphicsPSODesc_.Program_ = Program;
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsContextD3D12::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto CommandList = getCurrentCommandList();

	if( IndexBuffer != nullptr )
	{
		auto IndexBufferResource = IndexBuffer->getHandle< RsResourceD3D12* >();
		IndexBufferResource->resourceBarrierTransition( CommandList, D3D12_RESOURCE_STATE_GENERIC_READ );
		IndexBufferView_.BufferLocation = IndexBufferResource->getGPUVirtualAddress();
		IndexBufferView_.SizeInBytes = static_cast< UINT >( IndexBuffer->getDesc().SizeBytes_ );
		IndexBufferView_.Format = DXGI_FORMAT_R16_UINT; // TODO: Select properly
	}
	else
	{
		IndexBufferView_.BufferLocation = 0;
		IndexBufferView_.SizeInBytes = 0;
		IndexBufferView_.Format = DXGI_FORMAT_R16_UINT; // TODO: Select properly
	}
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextD3D12::setVertexBuffer( 
	BcU32 StreamIdx, 
	class RsBuffer* VertexBuffer,
	BcU32 Stride )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto & VertexBufferView = VertexBufferViews_[ StreamIdx ];
	auto CommandList = getCurrentCommandList();

	if( VertexBuffer != nullptr )
	{
		auto VertexBufferResource = VertexBuffer->getHandle< RsResourceD3D12* >();
		VertexBufferResource->resourceBarrierTransition( CommandList, D3D12_RESOURCE_STATE_GENERIC_READ );
		VertexBufferView.BufferLocation = VertexBufferResource->getGPUVirtualAddress();
		VertexBufferView.SizeInBytes = static_cast< UINT >( VertexBuffer->getDesc().SizeBytes_ );
		VertexBufferView.StrideInBytes = Stride;
	}
	else
	{
		VertexBufferView.BufferLocation = 0;
		VertexBufferView.SizeInBytes = 0;
		VertexBufferView.StrideInBytes = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextD3D12::setUniformBuffer( 
	BcU32 Handle, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Bind for each shader based on specified handle.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
	{
		BcU32 SlotIdx = ( Handle >> ( Idx * RsProgramD3D12::BitsPerShader ) ) & RsProgramD3D12::MaxBindPoints;

		if( SlotIdx != RsProgramD3D12::MaxBindPoints )
		{
			ShaderResourceDescs_[ Idx ].Buffers_[ SlotIdx ] = UniformBuffer;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextD3D12::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	GraphicsPSODesc_.VertexDeclaration_ = VertexDeclaration;
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextD3D12::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto CommandList = getCurrentCommandList();

	auto LastFrameBuffer = FrameBuffer_;

	// Even if null, we want backbuffer bound.
	if( FrameBuffer == nullptr )
	{
		FrameBuffer_ = BackBufferFB_[ CurrentSwapBuffer_ ];
	}
	else
	{
		FrameBuffer_ = FrameBuffer;
	}

	// Transition last to read.
	if( LastFrameBuffer != nullptr && LastFrameBuffer != FrameBuffer_ )
	{
		auto D3DFrameBuffer = LastFrameBuffer->getHandle< RsFrameBufferD3D12* >();
		D3DFrameBuffer->transitionToRead( CommandList );
	}

	// Set viewport to framebuffer size.

	const auto& Desc = FrameBuffer_->getDesc().RenderTargets_[ 0 ]->getDesc();
	setViewport( RsViewport( 0, 0, Desc.Width_, Desc.Height_ ) );
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextD3D12::clear( 
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto CommandList = getCurrentCommandList();
	flushState();
	BcAssert( FrameBuffer_ );
	RsFrameBufferD3D12* FrameBufferD3D12 = FrameBuffer_->getHandle< RsFrameBufferD3D12* >();
	FrameBufferD3D12->clear( CommandList, Colour, EnableClearColour, EnableClearDepth, EnableClearStencil );
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextD3D12::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto CommandList = getCurrentCommandList();
	GraphicsPSODesc_.Topology_ = PrimitiveType;
	flushState();

	CommandList->DrawInstanced( NoofIndices, 1, IndexOffset, 0 );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextD3D12::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto CommandList = getCurrentCommandList();
	GraphicsPSODesc_.Topology_ = PrimitiveType;
	flushState();
	
	CommandList->DrawIndexedInstanced( NoofIndices, 1, IndexOffset, VertexOffset, 0 );
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
bool RsContextD3D12::createRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextD3D12::destroyRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	PSOCache_->destroyResources(
		[ this, RenderState ]( const RsGraphicsPipelineStateDescD3D12& PSODesc, ID3D12PipelineState* PSO )->bool
		{
			return PSODesc.RenderState_ == RenderState;
		} );

	if( GraphicsPSODesc_.RenderState_ == RenderState )
	{
		GraphicsPSODesc_.RenderState_ = nullptr;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextD3D12::createSamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextD3D12::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	DHCache_->destroySamplers( SamplerState );

	for( auto& SamplerStateDesc : SamplerStateDescs_ )
	{
		for( auto& Sampler : SamplerStateDesc.SamplerStates_ )
		{
			if( Sampler == SamplerState )
			{
				Sampler = nullptr;
			}
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextD3D12::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	FrameBuffer->setHandle( new RsFrameBufferD3D12( FrameBuffer, Device_.Get() ) );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextD3D12::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( FrameBuffer_ == FrameBuffer )
	{
		FrameBuffer_ = nullptr;
	}

	auto FrameBufferInternal = FrameBuffer->getHandle< RsFrameBufferD3D12* >();
	delete FrameBufferInternal;
	FrameBuffer->setHandle< BcU64 >( 0 );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextD3D12::createBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	HRESULT RetVal = E_FAIL;

	const auto& BufferDesc = Buffer->getDesc();

	CD3DX12_HEAP_PROPERTIES HeapProperties( D3D12_HEAP_TYPE_DEFAULT );
	CD3DX12_RESOURCE_DESC ResourceDesc( CD3DX12_RESOURCE_DESC::Buffer( BufferDesc.SizeBytes_, D3D12_RESOURCE_FLAG_NONE) );

	// Determine appropriate resource usage.
	const auto Flags = BufferDesc.BindFlags_;

	// Allow flags.
	D3D12_RESOURCE_STATES ResourceUsage = D3D12_RESOURCE_STATE_COMMON;
	if( ( Flags & RsResourceBindFlags::VERTEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		ResourceUsage |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		ResourceUsage |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		//ResourceUsage |= D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( Flags & RsResourceBindFlags::INDEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		ResourceUsage |= D3D12_RESOURCE_STATE_INDEX_BUFFER;
		ResourceUsage |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		//ResourceUsage |= D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( Flags & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE )
	{
		ResourceUsage |= D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		ResourceUsage |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		//ResourceUsage |= D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( Flags & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		ResourceUsage |= D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		ResourceUsage |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		//ResourceUsage |= D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( Flags & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
	//	ResourceUsage |= D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		ResourceUsage |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		//ResourceUsage |= D3D12_RESOURCE_STATE_COPY_DEST;
	}
	if( ( Flags & RsResourceBindFlags::STREAM_OUTPUT ) != RsResourceBindFlags::NONE )
	{
		ResourceUsage |= D3D12_RESOURCE_STATE_STREAM_OUT;
		ResourceUsage |= D3D12_RESOURCE_STATE_COPY_SOURCE;
		//ResourceUsage |= D3D12_RESOURCE_STATE_COPY_DEST;
	}

	// Allow misc flags.
	D3D12_RESOURCE_FLAGS MiscFlag = D3D12_RESOURCE_FLAG_NONE;
	if( ( Flags & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if( ( Flags & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if( ( Flags & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	//ResourceDesc.Flags = MiscFlag;

	ComPtr< ID3D12Resource > D3DResource;
	RetVal = Device_->CreateCommittedResource( 
		&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		ResourceUsage,
		nullptr,
		IID_PPV_ARGS( D3DResource.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	Buffer->setHandle( new RsResourceD3D12( 
		D3DResource.Get(), 
		static_cast< D3D12_RESOURCE_STATES >( ResourceUsage | D3D12_RESOURCE_STATE_COPY_DEST ), 
		ResourceUsage ) );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextD3D12::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	auto Resource = Buffer->getHandle< RsResourceD3D12* >();

	DHCache_->destroyShaderResources( Buffer );

	// Check shader resource bindings.
	for( auto& ShaderResourceDesc : ShaderResourceDescs_ )
	{
		for( auto& SRBuffer : ShaderResourceDesc.Buffers_ )
		{
			if( SRBuffer == Buffer )
			{
				SRBuffer = nullptr;
			}
		}
	}

	// Check index buffer view.
	if( IndexBufferView_.BufferLocation == Resource->getGPUVirtualAddress() )
	{
		IndexBufferView_.BufferLocation = 0;
	}

	// Check vertex buffer views.
	for( auto& VertexBufferView : VertexBufferViews_ )
	{
		if( VertexBufferView.BufferLocation == Resource->getGPUVirtualAddress() )
		{
			VertexBufferView.BufferLocation = 0;
		}
	}	

	delete Resource;
	Buffer->setHandle< BcU64 >( 0 );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsContextD3D12::updateBuffer( 
	class RsBuffer* Buffer,
	BcSize Offset,
	BcSize Size,
	RsResourceUpdateFlags Flags,
	RsBufferUpdateFunc UpdateFunc )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto Resource = Buffer->getHandle< RsResourceD3D12* >();
	auto& D3DResource = Resource->getInternalResource();
	auto CommandList = getCurrentCommandList();
	auto UploadAllocator = getCurrentUploadAllocator();

	// Allocate for upload.
	auto Allocation = UploadAllocator->allocate( Size );
	RsBufferLock Lock;
	Lock.Buffer_ = Allocation.Address_;
	UpdateFunc( Buffer, Lock );

	// Transition to copy dest.
	auto OldUsage = Resource->resourceBarrierTransition( CommandList, D3D12_RESOURCE_STATE_COPY_DEST );

	// Copy into buffer.
	CommandList->CopyBufferRegion( 
		D3DResource.Get(), Offset, Allocation.BaseResource_.Get(),
		Allocation.OffsetInBaseResource_, Size );

	// Transition back to original.
	Resource->resourceBarrierTransition( CommandList, OldUsage );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextD3D12::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	HRESULT RetVal = E_FAIL;

	const auto& TextureDesc = Texture->getDesc();

	D3D12_RESOURCE_FLAGS MiscFlag = D3D12_RESOURCE_FLAG_NONE;

	// Allow misc flags.
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
	}
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	}
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	}

	// Deny misc flags.
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) == RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
	}
	
	// TODO: Improve heap determination. Should always be default going forward.
	D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT;
	
	CD3DX12_HEAP_PROPERTIES HeapProperties( HeapType );
	CD3DX12_RESOURCE_DESC ResourceDesc;
	
	const auto& Format = RsUtilsD3D12::GetTextureFormat( TextureDesc.Format_ );

	switch( TextureDesc.Type_ )
	{
	case RsTextureType::TEX1D:
		ResourceDesc = CD3DX12_RESOURCE_DESC::Tex1D( 
			Format.RTVFormat_,
			TextureDesc.Width_, 1, (BcU16)TextureDesc.Levels_, 
			MiscFlag );
		break;

	case RsTextureType::TEX2D:
		ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( 
			Format.RTVFormat_,
			TextureDesc.Width_, TextureDesc.Height_, 1, (BcU16)TextureDesc.Levels_, 1, 0,
			MiscFlag );
		break;

	case RsTextureType::TEX3D:
		ResourceDesc = CD3DX12_RESOURCE_DESC::Tex3D( 
			Format.RTVFormat_,
			TextureDesc.Width_, TextureDesc.Height_, (BcU16)TextureDesc.Depth_, (BcU16)TextureDesc.Levels_, 
			MiscFlag );
		break;

	case RsTextureType::TEXCUBE:
		ResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D( 
			Format.RTVFormat_,
			TextureDesc.Width_, TextureDesc.Height_, 6, (BcU16)TextureDesc.Levels_, 1, 0,
			MiscFlag );
		break;
	}

	// Clear value.
	D3D12_CLEAR_VALUE ClearValue;
	D3D12_CLEAR_VALUE* SetClearValue = nullptr;
	BcMemZero( &ClearValue, sizeof( ClearValue ) );

	// Setup initial bind type to be whatever is likely what it will be used as first.
	BcU32 InitialUsage = D3D12_RESOURCE_STATE_COMMON;
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		ClearValue.Format = Format.RTVFormat_;
		SetClearValue = &ClearValue;
		InitialUsage = D3D12_RESOURCE_STATE_RENDER_TARGET;
	}
	else if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		ClearValue.Format = Format.DSVFormat_;
		ClearValue.DepthStencil.Depth = 1.0f;
		SetClearValue = &ClearValue;
		InitialUsage = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	}
	else if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		InitialUsage = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	}

	// TODO: Change this to use CreatePlacedResource, and appropriate heaps.
	// Should have a single large upload heap, an upload command list,
	// treat the heap as a circular buffer and fence to ensure we don't overwrite.
	ComPtr< ID3D12Resource > D3DResource;
	RetVal = Device_->CreateCommittedResource( 
		&HeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		static_cast< D3D12_RESOURCE_STATES >( InitialUsage ),
		SetClearValue,
		IID_PPV_ARGS( D3DResource.GetAddressOf() ) );
 	BcAssert( SUCCEEDED( RetVal ) );

	Texture->setHandle( new RsResourceD3D12( 
		D3DResource.Get(), 
		RsUtilsD3D12::GetResourceUsage( TextureDesc.BindFlags_ ),
		static_cast< D3D12_RESOURCE_STATES >( InitialUsage ) ) );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextD3D12::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto Resource = Texture->getHandle< RsResourceD3D12* >();

	DHCache_->destroyShaderResources( Texture );

	// Check shader resource bindings.
	for( auto& ShaderResourceDesc : ShaderResourceDescs_ )
	{
		for( auto& SRTexture : ShaderResourceDesc.Textures_ )
		{
			if( SRTexture == Texture )
			{
				SRTexture = nullptr;
			}
		}
	}

	delete Resource;
	Texture->setHandle< BcU64 >( 0 );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
bool RsContextD3D12::updateTexture( 
	class RsTexture* Texture,
	const struct RsTextureSlice& Slice,
	RsResourceUpdateFlags Flags,
	RsTextureUpdateFunc UpdateFunc )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto CommandList = getCurrentCommandList();
	auto UploadAllocator = getCurrentUploadAllocator();

	const auto TextureDesc = Texture->getDesc();
	RsResourceD3D12* Resource = Texture->getHandle< RsResourceD3D12* >();
	ID3D12Resource* D3DResource = Resource->getInternalResource().Get();
	auto D3DResourceDesc = D3DResource->GetDesc();

	BcU32 SubResource = Slice.Level_;
	
	// Calculate cubemap subresource.
	if( TextureDesc.Type_ == RsTextureType::TEXCUBE )
	{
		BcAssert( Slice.Face_ != RsTextureFace::NONE );
		SubResource = Slice.Level_ * 6 + ( (BcU32)Slice.Face_ - 1 );
	}

	// Setup pitched subresource to match source data.
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT Layout;
	UINT NumRows = 0;
	UINT64 TotalBytes = 0;
	Device_->GetCopyableFootprints( &D3DResourceDesc, SubResource, 1, 0, &Layout, &NumRows, nullptr, &TotalBytes );

	// Update texture.
	auto Allocation = UploadAllocator->allocate( TotalBytes, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT );
	RsTextureLock Lock;
	Layout.Offset = Allocation.OffsetInBaseResource_;
	Lock.Buffer_ = Allocation.Address_;
	Lock.Pitch_ = static_cast< BcU32 >( Layout.Footprint.RowPitch );
	Lock.SlicePitch_ = Lock.Pitch_ * NumRows;
	
	UpdateFunc( Texture, Lock );

	// Transition to copy dest.
	auto OldUsage = Resource->resourceBarrierTransition( CommandList, D3D12_RESOURCE_STATE_COPY_DEST );

	// Copy in.
	CD3DX12_TEXTURE_COPY_LOCATION Dst( D3DResource, SubResource );
	CD3DX12_TEXTURE_COPY_LOCATION Src( Allocation.BaseResource_.Get(), Layout );
	CommandList->CopyTextureRegion( &Dst, 0, 0, 0, &Src, nullptr );

	// Transition back to original.
	Resource->resourceBarrierTransition( CommandList, OldUsage );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextD3D12::createShader(
	class RsShader* Shader )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextD3D12::destroyShader(
	class RsShader* Shader )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextD3D12::createProgram(
	class RsProgram* Program )
{
	Program->setHandle( new RsProgramD3D12( Program, Device_.Get() ) );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextD3D12::destroyProgram(
	class RsProgram* Program )
{
	PSOCache_->destroyResources(
		[ this, Program ]( const RsGraphicsPipelineStateDescD3D12& PSODesc, ID3D12PipelineState* PSO )->bool
		{
			return PSODesc.Program_ == Program;
		} );

	if( GraphicsPSODesc_.Program_ == Program )
	{
		GraphicsPSODesc_.Program_ = nullptr;
	}

	auto ProgramD3D12 = Program->getHandle< RsProgramD3D12* >();
	delete ProgramD3D12;
	Program->setHandle< BcU64 >( 0 );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgramBinding
bool RsContextD3D12::createProgramBinding( class RsProgramBinding* ProgramBinding )
{
	PSY_PROFILE_FUNCTION;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgramBinding
bool RsContextD3D12::destroyProgramBinding( class RsProgramBinding* ProgramBinding )
{
	PSY_PROFILE_FUNCTION;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
bool RsContextD3D12::createVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyVertexDeclaration
bool RsContextD3D12::destroyVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration )
{
	PSOCache_->destroyResources(
		[ this, VertexDeclaration ]( const RsGraphicsPipelineStateDescD3D12& PSODesc, ID3D12PipelineState* PSO )->bool
		{
			return PSODesc.VertexDeclaration_ == VertexDeclaration;
		} );

	if( GraphicsPSODesc_.VertexDeclaration_ == VertexDeclaration )
	{
		GraphicsPSODesc_.VertexDeclaration_ = nullptr;
	}
	return true;
}
	
//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D12::flushState()
{
	PSY_PROFILE_FUNCTION;
	auto CommandList = getCurrentCommandList();

	// Graphics root signature.
	CommandList->SetGraphicsRootSignature( DefaultRootSignature_.Get() );

	// Frame buffer.
	BcAssert( FrameBuffer_ );

	// Setup the formats for the pipeline state object.
	const auto FrameBufferDesc = FrameBuffer_->getDesc();
	GraphicsPSODesc_.FrameBufferFormatDesc_.NumRenderTargets_ = FrameBufferDesc.RenderTargets_.size();
	GraphicsPSODesc_.FrameBufferFormatDesc_.RTVFormats_.fill( RsTextureFormat::UNKNOWN );
	for( size_t Idx = 0; Idx < FrameBufferDesc.RenderTargets_.size(); ++Idx )
	{
		auto RenderTarget = FrameBufferDesc.RenderTargets_[ Idx ];
		if( RenderTarget != nullptr )
		{
			const auto& RenderTargetDesc =  RenderTarget->getDesc();
			GraphicsPSODesc_.FrameBufferFormatDesc_.RTVFormats_[ Idx ] = RenderTargetDesc.Format_;
		}
	}

	if( FrameBufferDesc.DepthStencilTarget_ != nullptr )
	{
		auto DepthStencil = FrameBufferDesc.DepthStencilTarget_;
		const auto& DepthStencilDesc = DepthStencil->getDesc();
		GraphicsPSODesc_.FrameBufferFormatDesc_.DSVFormat_ = DepthStencilDesc.Format_;
	}
	else
	{
		GraphicsPSODesc_.FrameBufferFormatDesc_.DSVFormat_ = RsTextureFormat::UNKNOWN;
	}

	// Set render targets on command list.
	auto FrameBufferD3D12 = FrameBuffer_->getHandle< RsFrameBufferD3D12* >();
	FrameBufferD3D12->setRenderTargets( CommandList );
		   
	// Get current pipeline state.
	ID3D12PipelineState* GraphicsPS = nullptr;
	GraphicsPS = PSOCache_->getPipelineState( GraphicsPSODesc_, DefaultRootSignature_.Get() );

	// If null, just use default (bad)
	if( GraphicsPS == nullptr )
	{
		GraphicsPS = DefaultPSO_.Get();
	}

	// Reset command list if we need to, otherwise just set new pipeline state.
	if( GraphicsPS != nullptr )
	{
		CommandList->SetPipelineState( GraphicsPS );
	}

	// Assert resource usage.
#if PSY_DEBUG
	BcU32 ShaderType = 0;
	for( const auto& ShaderResourceDesc : ShaderResourceDescs_ )
	{
		for( auto* Texture : ShaderResourceDesc.Textures_ )
		{
			if( Texture != nullptr )
			{
				auto Resource = Texture->getHandle< RsResourceD3D12* >();
				BcAssert( 
					( static_cast< RsShaderType >( ShaderType ) == RsShaderType::PIXEL && 
						( Resource->resourceUsage() & D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE ) != 0 ) ||
					( static_cast< RsShaderType >( ShaderType ) != RsShaderType::PIXEL && 
						( Resource->resourceUsage() & D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE ) != 0 ) );
			}
		}

		for( auto* Buffer : ShaderResourceDesc.Buffers_ )
		{
			if( Buffer != nullptr )
			{
				auto Resource = Buffer->getHandle< RsResourceD3D12* >();
				BcAssert( ( Resource->resourceUsage() & D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER ) != 0 );
			}
		}
		++ShaderType;
	} 
#endif

	// Get descriptor sets from cache.
	std::array< ID3D12DescriptorHeap*, 2 > DescriptorHeaps;
	DescriptorHeaps[ 0 ] = DHCache_->getSamplersDescriptorHeap( SamplerStateDescs_ );
	DescriptorHeaps[ 1 ] = DHCache_->getShaderResourceDescriptorHeap( ShaderResourceDescs_ );

	CommandList->SetDescriptorHeaps( static_cast< UINT >( DescriptorHeaps.size() ), DescriptorHeaps.data() );

	// Set the descriptor tables.
	CD3DX12_GPU_DESCRIPTOR_HANDLE BaseSamplerDHHandle( DescriptorHeaps[0]->GetGPUDescriptorHandleForHeapStart() );
	CD3DX12_GPU_DESCRIPTOR_HANDLE BaseShaderResourceDHHandle( DescriptorHeaps[1]->GetGPUDescriptorHandleForHeapStart() );
	auto SamplerDescriptorSize = Device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	auto ShaderResourceDescriptorSize = Device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	UINT RootDescirptorIdx = 0;
	for( INT ShaderIdx = 0; ShaderIdx < 5; ++ShaderIdx )
	{
		// Samplers.
		CommandList->SetGraphicsRootDescriptorTable( 
			RootDescirptorIdx++, BaseSamplerDHHandle );
		BaseSamplerDHHandle.Offset( RsDescriptorHeapSamplerStateDescD3D12::MAX_SAMPLERS, SamplerDescriptorSize );

		// Shader resource views.
		CommandList->SetGraphicsRootDescriptorTable( 
			RootDescirptorIdx++, BaseShaderResourceDHHandle );
		BaseShaderResourceDHHandle.Offset( RsDescriptorHeapShaderResourceDescD3D12::MAX_SRVS, ShaderResourceDescriptorSize );

		// Constant buffer views.
		CommandList->SetGraphicsRootDescriptorTable( 
			RootDescirptorIdx++, BaseShaderResourceDHHandle );
		BaseShaderResourceDHHandle.Offset( RsDescriptorHeapShaderResourceDescD3D12::MAX_CBVS, ShaderResourceDescriptorSize );
	}

	// Setup primitive, index buffer, and vertex buffers.
	if( GraphicsPSODesc_.Topology_ != RsTopologyType::INVALID )
	{
		CommandList->IASetPrimitiveTopology( RsUtilsD3D12::GetPrimitiveTopology( GraphicsPSODesc_.Topology_ ) );
	}

	if( IndexBufferView_.BufferLocation != 0 )
	{
		CommandList->IASetIndexBuffer( &IndexBufferView_ );
	}

	for( UINT Idx = 0; Idx < VertexBufferViews_.size(); ++Idx )
	{
		if( VertexBufferViews_[ Idx ].BufferLocation != 0 )
		{
			CommandList->IASetVertexBuffers( Idx, 1, &VertexBufferViews_[ Idx ] ); 
		}
	}

	// Setup viewport.
	CommandList->RSSetViewports( static_cast< UINT >( Viewports_.size() ), Viewports_.data() );
	CommandList->RSSetScissorRects( static_cast< UINT >( ScissorRects_.size() ), ScissorRects_.data() );
}

//////////////////////////////////////////////////////////////////////////
// flushCommandList
void RsContextD3D12::flushCommandList( std::function< void() > PostExecute )
{
	PSY_PROFILE_FUNCTION;
	HRESULT RetVal = E_FAIL;

	// Close current list and execute.
	if( CurrentCommandListData_ >= 0 )
	{
		auto& CommandListData = CommandListDatas_[ CurrentCommandListData_ ];

		// Close command list.
		RetVal = CommandListData.CommandList_->Close();
		BcAssert( SUCCEEDED( RetVal ) );

		// Execute command list.
		ID3D12CommandList* CommandLists[] = { CommandListData.CommandList_.Get() };
		CommandQueue_->ExecuteCommandLists( 1, CommandLists );

		if( PostExecute )
		{
			PostExecute();
		}

		// Signal next.
		RetVal = CommandQueue_->Signal( CommandListData.CompleteFence_.Get(), ++CommandListData.CompletionValue_ );
		BcAssert( SUCCEEDED( RetVal ) );
	}

	// Advance current command list.
	CurrentCommandListData_ = ( CurrentCommandListData_ + 1 ) % CommandListDatas_.size();

	// Reset next command list.
	{
		const auto& CommandListData = CommandListDatas_[ CurrentCommandListData_ ];

		// Wait for completion. Shouldn't stall, if it does there may be a need
		// to increase the number of command lists.
		if( CommandListData.CompleteFence_->GetCompletedValue() < CommandListData.CompletionValue_ )
		{
			PSY_PROFILER_SECTION( Section, "Waiting on fence..." );
			RetVal = CommandListData.CompleteFence_->SetEventOnCompletion( CommandListData.CompletionValue_, WaitOnCommandListEvent_ );
			BcAssert( SUCCEEDED( RetVal ) );
			::WaitForSingleObject( WaitOnCommandListEvent_, INFINITE );
		}

		// Reset allocator and command list.
		RetVal = CommandListData.CommandAllocator_->Reset();
		BcAssert( SUCCEEDED( RetVal ) );

		RetVal = CommandListData.CommandList_->Reset( CommandListData.CommandAllocator_.Get(), DefaultPSO_.Get() );
		BcAssert( SUCCEEDED( RetVal ) );

		// Reset allocator.
		CommandListData.UploadAllocator_->reset();
	}
}

//////////////////////////////////////////////////////////////////////////
// createDefaultRootSignature
void RsContextD3D12::createDefaultRootSignature()
{
	HRESULT RetVal = E_FAIL;
	ComPtr< ID3DBlob > OutBlob, ErrorBlob;

	std::array< CD3DX12_DESCRIPTOR_RANGE, 3 > DescriptorRanges;
	DescriptorRanges[0].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, RsDescriptorHeapSamplerStateDescD3D12::MAX_SAMPLERS, 0 );
	DescriptorRanges[1].Init( D3D12_DESCRIPTOR_RANGE_TYPE_SRV, RsDescriptorHeapShaderResourceDescD3D12::MAX_SRVS, 0 );
	DescriptorRanges[2].Init( D3D12_DESCRIPTOR_RANGE_TYPE_CBV, RsDescriptorHeapShaderResourceDescD3D12::MAX_CBVS, 0 );

	std::array< CD3DX12_ROOT_PARAMETER, 15 > Parameters;
	Parameters[0].InitAsDescriptorTable( 1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_VERTEX );
	Parameters[1].InitAsDescriptorTable( 1, &DescriptorRanges[1], D3D12_SHADER_VISIBILITY_VERTEX );
	Parameters[2].InitAsDescriptorTable( 1, &DescriptorRanges[2], D3D12_SHADER_VISIBILITY_VERTEX );

	Parameters[3].InitAsDescriptorTable( 1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL );
	Parameters[4].InitAsDescriptorTable( 1, &DescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL );
	Parameters[5].InitAsDescriptorTable( 1, &DescriptorRanges[2], D3D12_SHADER_VISIBILITY_PIXEL );

	Parameters[6].InitAsDescriptorTable( 1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_HULL );
	Parameters[7].InitAsDescriptorTable( 1, &DescriptorRanges[1], D3D12_SHADER_VISIBILITY_HULL );
	Parameters[8].InitAsDescriptorTable( 1, &DescriptorRanges[2], D3D12_SHADER_VISIBILITY_HULL );

	Parameters[9].InitAsDescriptorTable( 1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_DOMAIN );
	Parameters[10].InitAsDescriptorTable( 1, &DescriptorRanges[1], D3D12_SHADER_VISIBILITY_DOMAIN );
	Parameters[11].InitAsDescriptorTable( 1, &DescriptorRanges[2], D3D12_SHADER_VISIBILITY_DOMAIN );

	Parameters[12].InitAsDescriptorTable( 1, &DescriptorRanges[0], D3D12_SHADER_VISIBILITY_GEOMETRY );
	Parameters[13].InitAsDescriptorTable( 1, &DescriptorRanges[1], D3D12_SHADER_VISIBILITY_GEOMETRY );
	Parameters[14].InitAsDescriptorTable( 1, &DescriptorRanges[2], D3D12_SHADER_VISIBILITY_GEOMETRY );
	
	CD3DX12_ROOT_SIGNATURE_DESC RootSignatureDesc;
	RootSignatureDesc.Init(
		static_cast< UINT >( Parameters.size() ), Parameters.data(), 
		0, nullptr, 
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );
	RetVal = D3D12SerializeRootSignature( &RootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, OutBlob.GetAddressOf(), ErrorBlob.GetAddressOf() );
	if( FAILED( RetVal ) )
	{
		const void* BufferData = ErrorBlob->GetBufferPointer();
		PSY_LOG( reinterpret_cast< const char * >( BufferData ) );
		BcBreakpoint;
	}
	RetVal = Device_->CreateRootSignature( 0, OutBlob->GetBufferPointer(), OutBlob->GetBufferSize(), IID_PPV_ARGS( DefaultRootSignature_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
}

//////////////////////////////////////////////////////////////////////////
// createDefaultPSO
void RsContextD3D12::createDefaultPSO()
{
	HRESULT RetVal = E_FAIL;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC DefaultPSO = {};
	BcMemZero( &DefaultPSO, sizeof( DefaultPSO ) );

	D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
	DefaultPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	DefaultPSO.InputLayout.NumElements = ARRAYSIZE( InputElementDescs );
	DefaultPSO.InputLayout.pInputElementDescs = InputElementDescs;
	DefaultPSO.VS.pShaderBytecode = g_VShader;
	DefaultPSO.VS.BytecodeLength = ARRAYSIZE( g_VShader );
	DefaultPSO.PS.pShaderBytecode = g_PShader;
	DefaultPSO.PS.BytecodeLength = ARRAYSIZE( g_PShader );
	DefaultPSO.pRootSignature = DefaultRootSignature_.Get();
	DefaultPSO.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	DefaultPSO.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	DefaultPSO.NumRenderTargets = 1;

	DefaultPSO.SampleDesc = SwapChainDesc_.SampleDesc;
	DefaultPSO.RTVFormats[ 0 ] = SwapChainDesc_.BufferDesc.Format;

	RetVal = Device_->CreateGraphicsPipelineState( &DefaultPSO, 
		IID_PPV_ARGS( DefaultPSO_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
}

//////////////////////////////////////////////////////////////////////////
// createCommandListData
void RsContextD3D12::createCommandListData( size_t NoofBuffers )
{
	CommandListDatas_.resize( NoofBuffers );
	for( auto& CommandListData : CommandListDatas_ )
	{
		HRESULT RetVal = E_FAIL;

		// Command allocator.
		RetVal = Device_->CreateCommandAllocator( 
			D3D12_COMMAND_LIST_TYPE_DIRECT, 
			IID_PPV_ARGS( CommandListData.CommandAllocator_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );

		// Command list.
		RetVal = Device_->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
			CommandListData.CommandAllocator_.Get(), DefaultPSO_.Get(), 
			IID_PPV_ARGS( CommandListData.CommandList_.GetAddressOf() ) );
		BcAssert( SUCCEEDED( RetVal ) );

		// Upload allocator.
		CommandListData.UploadAllocator_.reset(
			new RsLinearHeapAllocatorD3D12( Device_.Get(), D3D12_HEAP_TYPE_UPLOAD, 64 * 1024 ) );

		// Complete fence.
		CommandListData.CompletionValue_ = 0;
		Device_->CreateFence( 
			CommandListData.CompletionValue_, D3D12_FENCE_FLAG_NONE, 
			IID_PPV_ARGS( CommandListData.CompleteFence_.GetAddressOf() ) );

		// Close list.
		RetVal = CommandListData.CommandList_->Close();
		BcAssert( SUCCEEDED( RetVal ) );
	}

	WaitOnCommandListEvent_ = ::CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );
}

//////////////////////////////////////////////////////////////////////////
// recreateBackBuffers
void RsContextD3D12::recreateBackBuffers( BcU32 Width, BcU32 Height )
{
	if( Width_ != Width || Height_ != Height )
	{
		// Flush command list in flight before recreating back buffer.
		for( BcU32 Idx = 0; Idx < SwapChainDesc_.BufferCount; ++Idx )
		{
			flushCommandList( nullptr );
		}

		// Destroy backbuffer.
		for( auto BackBufferFB : BackBufferFB_ )
		{
			destroyFrameBuffer( BackBufferFB );
		}

		for( auto BackBufferRT : BackBufferRT_ )
		{
			destroyTexture( BackBufferRT );
		}

		if( BackBufferDS_ != nullptr )
		{
			destroyTexture( BackBufferDS_ );
		}

		// Resize.
		BackBufferFB_.resize( SwapChainDesc_.BufferCount );
		BackBufferRT_.resize( SwapChainDesc_.BufferCount );

		// Resize buffers.
		SwapChain_->ResizeBuffers( 
			SwapChainDesc_.BufferCount, 
			Width, Height, 
			SwapChainDesc_.BufferDesc.Format, 
			SwapChainDesc_.Flags );

		// Create depth stencil.
		{
			RsTextureDesc Desc;
			Desc.Type_ = RsTextureType::TEX2D;
			Desc.CreationFlags_  = RsResourceCreationFlags::NONE;
			Desc.Levels_ = 1;
			Desc.Width_ = Width;
			Desc.Height_ = Height;
			Desc.Depth_= 1;

			// Depth stencil.
			Desc.BindFlags_ = RsResourceBindFlags::DEPTH_STENCIL;
			Desc.Format_ = RsTextureFormat::D24S8;
			BackBufferDS_ = new RsTexture( this, Desc );
			auto RetVal = createTexture( BackBufferDS_ );
			BcAssert( RetVal );
		}

		// Create RT + FBs.
		for( BcU32 Idx = 0; Idx < SwapChainDesc_.BufferCount; ++Idx )
		{
			RsTextureDesc Desc;
			Desc.Type_ = RsTextureType::TEX2D;
			Desc.CreationFlags_  = RsResourceCreationFlags::NONE;
			Desc.Levels_ = 1;
			Desc.Width_ = Width;
			Desc.Height_ = Height;
			Desc.Depth_= 1;

			// Render target.
			Desc.BindFlags_ = RsResourceBindFlags::RENDER_TARGET | RsResourceBindFlags::PRESENT | RsResourceBindFlags::SHADER_RESOURCE;
			Desc.Format_ = RsTextureFormat::R8G8B8A8;
			BackBufferRT_[ Idx ] = new RsTexture( this, Desc );

			ComPtr< ID3D12Resource > BackBufferResource;
			auto RetVal = SwapChain_->GetBuffer( Idx, IID_PPV_ARGS( BackBufferResource.GetAddressOf() ));
			BcAssert( SUCCEEDED( RetVal ) );
		
			auto BackBufferRTResource = new RsResourceD3D12( BackBufferResource.Get(), 
				RsUtilsD3D12::GetResourceUsage( Desc.BindFlags_ ),
				D3D12_RESOURCE_STATE_PRESENT );
			BackBufferRT_[ Idx ]->setHandle( BackBufferRTResource );

			RsFrameBufferDesc FBDesc = RsFrameBufferDesc( 1 ).setRenderTarget( 0, BackBufferRT_[ Idx ] );

			FBDesc.setDepthStencilTarget( BackBufferDS_ );

			BackBufferFB_[ Idx ] = new RsFrameBuffer( this, FBDesc );
			RetVal = createFrameBuffer( BackBufferFB_[ Idx ] );
			BcAssert( RetVal );
		}
	}

	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// getCurrentCommandList
ID3D12GraphicsCommandList* RsContextD3D12::getCurrentCommandList()
{
	return CommandListDatas_[ CurrentCommandListData_ ].CommandList_.Get();
}

//////////////////////////////////////////////////////////////////////////
// getCurrentUploadAllocator
RsLinearHeapAllocatorD3D12* RsContextD3D12::getCurrentUploadAllocator()
{
	return CommandListDatas_[ CurrentCommandListData_ ].UploadAllocator_.get();
}
