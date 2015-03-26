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
	pParent_( pParent ),
	pClient_( pClient ),
	Adapter_( nullptr ),
	ScreenshotRequested_( BcFalse ),
	OwningThread_( BcErrorCode )
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
	return pClient_->getWidth();
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextD3D12::getHeight() const
{
	return pClient_->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextD3D12::getClient() const
{
	return pClient_;
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
	SwapChain_->Present( 0, 0 );

	++FrameCounter_;
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
	OsClientWindows* pClient = dynamic_cast< OsClientWindows* >( pClient_ );
	BcAssertMsg( pClient != nullptr, "Windows client is not being used!" );

	// Setup swap chain desc.
	BcMemZero( &SwapChainDesc_, sizeof( SwapChainDesc_ ) );
    SwapChainDesc_.BufferCount = 1;
	SwapChainDesc_.BufferDesc.Width = pClient->getWidth();
    SwapChainDesc_.BufferDesc.Height = pClient->getHeight();
    SwapChainDesc_.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc_.BufferDesc.RefreshRate.Numerator = 60;
    SwapChainDesc_.BufferDesc.RefreshRate.Denominator = 1;
    SwapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc_.OutputWindow = pClient->getHWND();
	SwapChainDesc_.SampleDesc.Count = 1;
	SwapChainDesc_.SampleDesc.Quality = 0;
    SwapChainDesc_.Windowed = TRUE;
	SwapChainDesc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	setDefaultState();
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

}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextD3D12::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextD3D12::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextD3D12::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D12::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextD3D12::setSamplerState( BcU32 Handle, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D12::setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextD3D12::getRenderState( RsRenderStateType State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextD3D12::setSamplerState( BcU32 Handle, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextD3D12::setTexture( BcU32 Handle, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextD3D12::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsContextD3D12::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextD3D12::setVertexBuffer( 
	BcU32 StreamIdx, 
	class RsBuffer* VertexBuffer,
	BcU32 Stride )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextD3D12::setUniformBuffer( 
	BcU32 Handle, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextD3D12::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextD3D12::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
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
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextD3D12::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextD3D12::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
bool RsContextD3D12::createRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextD3D12::destroyRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextD3D12::createSamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextD3D12::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextD3D12::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextD3D12::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextD3D12::createBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextD3D12::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
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
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextD3D12::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextD3D12::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
	return false;
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
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextD3D12::createShader(
	class RsShader* Shader )
{
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextD3D12::destroyShader(
	class RsShader* Shader )
{
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextD3D12::createProgram(
	class RsProgram* Program )
{
	BcBreakpoint;
	return false;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextD3D12::destroyProgram(
	class RsProgram* Program )
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D12::flushState()
{
	BcBreakpoint;
}
