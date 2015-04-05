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


#include "Base/BcMath.h"

#include "System/Os/OsClient.h"
#include "System/Os/OsClientWindows.h"

#include "Import/Img/Img.h"

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
	Adapter_(),
	CommandQueueDesc_(),
	CommandQueue_(),
	CommandAllocator_(),
	CommandList_(),
	PresentFence_(),
	FrameCounter_( 0 ),
	FlushCounter_( 1 ),
	NumSwapBuffers_( 1 ),
	LastSwapBuffer_( 0 ),
	BackBufferRT_( nullptr ),
	BackBufferDS_( nullptr ),
	BackBufferFB_( nullptr ),
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
// getWidth
//virtual
BcU32 RsContextD3D12::getWidth() const
{
	return Client_->getWidth();
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextD3D12::getHeight() const
{
	return Client_->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextD3D12::getClient() const
{
	return Client_;
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextD3D12::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	switch( CodeType )
	{
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
// presentBackBuffer
void RsContextD3D12::presentBackBuffer()
{
	HRESULT RetVal = E_FAIL;

	// Transition back buffer to present.
	RsResourceD3D12* BackBufferResource = BackBufferRT_->getHandle< RsResourceD3D12* >();
	BackBufferResource->resourceBarrierTransition( CommandList_.Get(), RsResourceBindFlags::PRESENT );
	
	// Flush command list (also waits for completion).
	flushCommandList();

	// Do present.
	RetVal = SwapChain_->Present( 1, 0 );
	BcAssert( SUCCEEDED( RetVal ) );
	++FrameCounter_;

	// Get next buffer.
	LastSwapBuffer_ = ( 1 + LastSwapBuffer_ ) % NumSwapBuffers_;

	// Prep for next frame.
	recreateBackBuffer();
	
	// Back to default frame buffer.
	setFrameBuffer( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextD3D12::takeScreenshot()
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextD3D12::setViewport( class RsViewport& Viewport )
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
// create
void RsContextD3D12::create()
{
	OsClientWindows* Client = dynamic_cast< OsClientWindows* >( Client_ );
	BcAssertMsg( Client != nullptr, "Windows client is not being used!" );
	HRESULT RetVal = E_FAIL;

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	// Create DXGI factory.
	RetVal = ::CreateDXGIFactory1( IID_PPV_ARGS( &Factory_ ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Create default device.
#ifdef _DEBUG
	const D3D12_CREATE_DEVICE_FLAG DeviceFlags = D3D12_CREATE_DEVICE_DEBUG;
#else
	const D3D12_CREATE_DEVICE_FLAG DeviceFlags = D3D12_CREATE_DEVICE_NONE;
#endif
	FeatureLevel_ = D3D_FEATURE_LEVEL_11_0;
	RetVal = D3D12CreateDevice(
		Adapter_.Get(),
		D3D_DRIVER_TYPE_HARDWARE,
		DeviceFlags,
		FeatureLevel_, 
		D3D12_SDK_VERSION,
		IID_PPV_ARGS( &Device_ ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Create command queue.
	BcMemZero( &CommandQueueDesc_, sizeof( CommandQueueDesc_ ) );
	CommandQueueDesc_.Flags = D3D12_COMMAND_QUEUE_NONE;
	CommandQueueDesc_.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	RetVal = Device_->CreateCommandQueue( &CommandQueueDesc_, IID_PPV_ARGS( &CommandQueue_ ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Setup swap chain desc.
	BcMemZero( &SwapChainDesc_, sizeof( SwapChainDesc_ ) );
    SwapChainDesc_.BufferCount = NumSwapBuffers_;
	SwapChainDesc_.BufferDesc.Width = Client->getWidth();
    SwapChainDesc_.BufferDesc.Height = Client->getHeight();
    SwapChainDesc_.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc_.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc_.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc_.OutputWindow = Client->getHWND();
	SwapChainDesc_.SampleDesc.Count = 1;
	SwapChainDesc_.SampleDesc.Quality = 0;
    SwapChainDesc_.Windowed = TRUE;
	SwapChainDesc_.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	SwapChainDesc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	RetVal = Factory_->CreateSwapChain( CommandQueue_.Get(), &SwapChainDesc_, &SwapChain_ );
	BcAssert( SUCCEEDED( RetVal ) );

	// Create present fence.
	FrameCounter_ = 0;
	RetVal = Device_->CreateFence( FrameCounter_, D3D12_FENCE_MISC_NONE, IID_PPV_ARGS( PresentFence_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
	PresentEvent_ = ::CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );

	// Create pipeline state cache.
	PSOCache_.reset( new RsPipelineStateCacheD3D12( Device_.Get() ) );

	// Create descriptor heap cache.
	DHCache_.reset( new RsDescriptorHeapCacheD3D12( Device_.Get() ) );

	// Create default root signature.
	createDefaultRootSignature();

	// Create default pipeline state.
	createDefaultPSO();

	// Create command allocators.
	createCommandAllocators();

	// Create command lists.
	createCommandLists();

	// Recreate backbuffer.
	recreateBackBuffer();

	// Default state.
	setDefaultState();

	// Default frame buffer.
	setFrameBuffer( nullptr );
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
	if( BackBufferFB_ != nullptr )
	{
		destroyFrameBuffer( BackBufferFB_ );
	}

	if( BackBufferRT_ != nullptr )
	{
		destroyTexture( BackBufferRT_ );
	}

	if( BackBufferDS_ != nullptr )
	{
		destroyTexture( BackBufferDS_ );
	}

	// Cleanup everything.
	DefaultRootSignature_.Reset();
	DefaultPSO_.Reset();
	DHCache_.reset();
	PSOCache_.reset();
	::CloseHandle( PresentEvent_ );
	PresentFence_.Reset();
	CommandList_.Reset();
	CommandAllocator_.Reset();
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
	
	if( IndexBuffer != nullptr )
	{
		auto VertexBufferResource = IndexBuffer->getHandle< RsResourceD3D12* >();
		IndexBufferView_.BufferLocation = VertexBufferResource->getInternalResource()->GetGPUVirtualAddress();
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

	if( VertexBuffer != nullptr )
	{
		auto VertexBufferResource = VertexBuffer->getHandle< RsResourceD3D12* >();
		VertexBufferView.BufferLocation = VertexBufferResource->getInternalResource()->GetGPUVirtualAddress();
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

	// Even if null, we want backbuffer bound.
	if( FrameBuffer == nullptr )
	{
		FrameBuffer_ = BackBufferFB_;
	}
	else
	{
		FrameBuffer_ = FrameBuffer;
	}
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextD3D12::clear( 
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	flushState();
	BcAssert( FrameBuffer_ );
	RsFrameBufferD3D12* FrameBufferD3D12 = FrameBuffer_->getHandle< RsFrameBufferD3D12* >();
	FrameBufferD3D12->clear( CommandList_.Get(), Colour, EnableClearColour, EnableClearDepth, EnableClearStencil );
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextD3D12::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	GraphicsPSODesc_.Topology_ = PrimitiveType;
	flushState();

	CommandList_->DrawInstanced( NoofIndices, 1, IndexOffset, 0 );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextD3D12::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	GraphicsPSODesc_.Topology_ = PrimitiveType;
	flushState();
	
	CommandList_->DrawIndexedInstanced( NoofIndices, 1, IndexOffset, VertexOffset, 0 );
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

	// TODO: Change this to use CreatePlacedResource, and appropriate heaps.
	// Should have a single large upload heap, an upload command list,
	// treat the heap as a circular buffer and fence to ensure we don't overwrite.
	CD3D12_HEAP_PROPERTIES HeapProperties( D3D12_HEAP_TYPE_UPLOAD );
	CD3D12_RESOURCE_DESC ResourceDesc( CD3D12_RESOURCE_DESC::Buffer( BufferDesc.SizeBytes_, D3D12_RESOURCE_MISC_NONE ) );

	ComPtr< ID3D12Resource > D3DResource;
	RetVal = Device_->CreateCommittedResource( 
		&HeapProperties,
		D3D12_HEAP_MISC_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_USAGE_GENERIC_READ, 
		nullptr,
		IID_PPV_ARGS( D3DResource.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	RsResourceBindFlags BindFlags = RsResourceBindFlags::NONE;
	switch( BufferDesc.Type_ )
	{
	case RsBufferType::INDEX:
		BindFlags = RsResourceBindFlags::VERTEX_BUFFER;
		break;
	case RsBufferType::VERTEX:
		BindFlags = RsResourceBindFlags::INDEX_BUFFER;
		break;
	case RsBufferType::UNIFORM:
		BindFlags = RsResourceBindFlags::UNIFORM_BUFFER;
		break;
	case RsBufferType::UNORDERED_ACCESS:
		BindFlags = RsResourceBindFlags::UNORDERED_ACCESS;
		break;
	case RsBufferType::STREAM_OUT:
		BindFlags = RsResourceBindFlags::STREAM_OUTPUT;
		break;
	default:
		BcBreakpoint;
	}

	Buffer->setHandle( new RsResourceD3D12( D3DResource.Get(), BindFlags, BindFlags ) );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextD3D12::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto Resource = Buffer->getHandle< RsResourceD3D12* >();
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
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	BcUnusedVar( Offset );
	BcUnusedVar( Size );
	BcUnusedVar( Flags );
	auto Resource = Buffer->getHandle< RsResourceD3D12* >();
	auto& D3DResource = Resource->getInternalResource();

	D3D12_RANGE Range = { Offset, Offset + Size };
	RsBufferLock Lock;
	if( SUCCEEDED( D3DResource->Map( 0, &Range, &Lock.Buffer_ ) ) )
	{
		UpdateFunc( Buffer, Lock );
		D3DResource->Unmap( 0, &Range );
	}
	else
	{
		BcBreakpoint;
		return false;
	}

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

	D3D12_RESOURCE_MISC_FLAG MiscFlag = D3D12_RESOURCE_MISC_NONE;

	// Allow misc flags.
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_MISC_ALLOW_RENDER_TARGET;
	}
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_MISC_ALLOW_DEPTH_STENCIL;
	}
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_MISC_ALLOW_UNORDERED_ACCESS;
	}

	// Deny misc flags.
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) == RsResourceBindFlags::NONE )
	{
		MiscFlag |= D3D12_RESOURCE_MISC_DENY_SHADER_RESOURCE;
	}
	
	// TODO: Improve heap determination. Should always be default going forward.
	D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT;
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		HeapType = D3D12_HEAP_TYPE_DEFAULT;		
	}
	else if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		HeapType = D3D12_HEAP_TYPE_DEFAULT;		
	}
	else if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::PRESENT ) != RsResourceBindFlags::NONE )
	{
		HeapType = D3D12_HEAP_TYPE_DEFAULT;		
	}
	else if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		HeapType = D3D12_HEAP_TYPE_UPLOAD;		
	}
	
	CD3D12_HEAP_PROPERTIES HeapProperties( HeapType );
	CD3D12_RESOURCE_DESC ResourceDesc;
	
	const auto& Format = RsUtilsD3D12::GetTextureFormat( TextureDesc.Format_ );

	switch( TextureDesc.Type_ )
	{
	case RsTextureType::TEX1D:
		ResourceDesc = CD3D12_RESOURCE_DESC::Tex1D( 
			Format.RTVFormat_,
			TextureDesc.Width_, 1, (BcU16)TextureDesc.Levels_, 
			MiscFlag,
			D3D12_TEXTURE_LAYOUT_UNKNOWN );
		break;

	case RsTextureType::TEX2D:
		ResourceDesc = CD3D12_RESOURCE_DESC::Tex2D( 
			Format.RTVFormat_,
			TextureDesc.Width_, TextureDesc.Height_, 1, (BcU16)TextureDesc.Levels_, 1, 0,
			MiscFlag,
			D3D12_TEXTURE_LAYOUT_UNKNOWN );
		break;

	case RsTextureType::TEX3D:
		ResourceDesc = CD3D12_RESOURCE_DESC::Tex3D( 
			Format.RTVFormat_,
			TextureDesc.Width_, TextureDesc.Height_, (BcU16)TextureDesc.Depth_, (BcU16)TextureDesc.Levels_, 
			MiscFlag,
			D3D12_TEXTURE_LAYOUT_UNKNOWN );
		break;
	}

	// Clear value.
	D3D12_CLEAR_VALUE ClearValue;
	D3D12_CLEAR_VALUE* SetClearValue = nullptr;
	BcMemZero( &ClearValue, sizeof( ClearValue ) );

	// Setup initial bind type to be whatever is likely what it will be used as first.
	RsResourceBindFlags InitialBindType = RsResourceBindFlags::NONE;
	if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		ClearValue.Format = Format.RTVFormat_;
		SetClearValue = &ClearValue;
		InitialBindType = RsResourceBindFlags::RENDER_TARGET;
	}
	else if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		ClearValue.Format = Format.DSVFormat_;
		ClearValue.DepthStencil.Depth = 1.0f;
		SetClearValue = &ClearValue;
		InitialBindType = RsResourceBindFlags::DEPTH_STENCIL;
	}
	else if( ( TextureDesc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		InitialBindType = RsResourceBindFlags::SHADER_RESOURCE;
	}

	// TODO: Change this to use CreatePlacedResource, and appropriate heaps.
	// Should have a single large upload heap, an upload command list,
	// treat the heap as a circular buffer and fence to ensure we don't overwrite.
	ComPtr< ID3D12Resource > D3DResource;
	RetVal = Device_->CreateCommittedResource( 
		&HeapProperties,
		D3D12_HEAP_MISC_NONE,
		&ResourceDesc,
		RsUtilsD3D12::GetResourceUsage( InitialBindType ), 
		SetClearValue,
		IID_PPV_ARGS( D3DResource.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	Texture->setHandle( new RsResourceD3D12( D3DResource.Get(), TextureDesc.BindFlags_, InitialBindType ) );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextD3D12::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	auto Resource = Texture->getHandle< RsResourceD3D12* >();
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
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	const auto& TextureDesc = Texture->getDesc();
	std::unique_ptr< BcU8[] > TempBuffer;
		BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
		BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
		BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );
		BcU32 DataSize = RsTextureFormatSize( 
			TextureDesc.Format_,
			Width,
			Height,
			Depth,
			1 );
	TempBuffer.reset( new BcU8[ DataSize ] );
	RsTextureLock Lock = 
	{
		TempBuffer.get(),
		TextureDesc.Width_,
		TextureDesc.Width_ * TextureDesc.Height_
	};	
	UpdateFunc( Texture, Lock );
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
	auto ProgramD3D12 = Program->getHandle< RsProgramD3D12* >();
	delete ProgramD3D12;
	Program->setHandle< BcU64 >( 0 );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D12::flushState()
{
	// Graphics root signature.
	CommandList_->SetGraphicsRootSignature( DefaultRootSignature_.Get() );

	// Frame buffer.
	BcAssert( FrameBuffer_ );

	// Setup the formats for the pipeline state object.
	const auto FrameBufferDesc = FrameBuffer_->getDesc();
	GraphicsPSODesc_.FrameBufferFormatDesc_.NumRenderTargets_ = FrameBufferDesc.RenderTargets_.size();
	GraphicsPSODesc_.FrameBufferFormatDesc_.RTVFormats_.fill( RsTextureFormat::UNKNOWN );
	for( size_t Idx = 0; Idx < FrameBufferDesc.RenderTargets_.size(); ++Idx )
	{
		auto RenderTarget = FrameBufferDesc.RenderTargets_[ Idx ];
		const auto& RenderTargetDesc =  RenderTarget->getDesc();
		GraphicsPSODesc_.FrameBufferFormatDesc_.RTVFormats_[ Idx ] = RenderTargetDesc.Format_;
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
	FrameBufferD3D12->setRenderTargets( CommandList_.Get() );

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
		BcAssert( CommandList_ );
		CommandList_->SetPipelineState( GraphicsPS );
	}

	// Get descriptor sets from cache.
	std::array< ID3D12DescriptorHeap*, 2 > DescriptorHeaps;
	DescriptorHeaps[ 0 ] = DHCache_->getSamplersDescriptorHeap( SamplerStateDescs_ );
	DescriptorHeaps[ 1 ] = DHCache_->getShaderResourceDescriptorHeap( ShaderResourceDescs_ );

	CommandList_->SetDescriptorHeaps( DescriptorHeaps.data(), static_cast< UINT >( DescriptorHeaps.size() ) );

	// Set the descriptor tables.
	auto BaseSamplerDHHandle = DescriptorHeaps[ 0 ]->GetGPUDescriptorHandleForHeapStart();
	auto SamplerDescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3D12_SAMPLER_DESCRIPTOR_HEAP );
	auto BaseShaderResourceDHHandle = DescriptorHeaps[ 1 ]->GetGPUDescriptorHandleForHeapStart();
	auto ShaderResourceDescriptorSize = Device_->GetDescriptorHandleIncrementSize( D3D12_CBV_SRV_UAV_DESCRIPTOR_HEAP );

	UINT RootDescirptorIdx = 0;
	for( INT ShaderIdx = 0; ShaderIdx < 5; ++ShaderIdx )
	{
		// Samplers.
		CommandList_->SetGraphicsRootDescriptorTable( 
			RootDescirptorIdx++, BaseSamplerDHHandle );
		BaseSamplerDHHandle.Offset( RsDescriptorHeapSamplerStateDescD3D12::MAX_SAMPLERS, SamplerDescriptorSize );

		// Shader resource views.
		CommandList_->SetGraphicsRootDescriptorTable( 
			RootDescirptorIdx++, BaseShaderResourceDHHandle );
		BaseShaderResourceDHHandle.Offset( RsDescriptorHeapShaderResourceDescD3D12::MAX_SRVS, ShaderResourceDescriptorSize );

		// Constant buffer views.
		CommandList_->SetGraphicsRootDescriptorTable( 
			RootDescirptorIdx++, BaseShaderResourceDHHandle );
		BaseShaderResourceDHHandle.Offset( RsDescriptorHeapShaderResourceDescD3D12::MAX_CBVS, ShaderResourceDescriptorSize );
	}

	// Setup primitive, index buffer, and vertex buffers.
	if( GraphicsPSODesc_.Topology_ != RsTopologyType::INVALID )
	{
		CommandList_->IASetPrimitiveTopology( RsUtilsD3D12::GetPrimitiveTopology( GraphicsPSODesc_.Topology_ ) );
	}

	if( IndexBufferView_.BufferLocation != 0 )
	{
		CommandList_->SetIndexBuffer( &IndexBufferView_ );
	}

	for( UINT Idx = 0; Idx < VertexBufferViews_.size(); ++Idx )
	{
		if( VertexBufferViews_[ Idx ].BufferLocation != 0 )
		{
			CommandList_->SetVertexBuffers( Idx, &VertexBufferViews_[ Idx ], 1 ); 
		}
	}

	// Setup viewport.
	CommandList_->RSSetScissorRects( static_cast< UINT >( ScissorRects_.size() ), ScissorRects_.data() );
	CommandList_->RSSetViewports( static_cast< UINT >( Viewports_.size() ), Viewports_.data() );
}

//////////////////////////////////////////////////////////////////////////
// flushCommandList
void RsContextD3D12::flushCommandList()
{
	// Close command list.
	HRESULT RetVal = CommandList_->Close();
	BcAssert( SUCCEEDED( RetVal ) );

	// Execute command list.
	ID3D12CommandList* CommandLists[] = { CommandList_.Get() };
	CommandQueue_->ExecuteCommandLists( 1, CommandLists );

	// Wait for frame to complete.
	// This is not good for performance, so redo this later on.
	const UINT64 Fence = FlushCounter_;
	RetVal = CommandQueue_->Signal( PresentFence_.Get(), Fence );
	BcAssert( SUCCEEDED( RetVal ) );
	++FlushCounter_;

	auto CompletedValue = PresentFence_->GetCompletedValue();
	if( CompletedValue < Fence )
	{
		RetVal = PresentFence_->SetEventOnCompletion( Fence, PresentEvent_ );
		BcAssert( SUCCEEDED( RetVal ) );
		::WaitForSingleObject( PresentEvent_, INFINITE );
	}

	// Reset allocator and command list.
	RetVal = CommandAllocator_->Reset();
	BcAssert( SUCCEEDED( RetVal ) );

	RetVal = CommandList_->Reset( CommandAllocator_.Get(), DefaultPSO_.Get() );
	BcAssert( SUCCEEDED( RetVal ) );
}

//////////////////////////////////////////////////////////////////////////
// recreateBackBuffer
void RsContextD3D12::recreateBackBuffer()
{
	// TODO: Window resizing.
	if( BackBufferRT_ == nullptr || BackBufferDS_ == nullptr )
	{
		BcAssert( BackBufferRT_ == nullptr && BackBufferDS_ == nullptr );

		RsTextureDesc Desc;
		Desc.Type_ = RsTextureType::TEX2D;
		Desc.CreationFlags_  = RsResourceCreationFlags::NONE;
		Desc.Levels_ = 1;
		Desc.Width_ = SwapChainDesc_.BufferDesc.Width;
		Desc.Height_ = SwapChainDesc_.BufferDesc.Height;
		Desc.Depth_= 1;
		
		// Render target.
		Desc.BindFlags_ = RsResourceBindFlags::RENDER_TARGET | RsResourceBindFlags::PRESENT | RsResourceBindFlags::SHADER_RESOURCE;
		Desc.Format_ = RsTextureFormat::R8G8B8A8;
		BackBufferRT_ = new RsTexture( this, Desc );
		BackBufferRT_->setHandle( new RsResourceD3D12( nullptr, 
			Desc.BindFlags_,
			RsResourceBindFlags::PRESENT ) );

		// Depth stencil.
		Desc.BindFlags_ = RsResourceBindFlags::DEPTH_STENCIL;
		Desc.Format_ = RsTextureFormat::D24S8;
		BackBufferDS_ = new RsTexture( this, Desc );
		auto RetVal = createTexture( BackBufferDS_ );
		BcAssert( RetVal );
	}

	auto BackBufferRTResource = BackBufferRT_->getHandle< RsResourceD3D12* >();
	BcAssert( BackBufferRTResource );
	SwapChain_->GetBuffer( LastSwapBuffer_, IID_PPV_ARGS( BackBufferRTResource->getInternalResource().ReleaseAndGetAddressOf() ));
	
	if( BackBufferFB_ == nullptr )
	{
		RsFrameBufferDesc Desc = RsFrameBufferDesc( 1 )
			.setRenderTarget( 0, BackBufferRT_ )
			.setDepthStencilTarget( BackBufferDS_ );
		BackBufferFB_ = new RsFrameBuffer( this, Desc );
		auto RetVal = createFrameBuffer( BackBufferFB_ );
		BcAssert( RetVal );
	}
}

//////////////////////////////////////////////////////////////////////////
// createDefaultRootSignature
void RsContextD3D12::createDefaultRootSignature()
{
	HRESULT RetVal = E_FAIL;
	ComPtr< ID3DBlob > OutBlob, ErrorBlob;

	std::array< D3D12_DESCRIPTOR_RANGE, 3 > DescriptorRanges;
	DescriptorRanges[0].Init( D3D12_DESCRIPTOR_RANGE_SAMPLER, RsDescriptorHeapSamplerStateDescD3D12::MAX_SAMPLERS, 0 );
	DescriptorRanges[1].Init( D3D12_DESCRIPTOR_RANGE_SRV, RsDescriptorHeapShaderResourceDescD3D12::MAX_SRVS, 0 );
	DescriptorRanges[2].Init( D3D12_DESCRIPTOR_RANGE_CBV, RsDescriptorHeapShaderResourceDescD3D12::MAX_CBVS, 0 );

	std::array< D3D12_ROOT_PARAMETER, 15 > Parameters;
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
	
	D3D12_ROOT_SIGNATURE RootSignatureDesc;
	RootSignatureDesc.Init( 
		static_cast< UINT >( Parameters.size() ), Parameters.data(), 
		0, nullptr, 
		D3D12_ROOT_SIGNATURE_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT );
	RetVal = D3D12SerializeRootSignature( &RootSignatureDesc, D3D_ROOT_SIGNATURE_V1, OutBlob.GetAddressOf(), ErrorBlob.GetAddressOf() );
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
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_PER_VERTEX_DATA, 0 }
	};
	DefaultPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	DefaultPSO.InputLayout.NumElements = ARRAYSIZE( InputElementDescs );
	DefaultPSO.InputLayout.pInputElementDescs = InputElementDescs;
	DefaultPSO.VS.pShaderBytecode = g_VShader;
	DefaultPSO.VS.BytecodeLength = ARRAYSIZE( g_VShader );
	DefaultPSO.PS.pShaderBytecode = g_PShader;
	DefaultPSO.PS.BytecodeLength = ARRAYSIZE( g_PShader );
	DefaultPSO.pRootSignature = DefaultRootSignature_.Get();
	DefaultPSO.RasterizerState.FillMode = D3D12_FILL_SOLID;
	DefaultPSO.RasterizerState.CullMode = D3D12_CULL_NONE;
	DefaultPSO.NumRenderTargets = 1;

	DefaultPSO.SampleDesc = SwapChainDesc_.SampleDesc;
	DefaultPSO.RTVFormats[ 0 ] = SwapChainDesc_.BufferDesc.Format;

	RetVal = Device_->CreateGraphicsPipelineState( &DefaultPSO, IID_PPV_ARGS( DefaultPSO_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
}

//////////////////////////////////////////////////////////////////////////
// createCommandAllocators
void RsContextD3D12::createCommandAllocators()
{
	HRESULT RetVal = E_FAIL;
	RetVal = Device_->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( CommandAllocator_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
}

//////////////////////////////////////////////////////////////////////////
// createCommandLists
void RsContextD3D12::createCommandLists()
{
	HRESULT RetVal = E_FAIL;
	RetVal = Device_->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
		CommandAllocator_.Get(), DefaultPSO_.Get(), IID_PPV_ARGS( CommandList_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
}
