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
// Type conversion.
#if 0
static const D3D12_BIND_FLAG gBufferType[] =
{
	(D3D12_BIND_FLAG)0,			// RsBufferType::UNKNOWN
	D3D12_BIND_VERTEX_BUFFER,	// RsBufferType::VERTEX
	D3D12_BIND_INDEX_BUFFER,	// RsBufferType::INDEX
	D3D12_BIND_CONSTANT_BUFFER,	// RsBufferType::UNIFORM
	D3D12_BIND_UNORDERED_ACCESS,// RsBufferType::UNORDERED_ACCESS
	(D3D12_BIND_FLAG)0,			// RsBufferType::DRAW_INDIRECT
	D3D12_BIND_STREAM_OUTPUT,	// RsBufferType::STREAM_OUT
};
#endif

static const LPCSTR gSemanticName[] =
{
	"POSITION",					// RsVertexUsage::POSITION,
	"BLENDWEIGHTS",				// RsVertexUsage::BLENDWEIGHTS,
	"BLENDINDICES",				// RsVertexUsage::BLENDINDICES,
	"NORMAL",					// RsVertexUsage::NORMAL,
	"PSIZE",					// RsVertexUsage::PSIZE,
	"TEXCOORD",					// RsVertexUsage::TEXCOORD,
	"TANGENT",					// RsVertexUsage::TANGENT,
	"BINORMAL",					// RsVertexUsage::BINORMAL,
	"TESSFACTOR",				// RsVertexUsage::TESSFACTOR,
	"POSITIONT",				// RsVertexUsage::POSITIONT,
	"COLOR",					// RsVertexUsage::COLOUR,
	"FOG",						// RsVertexUsage::FOG,
	"DEPTH",					// RsVertexUsage::DEPTH,
	"SAMPLE",					// RsVertexUsage::SAMPLE,
};

static const D3D12_FILL_MODE gFillMode[] =
{
	D3D12_FILL_SOLID,
	D3D12_FILL_WIREFRAME,
};

static const D3D12_CULL_MODE gCullMode[] =
{
	D3D12_CULL_NONE,
	D3D12_CULL_BACK,
	D3D12_CULL_FRONT,
};

static const D3D12_COMPARISON_FUNC gCompareFunc[] =
{
	D3D12_COMPARISON_NEVER,			// RsCompareMode::NEVER,
	D3D12_COMPARISON_LESS,			// RsCompareMode::LESS,
	D3D12_COMPARISON_EQUAL,			// RsCompareMode::EQUAL,
	D3D12_COMPARISON_LESS_EQUAL,	// RsCompareMode::LESSEQUAL,
	D3D12_COMPARISON_GREATER,		// RsCompareMode::GREATER,
	D3D12_COMPARISON_NOT_EQUAL,		// RsCompareMode::NOTEQUAL,
	D3D12_COMPARISON_GREATER_EQUAL, // RsCompareMode::GREATEREQUAL,
	D3D12_COMPARISON_ALWAYS,		// RsCompareMode::ALWAYS,
};

static const D3D12_STENCIL_OP gStencilOp[] =
{
	D3D12_STENCIL_OP_KEEP,			// RsStencilOp::KEEP,
	D3D12_STENCIL_OP_ZERO,			// RsStencilOp::ZERO,
	D3D12_STENCIL_OP_REPLACE,		// RsStencilOp::REPLACE,
	D3D12_STENCIL_OP_INCR_SAT,		// RsStencilOp::INCR,
	D3D12_STENCIL_OP_INCR,			// RsStencilOp::INCR_WRAP,
	D3D12_STENCIL_OP_DECR_SAT,		// RsStencilOp::DECR,
	D3D12_STENCIL_OP_DECR,			// RsStencilOp::DECR_WRAP,
	D3D12_STENCIL_OP_INVERT,		// RsStencilOp::INVERT,
};

static const D3D12_TEXTURE_ADDRESS_MODE gTextureAddressMode[] =
{
	D3D12_TEXTURE_ADDRESS_WRAP,		// RsTextureSamplingMode::WRAP
	D3D12_TEXTURE_ADDRESS_MIRROR,	// RsTextureSamplingMode::MIRROR
	D3D12_TEXTURE_ADDRESS_CLAMP,	// RsTextureSamplingMode::CLAMP
	D3D12_TEXTURE_ADDRESS_BORDER,	// RsTextureSamplingMode::DECAL
};

static const D3D12_BLEND gBlendType[] =
{
    D3D12_BLEND_ZERO,
    D3D12_BLEND_ONE,
    D3D12_BLEND_SRC_COLOR,
    D3D12_BLEND_INV_SRC_COLOR,
    D3D12_BLEND_SRC_ALPHA,
    D3D12_BLEND_INV_SRC_ALPHA,
    D3D12_BLEND_DEST_COLOR,
    D3D12_BLEND_INV_DEST_COLOR,
    D3D12_BLEND_DEST_ALPHA,
    D3D12_BLEND_INV_DEST_ALPHA,
};

static const D3D12_BLEND_OP gBlendOp[] =
{
    D3D12_BLEND_OP_ADD,
    D3D12_BLEND_OP_SUBTRACT,
    D3D12_BLEND_OP_REV_SUBTRACT,
    D3D12_BLEND_OP_MIN,
    D3D12_BLEND_OP_MAX,
};


