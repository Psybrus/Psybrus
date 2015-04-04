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
#include "System/Renderer/D3D12/RsResourceD3D12.h"

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

// Texture formats
static const DXGI_FORMAT gTextureFormats[] =
{
	// Colour.
	DXGI_FORMAT_R8_UNORM,				// RsTextureFormat::R8,
	DXGI_FORMAT_R8G8_UNORM,				// RsTextureFormat::R8G8,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R8G8B8,
	DXGI_FORMAT_R8G8B8A8_UNORM,			// RsTextureFormat::R8G8B8A8,
	DXGI_FORMAT_R16_FLOAT,				// RsTextureFormat::R16F,
	DXGI_FORMAT_R16G16_FLOAT,			// RsTextureFormat::R16FG16F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R16FG16FB16F,
	DXGI_FORMAT_R16G16B16A16_FLOAT,		// RsTextureFormat::R16FG16FB16FA16F,
	DXGI_FORMAT_R32_FLOAT,				// RsTextureFormat::R32F,
	DXGI_FORMAT_R32G32_FLOAT,			// RsTextureFormat::R32FG32F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R32FG32FB32F,
	DXGI_FORMAT_R32G32B32A32_FLOAT,		// RsTextureFormat::R32FG32FB32FA32F,
	DXGI_FORMAT_BC1_UNORM,				// RsTextureFormat::DXT1,
	DXGI_FORMAT_BC2_UNORM,				// RsTextureFormat::DXT3,
	DXGI_FORMAT_BC3_UNORM,				// RsTextureFormat::DXT5,
	// Depth.
	DXGI_FORMAT_R16_TYPELESS,			// RsTextureFormat::D16,
	DXGI_FORMAT_R24G8_TYPELESS,			// RsTextureFormat::D24,
	DXGI_FORMAT_R32_TYPELESS,			// RsTextureFormat::D32,
	DXGI_FORMAT_R24G8_TYPELESS,			// RsTextureFormat::D24S8,
	DXGI_FORMAT_R32_TYPELESS,			// RsTextureFormat::D32F,
};

// Depth stencil view formats.
static const DXGI_FORMAT gDSVFormats[] =
{
	// Colour.
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R8,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R8G8,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R8G8B8,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R8G8B8A8,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R16F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R16FG16F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R16FG16FB16F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R16FG16FB16FA16F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R32F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R32FG32F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R32FG32FB32F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R32FG32FB32FA32F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::DXT1,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::DXT3,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::DXT5,
	// Depth.
	DXGI_FORMAT_D16_UNORM,				// RsTextureFormat::D16,
	DXGI_FORMAT_D24_UNORM_S8_UINT,		// RsTextureFormat::D24,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::D32,
	DXGI_FORMAT_D24_UNORM_S8_UINT,		// RsTextureFormat::D24S8,
	DXGI_FORMAT_D32_FLOAT,				// RsTextureFormat::D32F,
};

// Shader resource view formats.
static const DXGI_FORMAT gSRVFormats[] = 
{
	// Colour.
	DXGI_FORMAT_R8_UNORM,				// RsTextureFormat::R8,
	DXGI_FORMAT_R8G8_UNORM,				// RsTextureFormat::R8G8,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R8G8B8,
	DXGI_FORMAT_R8G8B8A8_UNORM,			// RsTextureFormat::R8G8B8A8,
	DXGI_FORMAT_R16_FLOAT,				// RsTextureFormat::R16F,
	DXGI_FORMAT_R16G16_FLOAT,			// RsTextureFormat::R16FG16F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R16FG16FB16F,
	DXGI_FORMAT_R16G16B16A16_FLOAT,		// RsTextureFormat::R16FG16FB16FA16F,
	DXGI_FORMAT_R32_FLOAT,				// RsTextureFormat::R32F,
	DXGI_FORMAT_R32G32_FLOAT,			// RsTextureFormat::R32FG32F,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::R32FG32FB32F,
	DXGI_FORMAT_R32G32B32A32_FLOAT,		// RsTextureFormat::R32FG32FB32FA32F,
	DXGI_FORMAT_BC1_UNORM,				// RsTextureFormat::DXT1,
	DXGI_FORMAT_BC2_UNORM,				// RsTextureFormat::DXT3,
	DXGI_FORMAT_BC3_UNORM,				// RsTextureFormat::DXT5,
	// Depth.
	DXGI_FORMAT_R16_UNORM,				// RsTextureFormat::D16,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,	// RsTextureFormat::D24,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::D32,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,	// RsTextureFormat::D24S8,
	DXGI_FORMAT_R32_FLOAT,				// RsTextureFormat::D32F,
};

static const D3D12_PRIMITIVE_TOPOLOGY gTopologyType[] =
{
	D3D_PRIMITIVE_TOPOLOGY_POINTLIST,					// RsTopologyType::POINTLIST = 0,
	D3D_PRIMITIVE_TOPOLOGY_LINELIST,					// RsTopologyType::LINE_LIST,
	D3D_PRIMITIVE_TOPOLOGY_LINESTRIP,					// RsTopologyType::LINE_STRIP,
	D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ,				// RsTopologyType::LINE_LIST_ADJACENCY,
	D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ,				// RsTopologyType::LINE_STRIP_ADJACENCY,
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST,				// RsTopologyType::TRIANGLE_LIST,
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,				// RsTopologyType::TRIANGLE_STRIP,
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ,			// RsTopologyType::TRIANGLE_LIST_ADJACENCY,
	D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ,			// RsTopologyType::TRIANGLE_STRIP_ADJACENCY,
	D3D_PRIMITIVE_TOPOLOGY_UNDEFINED,					// RsTopologyType::TRIANGLE_FAN,
	D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST,	// RsTopologyType::PATCHES,
};

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


	const BcU32 NoofBindPoints = 32;
	const BcU32 MaxBindPoints = NoofBindPoints - 1;
	const BcU32 BitsPerShader = 5; // Up to 32 bindings.
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
	ResetCommandList_( BcTrue ),
	CommandList_(),
	PresentFence_(),
	FrameCounter_( 0 ),
	NumSwapBuffers_( 2 ),
	LastSwapBuffer_( 0 ),
	BackBufferRT_( nullptr ),
	BackBufferDS_( nullptr ),
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
	// End current frame.
	end();

	// 
	if( ResetCommandList_ == BcFalse )
	{
		// Do present.
		SwapChain_->Present( 1, 0 );

		// Get next buffer.
		LastSwapBuffer_ = ( 1 + LastSwapBuffer_ ) % NumSwapBuffers_;

		// Wait for frame to complete.
		// This is not good for performance, so redo this later on.
		HRESULT RetVal = E_FAIL;
		const UINT64 Fence = FrameCounter_;
		RetVal = CommandQueue_->Signal( PresentFence_.Get(), Fence );
		BcAssert( SUCCEEDED( RetVal ) );
		++FrameCounter_;

		if( PresentFence_->GetCompletedValue() < Fence )
		{
			RetVal = PresentFence_->SetEventOnCompletion( Fence, PresentEvent_ );
			BcAssert( SUCCEEDED( RetVal ) );
			::WaitForSingleObject( PresentEvent_, INFINITE );
		}
	}

	// Begin next frame.
	begin();
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
    SwapChainDesc_.BufferCount = 1;
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
	SwapChainDesc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	RetVal = Factory_->CreateSwapChain( CommandQueue_.Get(), &SwapChainDesc_, &SwapChain_ );
	BcAssert( SUCCEEDED( RetVal ) );

	// Create present fence.
	FrameCounter_ = 0;
	RetVal = Device_->CreateFence( FrameCounter_, D3D12_FENCE_MISC_NONE, IID_PPV_ARGS( PresentFence_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );
	PresentEvent_ = ::CreateEventEx( nullptr, FALSE, FALSE, EVENT_ALL_ACCESS );

	// Create pipeline state cache.
	PSCache_.reset( new RsPipelineStateCacheD3D12( Device_.Get() ) );
		
	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	// Recreate backbuffer.
	recreateBackBuffer();

	setDefaultState();

	// Begin frame.
	begin();
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
	if( BackBufferRT_ != nullptr )
	{
		destroyTexture( BackBufferRT_ );
	}

	if( BackBufferDS_ != nullptr )
	{
		destroyTexture( BackBufferDS_ );
	}

	// Cleanup everything.
	PSCache_.reset();
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
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextD3D12::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextD3D12::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D12::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextD3D12::setSamplerState( BcU32 Handle, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D12::setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextD3D12::getRenderState( RsRenderStateType State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextD3D12::setSamplerState( BcU32 Handle, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextD3D12::setTexture( BcU32 Handle, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextD3D12::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsContextD3D12::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextD3D12::setVertexBuffer( 
	BcU32 StreamIdx, 
	class RsBuffer* VertexBuffer,
	BcU32 Stride )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextD3D12::setUniformBuffer( 
	BcU32 Handle, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextD3D12::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextD3D12::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
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

	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextD3D12::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	flushState();
	
	PSY_LOG( "UNIMPLEMENTED" );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextD3D12::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	flushState();
	
	PSY_LOG( "UNIMPLEMENTED" );
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
	PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextD3D12::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextD3D12::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextD3D12::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
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

	BackBufferRT_->setHandle( new RsResourceD3D12( D3DResource.Get(), BindFlags, BindFlags ) );
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
	BcUnusedVar( UpdateFunc );
	std::unique_ptr< BcU8[] > TempBuffer;
	TempBuffer.reset( new BcU8[ Buffer->getDesc().SizeBytes_ ] );
	RsBufferLock Lock = { TempBuffer.get() };
	UpdateFunc( Buffer, Lock );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextD3D12::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	PSY_LOG( "UNIMPLEMENTED" );
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
	PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextD3D12::destroyShader(
	class RsShader* Shader )
{
	PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextD3D12::createProgram(
	class RsProgram* Program )
{
	// TODO: Look up also by type, size, and flags. Not just name.
	// TODO: Do this work offline.
	typedef std::map< std::string, BcU32 > ResourceHandleMapping;
	ResourceHandleMapping SamplerBindings;
	ResourceHandleMapping TextureBindings;
	ResourceHandleMapping ConstantBufferBindings;
	ResourceHandleMapping ConstantBufferSizes;

	// Iterate over shaders and setup handles for all constant
	// buffers and shader resources.
	for( auto* Shader : Program->getShaders() )
	{
		const auto& ShaderDesc = Shader->getDesc();
		ID3D12ShaderReflection* Reflector = nullptr; 
		D3DReflect( Shader->getData(), Shader->getDataSize(),
			IID_ID3D12ShaderReflection, (void**)&Reflector );

		const BcU32 ShiftAmount = ( (BcU32)ShaderDesc.ShaderType_ * BitsPerShader );
		const BcU32 MaskOff = ~( MaxBindPoints << ShiftAmount );

		// Just iterate over a big number...we'll assert if we go over.
		for( BcU32 Idx = 0; Idx < 128; ++Idx )
		{
			D3D12_SHADER_INPUT_BIND_DESC BindDesc;
			if( SUCCEEDED( Reflector->GetResourceBindingDesc( Idx, &BindDesc ) ) )
			{
				// Validate.
				BcAssert( 
					BindDesc.BindPoint < MaxBindPoints && 
					( BindDesc.BindPoint + BindDesc.BindCount ) <= MaxBindPoints );

				// Check if it's a cbuffer or tbuffer.
				if( BindDesc.Type == D3D_SIT_CBUFFER ||
					BindDesc.Type == D3D_SIT_TBUFFER )
				{
					if( ConstantBufferBindings.find( BindDesc.Name ) == ConstantBufferBindings.end() )
					{
						ConstantBufferBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = ConstantBufferBindings[ BindDesc.Name ];
					BcU32 Size = ConstantBufferSizes[ BindDesc.Name ];			
					
					auto ConstantBuffer = Reflector->GetConstantBufferByName( BindDesc.Name );
					D3D12_SHADER_BUFFER_DESC BufferDesc;
					ConstantBuffer->GetDesc( &BufferDesc );
					if( Size != 0 )
					{
						BcAssert( BufferDesc.Size == Size );
					}


					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					Size = BufferDesc.Size;
					ConstantBufferBindings[ BindDesc.Name ] = Handle;
					ConstantBufferSizes[ BindDesc.Name ] = Size;
				}
				else if( BindDesc.Type == D3D_SIT_TEXTURE )
				{
					if( TextureBindings.find( BindDesc.Name ) == TextureBindings.end() )
					{
						TextureBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = TextureBindings[ BindDesc.Name ];
					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					TextureBindings[ BindDesc.Name ] = Handle;
				}
				else if( BindDesc.Type == D3D_SIT_SAMPLER )
				{
					if( SamplerBindings.find( BindDesc.Name ) == SamplerBindings.end() )
					{
						SamplerBindings[ BindDesc.Name ] = BcErrorCode;
					}

					BcU32 Handle = SamplerBindings[ BindDesc.Name ];
					Handle = ( Handle & MaskOff ) | ( BindDesc.BindPoint << ShiftAmount );
					SamplerBindings[ BindDesc.Name ] = Handle;
				}
				else
				{
					// TOOD.
				}
			}
		}
	}

	// Add all constant buffer bindings
	for( const auto& ConstantBuffer : ConstantBufferBindings )
	{
		auto Size = ConstantBufferSizes[ ConstantBuffer.first ];
		auto Class = ReManager::GetClass( ConstantBuffer.first );
		BcAssert( Class->getSize() == Size );
		Program->addUniformBufferSlot( 
			ConstantBuffer.first,
			ConstantBuffer.second,
			Class );
	}

	// Add all sampler bindings
	for( const auto& Sampler : SamplerBindings )
	{
		Program->addSamplerSlot( 
			Sampler.first,
			Sampler.second );
	}

	// Add all texture bindings
	for( const auto& Texture : TextureBindings )
	{
		Program->addTextureSlot( 
			Texture.first,
			Texture.second );
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextD3D12::destroyProgram(
	class RsProgram* Program )
{
	PSY_LOG( "UNIMPLEMENTED" );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D12::flushState()
{
	ID3D12PipelineState* GraphicsPS = nullptr;

	// Get current pipeline state.
	GraphicsPS = PSCache_->getPipelineState( GraphicsPSDesc_ );

	// Reset command list if we need to, otherwise just set new pipeline state.
	if( GraphicsPS != nullptr )
	{
		if( ResetCommandList_ )
		{
			// Create command list if we haven't already got one.
			if( CommandList_ == nullptr )
			{
				HRESULT RetVal = E_FAIL;
				RetVal = Device_->CreateCommandList( 0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
					CommandAllocator_.Get(), GraphicsPS, IID_PPV_ARGS( CommandList_.GetAddressOf() ) );
				BcAssert( SUCCEEDED( RetVal ) );
			}

			HRESULT RetVal = E_FAIL;
			RetVal = CommandAllocator_->Reset();
			BcAssert( SUCCEEDED( RetVal ) );

			RetVal = CommandList_->Reset( CommandAllocator_.Get(), GraphicsPS );
			BcAssert( SUCCEEDED( RetVal ) );

			ResetCommandList_ = BcFalse;
		}
		else
		{
			BcAssert( CommandList_ );
			CommandList_->SetPipelineState( GraphicsPS );
		}
	}
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
		Desc.BindFlags_ = RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::RENDER_TARGET;
		Desc.Format_ = RsTextureFormat::R8G8B8A8;
		BackBufferRT_ = new RsTexture( this, Desc );
		BackBufferRT_->setHandle( new RsResourceD3D12( nullptr, 
			RsResourceBindFlags::RENDER_TARGET | 
			RsResourceBindFlags::SHADER_RESOURCE |
			RsResourceBindFlags::PRESENT,
			RsResourceBindFlags::RENDER_TARGET ) );

		// Depth stencil.
		Desc.BindFlags_ = RsResourceBindFlags::SHADER_RESOURCE | RsResourceBindFlags::DEPTH_STENCIL;
		Desc.Format_ = RsTextureFormat::D24S8;
		BackBufferDS_ = new RsTexture( this, Desc );
		BackBufferDS_->setHandle( new RsResourceD3D12( nullptr, 
			RsResourceBindFlags::DEPTH_STENCIL | 
			RsResourceBindFlags::SHADER_RESOURCE |
			RsResourceBindFlags::PRESENT,
			RsResourceBindFlags::DEPTH_STENCIL ) );
	}

	auto BackBufferRTResource = BackBufferRT_->getHandle< RsResourceD3D12* >();
	BcAssert( BackBufferRTResource );
	SwapChain_->GetBuffer( LastSwapBuffer_, IID_PPV_ARGS( BackBufferRTResource->getInternalResource().ReleaseAndGetAddressOf() ));
}

//////////////////////////////////////////////////////////////////////////
// begin
void RsContextD3D12::begin()
{
	recreateBackBuffer();

	ResetCommandList_ = BcTrue;
}


//////////////////////////////////////////////////////////////////////////
// end
void RsContextD3D12::end()
{
	// Still waiting on a reset.
	if( ResetCommandList_ == BcTrue )
	{
		return;
	}

	BcAssert( CommandList_ != nullptr );

	// Transition back buffer to present.
	RsResourceD3D12* BackBufferResource = BackBufferRT_->getHandle< RsResourceD3D12* >();
	BackBufferResource->resourceBarrierTransition( CommandList_.Get(), RsResourceBindFlags::PRESENT );

	// Close command list.
	CommandList_->Close();

	// Execute command list.
	ID3D12CommandList* CommandLists[] = { CommandList_.Get() };
	CommandQueue_->ExecuteCommandLists( 1, CommandLists );
}