namespace
{
	DXGI_FORMAT getVertexElementFormat( RsVertexElement Element )
	{
		DXGI_FORMAT Format = DXGI_FORMAT_UNKNOWN;
		switch( Element.DataType_ )
		{
		case RsVertexDataType::FLOAT32:
			if( Element.Components_ == 1 )
				Format = DXGI_FORMAT_R32_FLOAT;
			else if( Element.Components_ == 2 )
				Format = DXGI_FORMAT_R32G32_FLOAT;
			else if( Element.Components_ == 3 )
				Format = DXGI_FORMAT_R32G32B32_FLOAT;
			else if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case RsVertexDataType::FLOAT16:
			if( Element.Components_ == 1 )
				Format = DXGI_FORMAT_R16_FLOAT;
			else if( Element.Components_ == 2 )
				Format = DXGI_FORMAT_R16G16_FLOAT;
			else if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
			break;
		case RsVertexDataType::BYTE:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R8G8B8A8_SINT;
			break;
		case RsVertexDataType::BYTE_NORM:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R8G8B8A8_SNORM;
			break;
		case RsVertexDataType::UBYTE:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R8G8B8A8_UINT;
			break;
		case RsVertexDataType::UBYTE_NORM:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			break;
		case RsVertexDataType::SHORT:
			if( Element.Components_ == 2 )
				Format = DXGI_FORMAT_R16G16_SINT;
			else if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_SINT;
			break;
		case RsVertexDataType::SHORT_NORM:
			if( Element.Components_ == 2 )
				Format = DXGI_FORMAT_R16G16_SNORM;
			else if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_SNORM;
			break;
		case RsVertexDataType::USHORT:
			if( Element.Components_ == 2 )
				Format = DXGI_FORMAT_R16G16_UINT;
			else if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_UINT;
			break;
		case RsVertexDataType::USHORT_NORM:
			if( Element.Components_ == 2 )
				Format = DXGI_FORMAT_R16G16_UNORM;
			else if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_UNORM;
			break;
		case RsVertexDataType::INT:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_SINT;
			break;
		case RsVertexDataType::INT_NORM:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_SNORM;
			break;
		case RsVertexDataType::UINT:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_UINT;
			break;
		case RsVertexDataType::UINT_NORM:
			if( Element.Components_ == 4 )
				Format = DXGI_FORMAT_R16G16B16A16_UNORM;
			break;
		}

		return Format;
	}
}

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
	FlushCounter_( 0 ),
	NumSwapBuffers_( 1 ),
	LastSwapBuffer_( 0 ),
	BackBufferRT_( nullptr ),
	BackBufferDS_( nullptr ),
	BackBufferFB_( nullptr ),
	OwningThread_( BcErrorCode ),
	ScreenshotRequested_( BcFalse )
{

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
	Viewports_.fill( D3DViewport );
}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextD3D12::create()
{
	OsClientWindows* Client = dynamic_cast< OsClientWindows* >( Client_ );
	BcAssertMsg( Client != nullptr, "Windows client is not being used!" );
	HRESULT RetVal = E_FAIL;

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

	// Create default root signature.
	createDefaultRootSignature();

	// Create default pipeline state.
	createDefaultPSO();

	// Create command allocators.
	createCommandAllocators();

	// Create command lists.
	createCommandLists();

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

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
	//PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextD3D12::setTexture( BcU32 Handle, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	
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
	auto VertexBufferView = VertexBufferViews_[ StreamIdx ];

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
#if 0
	// TODO: Generate handle that covers shaders properly.
	auto ConstantBufferView = ConstantBufferView_[ Handle ];
	
	if( UniformBuffer != nullptr )
	{
		auto UniformBufferResource = UniformBuffer->getHandle< RsResourceD3D12* >();
		ConstantBufferView.BufferLocation = UniformBufferResource->getInternalResource()->GetGPUVirtualAddress();
		ConstantBufferView.SizeInBytes = static_cast< UINT >( UniformBuffer->getDesc().SizeBytes_ );
	}
	else
	{
		ConstantBufferView.BufferLocation = 0;
		ConstantBufferView.SizeInBytes = 0;
	}
#endif
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
	
	//PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextD3D12::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	GraphicsPSODesc_.Topology_ = PrimitiveType;
	flushState();
	
	//PSY_LOG( "UNIMPLEMENTED" );
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
	//PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextD3D12::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	//PSY_LOG( "UNIMPLEMENTED" );
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
	BcUnusedVar( Texture );
	BcUnusedVar( Slice );
	BcUnusedVar( Flags );
	BcUnusedVar( UpdateFunc );
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
	//PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextD3D12::destroyShader(
	class RsShader* Shader )
{
	//PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextD3D12::createProgram(
	class RsProgram* Program )
{
	// Create storage class for program.
	Program->setHandle( new RsProgramD3D12( Program, Device_.Get() ) );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextD3D12::destroyProgram(
	class RsProgram* Program )
{
	auto ProgramInternal = Program->getHandle< RsProgramD3D12* >();
	delete ProgramInternal;
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
		GraphicsPSODesc_.FrameBufferFormatDesc_.DSVFormat_ = RsTextureFormat::INVALID;
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
	DescriptorRanges[0].Init( D3D12_DESCRIPTOR_RANGE_SRV, 32, 0 );
	DescriptorRanges[1].Init( D3D12_DESCRIPTOR_RANGE_CBV, 8, 0 );
	DescriptorRanges[2].Init( D3D12_DESCRIPTOR_RANGE_SAMPLER, 32, 0 );

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
