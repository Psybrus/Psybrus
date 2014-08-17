/**************************************************************************
*
* File:		RsContextD3D11.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/D3D11/RsContextD3D11.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsVertexDeclaration.h"

#include "Base/BcMath.h"

#include "System/Os/OsClient.h"
#include "System/Os/OsClientWindows.h"

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// Direct3D 11 libraries.
#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxguid.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment( lib, "D3DCompiler.lib" )

//////////////////////////////////////////////////////////////////////////
// Type conversion.
static D3D11_BIND_FLAG gBufferType[] =
{
	(D3D11_BIND_FLAG)0,			// RsBufferType::UNKNOWN
	D3D11_BIND_VERTEX_BUFFER,	// RsBufferType::VERTEX
	D3D11_BIND_INDEX_BUFFER,	// RsBufferType::INDEX
	D3D11_BIND_CONSTANT_BUFFER,	// RsBufferType::UNIFORM
	D3D11_BIND_UNORDERED_ACCESS,// RsBufferType::UNORDERED_ACCESS
	(D3D11_BIND_FLAG)0,			// RsBufferType::DRAW_INDIRECT
	D3D11_BIND_STREAM_OUTPUT,	// RsBufferType::STREAM_OUT
};

static DXGI_FORMAT gTextureFormats[] =
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
	DXGI_FORMAT_BC3_UNORM,				// RsTextureFormat::DXT3,
	DXGI_FORMAT_BC5_UNORM,				// RsTextureFormat::DXT5,
	DXGI_FORMAT_D16_UNORM,				// RsTextureFormat::D16,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::D32,
	DXGI_FORMAT_D24_UNORM_S8_UINT,		// RsTextureFormat::D24S8,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::D32F,
};

static D3D11_PRIMITIVE_TOPOLOGY gTopologyType[] =
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

static LPCSTR gSemanticName[] =
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

static D3D11_COMPARISON_FUNC gCompareFunc[] =
{
	D3D11_COMPARISON_NEVER,			// RsCompareMode::NEVER,
	D3D11_COMPARISON_LESS,			// RsCompareMode::LESS,
	D3D11_COMPARISON_EQUAL,			// RsCompareMode::EQUAL,
	D3D11_COMPARISON_LESS_EQUAL,	// RsCompareMode::LESSEQUAL,
	D3D11_COMPARISON_GREATER,		// RsCompareMode::GREATER,
	D3D11_COMPARISON_NOT_EQUAL,		// RsCompareMode::NOTEQUAL,
	D3D11_COMPARISON_GREATER_EQUAL, // RsCompareMode::GREATEREQUAL,
	D3D11_COMPARISON_ALWAYS,		// RsCompareMode::ALWAYS,
};

static D3D11_STENCIL_OP gStencilOp[] =
{
	D3D11_STENCIL_OP_KEEP,			// RsStencilOp::KEEP,
	D3D11_STENCIL_OP_ZERO,			// RsStencilOp::ZERO,
	D3D11_STENCIL_OP_REPLACE,		// RsStencilOp::REPLACE,
	D3D11_STENCIL_OP_INCR_SAT,		// RsStencilOp::INCR,
	D3D11_STENCIL_OP_INCR,			// RsStencilOp::INCR_WRAP,
	D3D11_STENCIL_OP_DECR_SAT,		// RsStencilOp::DECR,
	D3D11_STENCIL_OP_DECR,			// RsStencilOp::DECR_WRAP,
	D3D11_STENCIL_OP_INVERT,		// RsStencilOp::INVERT,
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
RsContextD3D11::RsContextD3D11( OsClient* pClient, RsContextD3D11* pParent ):
	RsContext( pParent ),
	pParent_( pParent ),
	pClient_( pClient ),
	Adapter_( nullptr ),
	Device_( nullptr ),
	Context_( nullptr ),
	ScreenshotRequested_( BcFalse ),
	OwningThread_( BcErrorCode )
{
	IndexBuffer_ = nullptr;
	D3DIndexBuffer_ = nullptr;
	BcMemZero( &VertexBuffers_[ 0 ], sizeof( VertexBuffers_ ) );
	BcMemZero( &D3DVertexBuffers_[ 0 ], sizeof( D3DVertexBuffers_ ) );
	BcMemZero( &D3DVertexBufferStrides_[ 0 ], sizeof( D3DVertexBufferStrides_ ) );
	BcMemZero( &D3DVertexBufferOffsets_[ 0 ], sizeof( D3DVertexBufferOffsets_ ) );
	BcMemZero( &UniformBuffers_[ 0 ], sizeof( UniformBuffers_ ) );
	BcMemZero( &D3DConstantBuffers_[ 0 ], sizeof( D3DConstantBuffers_ ) );
	BcMemZero( &TextureResources_[ 0 ], sizeof( TextureResources_ ) );
	BcMemZero( &D3DShaderResourceViews_[ 0 ], sizeof( D3DShaderResourceViews_ ) );
	InputLayoutChanged_ = false;
	Program_ = nullptr;
	VertexDeclaration_ = nullptr;
	TopologyType_ = RsTopologyType::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsContextD3D11::~RsContextD3D11()
{

}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 RsContextD3D11::getWidth() const
{
	return pClient_->getWidth();
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextD3D11::getHeight() const
{
	return pClient_->getHeight();
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextD3D11::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
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
RsShaderCodeType RsContextD3D11::maxShaderCodeType( RsShaderCodeType CodeType ) const
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
void RsContextD3D11::presentBackBuffer()
{
	SwapChain_->Present( 0, 0 );
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextD3D11::takeScreenshot()
{

}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextD3D11::setViewport( class RsViewport& Viewport )
{

}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextD3D11::create()
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

	// Create device and swap chain.
	FeatureLevel_ = D3D_FEATURE_LEVEL_11_0;
	HRESULT Result = D3D11CreateDeviceAndSwapChain( 
		Adapter_,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_SINGLETHREADED,// | D3D11_CREATE_DEVICE_DEBUG,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&SwapChainDesc_,
		&SwapChain_,
		&Device_,
		&FeatureLevel_,
		&Context_ );
	BcAssert( SUCCEEDED( Result ) ); 
	
	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	// Check threading feature.
	D3D11_FEATURE_DATA_THREADING FeatureThreading;
	Device_->CheckFeatureSupport(
		D3D11_FEATURE_THREADING,
		&FeatureThreading,
		sizeof( FeatureThreading ) );

	// Get back buffer from swap chain.
	SwapChain_->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer_ );
	BackBufferResourceIdx_ = addD3DResource( BackBuffer_ );

	ID3D11RenderTargetView* BackBufferRTView = getD3DRenderTargetView( BackBufferResourceIdx_ );
	Context_->OMSetRenderTargets( 1, &BackBufferRTView, nullptr ); 
}

//////////////////////////////////////////////////////////////////////////
// update
void RsContextD3D11::update()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
void RsContextD3D11::destroy()
{
	SwapChain_ = nullptr;
	Device_ = nullptr;
	Context_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextD3D11::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//BcPrintf( "WARNING: RsContextD3D11::setDefaultState unimplemented\n" );
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextD3D11::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//BcPrintf( "WARNING: RsContextD3D11::invalidateRenderState unimplemented\n" );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextD3D11::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//BcPrintf( "WARNING: RsContextD3D11::invalidateTextureState unimplemented\n" );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D11::setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	switch( State )
	{
	case RsRenderStateType::DEPTH_WRITE_ENABLE:
		DepthStencilState_.DepthWriteMask = Value ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
		break;
	case RsRenderStateType::DEPTH_TEST_ENABLE:
		DepthStencilState_.DepthEnable = Value ? 1 : 0;
		break;
	case RsRenderStateType::DEPTH_TEST_COMPARE:
		DepthStencilState_.DepthFunc = gCompareFunc[ Value ];
		break;
	case RsRenderStateType::STENCIL_WRITE_MASK:
		DepthStencilState_.StencilWriteMask = Value;
		break;
	case RsRenderStateType::STENCIL_TEST_ENABLE:
		DepthStencilState_.StencilEnable = Value ? 1 : 0;
		break;
	case RsRenderStateType::STENCIL_TEST_FUNC_COMPARE:
		DepthStencilState_.FrontFace.StencilFunc = gCompareFunc[ Value ];
		DepthStencilState_.BackFace.StencilFunc = gCompareFunc[ Value ];
		break;
	case RsRenderStateType::STENCIL_TEST_FUNC_REF:
		BcBreakpoint;
		break;
	case RsRenderStateType::STENCIL_TEST_FUNC_MASK:
		DepthStencilState_.StencilReadMask = Value;
		break;
	case RsRenderStateType::STENCIL_TEST_OP_SFAIL:
		DepthStencilState_.FrontFace.StencilFailOp = gStencilOp[ Value ];
		DepthStencilState_.BackFace.StencilFailOp = gStencilOp[ Value ];
		break;
	case RsRenderStateType::STENCIL_TEST_OP_DPFAIL:
		DepthStencilState_.FrontFace.StencilDepthFailOp = gStencilOp[ Value ];
		DepthStencilState_.BackFace.StencilDepthFailOp = gStencilOp[ Value ];
		break;
	case RsRenderStateType::STENCIL_TEST_OP_DPPASS:
		DepthStencilState_.FrontFace.StencilPassOp = gStencilOp[ Value ];
		DepthStencilState_.BackFace.StencilPassOp = gStencilOp[ Value ];
		break;
	case RsRenderStateType::COLOR_WRITE_MASK_0:
		BlendState_.RenderTarget[ 0 ].RenderTargetWriteMask = Value;
		break;
	case RsRenderStateType::COLOR_WRITE_MASK_1:
		BlendState_.RenderTarget[ 1 ].RenderTargetWriteMask = Value;
		break;
	case RsRenderStateType::COLOR_WRITE_MASK_2:
		BlendState_.RenderTarget[ 2 ].RenderTargetWriteMask = Value;
		break;
	case RsRenderStateType::COLOR_WRITE_MASK_3:
		BlendState_.RenderTarget[ 3 ].RenderTargetWriteMask = Value;
		break;
	case RsRenderStateType::BLEND_MODE:
		{
			for( BcU32 Idx = 0; Idx < 8; ++Idx )
			{
				switch( Value )
				{
				case RsBlendingMode::NONE:
					BlendState_.RenderTarget[ Idx ].BlendEnable = FALSE;
					break;
				case RsBlendingMode::BLEND:
					BlendState_.RenderTarget[ Idx ].BlendEnable = TRUE;
					BlendState_.RenderTarget[ Idx ].BlendOp = D3D11_BLEND_OP_ADD;
					BlendState_.RenderTarget[ Idx ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					BlendState_.RenderTarget[ Idx ].SrcBlendAlpha= D3D11_BLEND_SRC_ALPHA;
					BlendState_.RenderTarget[ Idx ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					BlendState_.RenderTarget[ Idx ].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
					break;
				case RsBlendingMode::ADD:
					BlendState_.RenderTarget[ Idx ].BlendEnable = TRUE;
					BlendState_.RenderTarget[ Idx ].BlendOp = D3D11_BLEND_OP_ADD;
					BlendState_.RenderTarget[ Idx ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					BlendState_.RenderTarget[ Idx ].SrcBlendAlpha= D3D11_BLEND_ONE;
					BlendState_.RenderTarget[ Idx ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					BlendState_.RenderTarget[ Idx ].DestBlendAlpha = D3D11_BLEND_ONE;
					break;
				case RsBlendingMode::SUBTRACT:
					BlendState_.RenderTarget[ Idx ].BlendEnable = TRUE;
					BlendState_.RenderTarget[ Idx ].BlendOp = D3D11_BLEND_OP_SUBTRACT;
					BlendState_.RenderTarget[ Idx ].SrcBlend = D3D11_BLEND_SRC_ALPHA;
					BlendState_.RenderTarget[ Idx ].SrcBlendAlpha= D3D11_BLEND_ONE;
					BlendState_.RenderTarget[ Idx ].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
					BlendState_.RenderTarget[ Idx ].DestBlendAlpha = D3D11_BLEND_ONE;
					break;
				}
			}
		}
		break;
	case RsRenderStateType::FILL_MODE:
		RasterizerState_.FillMode = Value == (BcU32)RsFillMode::SOLID ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextD3D11::getRenderState( RsRenderStateType State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//BcPrintf( "WARNING: RsContextD3D11::getRenderState unimplemented\n" );

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setTextureState
void RsContextD3D11::setTextureState( BcU32 SlotIdx, RsTexture* pTexture, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	TextureResources_[ SlotIdx ] = pTexture;

	// Find shader resource view.
	ID3D11ShaderResourceView* ShaderResourceView = getD3DShaderResourceView( pTexture->getHandle< BcU32 >() );

	//BcPrintf( "WARNING: RsContextD3D11::setTextureState unimplemented\n" );
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextD3D11::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	InputLayoutChanged_ |= Program_ != Program; // TODO: Only check vertex shader.
	Program_ = Program;
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsContextD3D11::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	IndexBuffer_ = IndexBuffer;
	D3DIndexBuffer_ = getD3DBuffer( IndexBuffer->getHandle< BcU32 >() );
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextD3D11::setVertexBuffer( 
	BcU32 StreamIdx, 
	class RsBuffer* VertexBuffer,
	BcU32 Stride )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	VertexBuffers_[ StreamIdx ] = VertexBuffer;
	D3DVertexBuffers_[ StreamIdx ] = getD3DBuffer( VertexBuffer->getHandle< BcU32 >() );
	D3DVertexBufferStrides_[ StreamIdx ] = Stride;
	D3DVertexBufferOffsets_[ StreamIdx ] = 0;
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextD3D11::setUniformBuffer( 
	BcU32 Handle, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Bind for each shader based on specified handle.
	for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
	{
		BcU32 SlotIdx = ( Handle << ( Idx * BitsPerShader ) ) & NoofBindPoints;

		if( SlotIdx != NoofBindPoints )
		{
			UniformBuffers_[ Idx ][ SlotIdx ] = UniformBuffer;
			D3DConstantBuffers_[ Idx ][ SlotIdx ] = getD3DBuffer( UniformBuffer->getHandle< BcU32 >() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextD3D11::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	InputLayoutChanged_ |= VertexDeclaration_ != VertexDeclaration;
	VertexDeclaration_ = VertexDeclaration;
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextD3D11::clear( const RsColour& Colour )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcPrintf( "WARNING: RsContextD3D11::clear unimplemented\n" );
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextD3D11::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	
	// Check topology type.
	TopologyType_ = PrimitiveType;

	flushState();
	Context_->Draw( NoofIndices, IndexOffset );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextD3D11::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Check topology type.
	TopologyType_ = PrimitiveType;

	flushState();
	Context_->DrawIndexed( NoofIndices, IndexOffset, VertexOffset );
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextD3D11::createBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//
	const auto& BufferDesc = Buffer->getDesc();

	// Buffer desc.
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DEFAULT;			// TODO.
	Desc.ByteWidth = BcPotRoundUp( BufferDesc.SizeBytes_, 16 );
	Desc.BindFlags = gBufferType[ (BcU32)BufferDesc.Type_ ];
	Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	Desc.MiscFlags = 0;
	Desc.StructureByteStride = 0;
	
	if( ( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
	{
		Desc.Usage = D3D11_USAGE_DYNAMIC;
	}
	else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
	{
		Desc.Usage = D3D11_USAGE_DYNAMIC;
	}

	// TODO: Use default, immutable, or staging.
	Desc.Usage = D3D11_USAGE_DYNAMIC;

	// Generate buffers.
	ID3D11Buffer* D3DBuffer = nullptr; 

	HRESULT Result = Device_->CreateBuffer( &Desc, nullptr, &D3DBuffer );
	if( Result == S_OK )
	{
		Buffer->setHandle( addD3DResource( D3DBuffer ) );
		return true;
	}

	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextD3D11::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	delD3DResource( Buffer->getHandle< BcU32 >() );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsContextD3D11::updateBuffer( 
	class RsBuffer* Buffer,
	BcSize Offset,
	BcSize Size,
	RsResourceUpdateFlags Flags,
	RsBufferUpdateFunc UpdateFunc )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	ID3D11Buffer* D3DBuffer = getD3DBuffer( Buffer->getHandle< BcU32 >() ); 

	if( D3DBuffer != nullptr )
	{
		D3D11_MAP MapType = 
			D3D11_MAP_WRITE_DISCARD;		// TODO: Select more optimal map type.
		BcAssert( Offset == 0 );
		D3D11_MAPPED_SUBRESOURCE SubRes = { nullptr, 0, 0 };
		HRESULT RetVal = Context_->Map( D3DBuffer, 0, MapType, 0, &SubRes );
		if( SUCCEEDED( RetVal ) )
		{
			RsBufferLock Lock = 
			{
				(BcU8*)SubRes.pData + Offset
			};
			UpdateFunc( Buffer, Lock );
			Context_->Unmap( D3DBuffer, 0 );
			return true;
		}
		BcBreakpoint;
		
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextD3D11::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	HRESULT Result = 0;
	//
	const auto& TextureDesc = Texture->getDesc();

	// Buffer desc.
	switch( TextureDesc.Type_ )
	{
	case RsTextureType::TEX1D:
		{
			D3D11_TEXTURE1D_DESC Desc;
			Desc.Width = TextureDesc.Width_;
			Desc.MipLevels = TextureDesc.Levels_;
			Desc.ArraySize = 1;
			Desc.Format = gTextureFormats[ (BcU32)TextureDesc.Format_ ];
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture1D* D3DTexture = nullptr;
			Result = Device_->CreateTexture1D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );
			
			Texture->setHandle( addD3DResource( D3DTexture ) );
			return true;
		}
		break;

	case RsTextureType::TEX2D:
		{
			D3D11_TEXTURE2D_DESC Desc;
			Desc.Width = TextureDesc.Width_;
			Desc.Height = TextureDesc.Height_;
			Desc.MipLevels = TextureDesc.Levels_;
			Desc.ArraySize = 1;
			Desc.Format = gTextureFormats[ (BcU32)TextureDesc.Format_ ];
			Desc.SampleDesc.Count = 1;
			Desc.SampleDesc.Quality = 0;
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture2D* D3DTexture = nullptr;
 			Result = Device_->CreateTexture2D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );

			Texture->setHandle( addD3DResource( D3DTexture ) );
			return true;
		}
		break;

	case RsTextureType::TEX3D:
		{
			D3D11_TEXTURE3D_DESC Desc;
			Desc.Width = TextureDesc.Width_;
			Desc.Height = TextureDesc.Height_;
			Desc.Depth = TextureDesc.Depth_;
			Desc.MipLevels = TextureDesc.Levels_;
			Desc.Format = gTextureFormats[ (BcU32)TextureDesc.Format_ ];
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			ID3D11Texture3D* D3DTexture = nullptr;
			Result = Device_->CreateTexture3D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );

			Texture->setHandle( addD3DResource( D3DTexture ) );
			return true;
		}
		break;

	case RsTextureType::TEXCUBE:
		{
			D3D11_TEXTURE2D_DESC Desc;
			Desc.Width = TextureDesc.Width_;
			Desc.Height = TextureDesc.Height_;
			Desc.MipLevels = TextureDesc.Levels_;
			Desc.ArraySize = 6;
			Desc.Format = gTextureFormats[ (BcU32)TextureDesc.Format_ ];
			Desc.SampleDesc.Count = 1;
			Desc.SampleDesc.Quality = 0;
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture2D* D3DTexture = nullptr;
			Result = Device_->CreateTexture2D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );

			Texture->setHandle( addD3DResource( D3DTexture ) );
			return true;
		}
		break;
	}
	
	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextD3D11::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	delD3DResource( Texture->getHandle< BcU32 >() );

	return true;	
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
bool RsContextD3D11::updateTexture( 
	class RsTexture* Texture,
	const struct RsTextureSlice& Slice,
	RsResourceUpdateFlags Flags,
	RsTextureUpdateFunc UpdateFunc )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcPrintf( "WARNING: RsContextD3D11::updateTexture unimplemented\n" );

	RsTextureLock Lock;
	Lock.Buffer_ = nullptr;
	Lock.Pitch_ = 0;
	Lock.SlicePitch_ = 0;
	UpdateFunc( Texture, Lock );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextD3D11::createShader(
	class RsShader* Shader )
{
	const auto& Desc = Shader->getDesc();
	ID3D11DeviceChild* D3DShader = nullptr;  

	HRESULT Result = 0;
	switch( Desc.ShaderType_ )
	{
	case RsShaderType::VERTEX:
		{
			Result = Device_->CreateVertexShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11VertexShader** >( &D3DShader ) );
		}
		break;

	case RsShaderType::TESSELATION_CONTROL:
		{
			Result = Device_->CreateHullShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11HullShader** >( &D3DShader ) );
		}
		break;

	case RsShaderType::TESSELATION_EVALUATION:
		{
			Result = Device_->CreateDomainShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11DomainShader** >( &D3DShader ) );
		}
		break;

	case RsShaderType::GEOMETRY:
		{
			Result = Device_->CreateGeometryShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11GeometryShader** >( &D3DShader ) );
		}
		break;

	case RsShaderType::FRAGMENT:
		{
			Result = Device_->CreatePixelShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11PixelShader** >( &D3DShader ) );
		}
		break;

	case RsShaderType::COMPUTE:
		{
			Result = Device_->CreateComputeShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11ComputeShader** >( &D3DShader ) );
		}
		break;
	}

	BcAssert( SUCCEEDED( Result ) );

	if( SUCCEEDED( Result ) )
	{
		Shader->setHandle( D3DShader );
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextD3D11::destroyShader(
	class RsShader* Shader )
{
	ID3D11DeviceChild* D3DShader = Shader->getHandle< ID3D11DeviceChild* >();

	if( D3DShader != nullptr )
	{
		D3DShader->Release();
		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextD3D11::createProgram(
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
		ID3D11ShaderReflection* Reflector = nullptr; 
		D3D11Reflect( 
			Shader->getData(), 
			Shader->getDataSize(), 
			&Reflector);

		const BcU32 ShiftAmount = ( (BcU32)ShaderDesc.ShaderType_ * BitsPerShader );
		const BcU32 MaskOff = ~( NoofBindPoints << ShiftAmount );

		// Just iterate over a big number...we'll assert if we go over.
		for( BcU32 Idx = 0; Idx < 128; ++Idx )
		{
			D3D11_SHADER_INPUT_BIND_DESC BindDesc;
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
					D3D11_SHADER_BUFFER_DESC BufferDesc;
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
		Program->addUniformBufferSlot( 
			ConstantBuffer.first,
			ConstantBuffer.second,
			Size );
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
bool RsContextD3D11::destroyProgram(
	class RsProgram* Program )
{
	return false;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D11::flushState()
{
	HRESULT Result = 0;

	// Bind shaders.
	RsShader* VertexShader = nullptr;
	std::array< bool, (BcU32)RsShaderType::MAX > HasShaders = { false };
	if( Program_ != nullptr )
	{
		const auto& Shaders = Program_->getShaders();
		for( auto* Shader : Shaders )
		{
			const auto& Desc = Shader->getDesc();
			BcU32 ShaderTypeIdx = (BcU32)Desc.ShaderType_;
			HasShaders[ ShaderTypeIdx ] = true;
			switch( Desc.ShaderType_ )
			{
			case RsShaderType::VERTEX:
				{
					// Cache for input assembly.
					VertexShader = Shader;

					// Bind.
					Context_->VSSetShader( Shader->getHandle< ID3D11VertexShader* >(), nullptr, 0 );
					for( BcU32 Idx = 0; Idx < D3DConstantBuffers_[ ShaderTypeIdx ].size(); ++Idx )
					{
						if( D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] != nullptr )
						{
							Context_->VSSetConstantBuffers( 
								0,
								1,							
								&D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] );
						}
					}
				}
				break;

			case RsShaderType::TESSELATION_CONTROL:
				{
					// Bind.
					Context_->HSSetShader( Shader->getHandle< ID3D11HullShader* >(), nullptr, 0 );
					Context_->VSSetShader( Shader->getHandle< ID3D11VertexShader* >(), nullptr, 0 );
					for( BcU32 Idx = 0; Idx < D3DConstantBuffers_[ ShaderTypeIdx ].size(); ++Idx )
					{
						if( D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] != nullptr )
						{
							Context_->HSSetConstantBuffers( 
								0,
								1,							
								&D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] );
						}
					}
				}
				break;

			case RsShaderType::TESSELATION_EVALUATION:
				{
					// Bind.
					Context_->DSSetShader( Shader->getHandle< ID3D11DomainShader* >(), nullptr, 0 );
					Context_->VSSetShader( Shader->getHandle< ID3D11VertexShader* >(), nullptr, 0 );
					for( BcU32 Idx = 0; Idx < D3DConstantBuffers_[ ShaderTypeIdx ].size(); ++Idx )
					{
						if( D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] != nullptr )
						{
							Context_->DSSetConstantBuffers( 
								0,
								1,							
								&D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] );
						}
					}
				}
				break;

			case RsShaderType::GEOMETRY:
				{
					// Bind.
					Context_->GSSetShader( Shader->getHandle< ID3D11GeometryShader* >(), nullptr, 0 );
					Context_->VSSetShader( Shader->getHandle< ID3D11VertexShader* >(), nullptr, 0 );
					for( BcU32 Idx = 0; Idx < D3DConstantBuffers_[ ShaderTypeIdx ].size(); ++Idx )
					{
						if( D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] != nullptr )
						{
							Context_->GSSetConstantBuffers( 
								0,
								1,							
								&D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] );
						}
					}
				}
				break;

			case RsShaderType::FRAGMENT:
				{
					// Bind.
					Context_->PSSetShader( Shader->getHandle< ID3D11PixelShader* >(), nullptr, 0 );
					Context_->VSSetShader( Shader->getHandle< ID3D11VertexShader* >(), nullptr, 0 );
					for( BcU32 Idx = 0; Idx < D3DConstantBuffers_[ ShaderTypeIdx ].size(); ++Idx )
					{
						if( D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] != nullptr )
						{
							Context_->PSSetConstantBuffers( 
								0,
								1,							
								&D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] );
						}
					}
				}
				break;

			case RsShaderType::COMPUTE:
				{
					// Bind.
					Context_->CSSetShader( Shader->getHandle< ID3D11ComputeShader* >(), nullptr, 0 );
					Context_->VSSetShader( Shader->getHandle< ID3D11VertexShader* >(), nullptr, 0 );
					for( BcU32 Idx = 0; Idx < D3DConstantBuffers_[ ShaderTypeIdx ].size(); ++Idx )
					{
						if( D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] != nullptr )
						{
							Context_->CSSetConstantBuffers( 
								0,
								1,							
								&D3DConstantBuffers_[ ShaderTypeIdx ][ Idx ] );
						}
					}
				}
				break;
			}
		}

		// Disable unused pipeline stages.
		for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
		{
			if( HasShaders[ Idx ] == false )
			{
				switch( Idx )
				{
				case RsShaderType::VERTEX:
					Context_->VSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::TESSELATION_CONTROL:
					Context_->HSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::TESSELATION_EVALUATION:
					Context_->DSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::GEOMETRY:
					Context_->GSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::FRAGMENT:
					Context_->PSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::COMPUTE:
					Context_->CSSetShader( nullptr, nullptr, 0 );
					break;

				}
			}
		}
	}

	// Input layout.
	if( InputLayoutChanged_ )
	{
		// Grab current input layout.
		BcU32 InputLayoutHash = generateInputLayoutHash();

		auto FoundInputLayout = InputLayoutMap_.find( InputLayoutHash );
		if( FoundInputLayout == InputLayoutMap_.end() )
		{
			const auto& Desc = VertexDeclaration_->getDesc();
			const auto& VertexAttributes = Program_->getVertexAttributeList();

			// Create input layout for current setup.
			// For missing vertex attributes, we just fudge them to be zero offset.
			std::vector< D3D11_INPUT_ELEMENT_DESC > ElementDescs;
			ElementDescs.reserve( Desc.Elements_.size() );

			for( const auto& VertexAttribute : VertexAttributes )
			{
				auto FoundElement = std::find_if( Desc.Elements_.begin(), Desc.Elements_.end(),
					[ & ]( const RsVertexElement& Element )
					{
						return ( Element.Usage_ == VertexAttribute.Usage_ &&
							Element.UsageIdx_ == VertexAttribute.UsageIdx_ );
					} );

				// Force to an element with zero offset if we can't find a valid one.
				// TODO: Find a better approach.
				if( FoundElement == Desc.Elements_.end() )
				{
					FoundElement = std::find_if( Desc.Elements_.begin(), Desc.Elements_.end(),
					[ & ]( const RsVertexElement& Element )
					{
						return Element.Offset_ == 0;
					} );
				}

				D3D11_INPUT_ELEMENT_DESC ElementDesc;
				ElementDesc.SemanticName = gSemanticName[ (BcU32)VertexAttribute.Usage_ ];
				ElementDesc.SemanticIndex = VertexAttribute.UsageIdx_;
				ElementDesc.Format = getVertexElementFormat( *FoundElement );
				ElementDesc.InputSlot = FoundElement->StreamIdx_;
				ElementDesc.AlignedByteOffset = FoundElement->Offset_;
				ElementDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA; // TODO: Instancing support.
				ElementDesc.InstanceDataStepRate = 0;
				ElementDescs.push_back( ElementDesc );
			}

			InputLayout InputLayoutEntry = 
			{
				nullptr,
				0
			};
			
			Result = Device_->CreateInputLayout( &ElementDescs[ 0 ], ElementDescs.size(), VertexShader->getData(), VertexShader->getDataSize(), &InputLayoutEntry.InputLayout_ );
			BcAssert( SUCCEEDED( Result ) );
			
			InputLayoutMap_[ InputLayoutHash ] = InputLayoutEntry;
			FoundInputLayout = InputLayoutMap_.find( InputLayoutHash );
		}

		// Set input layout.
		auto& InputLayout( FoundInputLayout->second );
		Context_->IASetInputLayout( InputLayout.InputLayout_ );

		// Set topology.
		Context_->IASetPrimitiveTopology(
			gTopologyType[ (BcU32)TopologyType_ ] );
	}

	// Set buffers.
	// TODO: Redundancy.
	Context_->IASetIndexBuffer( 
		D3DIndexBuffer_,
		DXGI_FORMAT_R16_UINT, // TODO: Investigate correct formats.
		0 );

	Context_->IASetVertexBuffers( 
		0, 
		D3DVertexBuffers_.size(), 
		&D3DVertexBuffers_[ 0 ],
		&D3DVertexBufferStrides_[ 0 ],
		&D3DVertexBufferOffsets_[ 0 ] );

}

//////////////////////////////////////////////////////////////////////////
// addD3DResource
BcU32 RsContextD3D11::addD3DResource( ID3D11Resource* D3DResource )
{
	ResourceViewCacheEntry Entry = {};

	// If the cache is empty, push another entry into it.
	if( ResourceViewCacheFreeIdx_.size() == 0 )
	{
		ResourceViewCacheFreeIdx_.push_back( ResourceViewCache_.size() );
		ResourceViewCache_.push_back( Entry );
	}

	// Get entry from back of the cache, and pop it off.
	BcU32 EntryIdx = ResourceViewCacheFreeIdx_.back();
	ResourceViewCacheFreeIdx_.pop_back();

	// Setup entry.
	Entry.Resource_ = D3DResource;

	// Store in cache.
	ResourceViewCache_[ EntryIdx ] = Entry;

	return EntryIdx;
}

//////////////////////////////////////////////////////////////////////////
// delD3DResource
void RsContextD3D11::delD3DResource( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	
	// Release entry.
	BcAssert( Entry.Resource_ != nullptr );
	Entry.Resource_->Release();

	// Release views.
	if( Entry.ShaderResourceView_ != nullptr )
	{
		Entry.ShaderResourceView_->Release();
	}
	if( Entry.UnorderedAccessView_ != nullptr )
	{
		Entry.UnorderedAccessView_->Release();
	}
	if( Entry.RenderTargetView_ != nullptr )
	{
		Entry.RenderTargetView_->Release();
	}
	if( Entry.DepthStencilView_ != nullptr )
	{
		Entry.DepthStencilView_->Release();
	}

	BcMemZero( &Entry, sizeof( Entry ) );

	// Put in free list.
	ResourceViewCacheFreeIdx_.push_back( ResourceIdx );
}

//////////////////////////////////////////////////////////////////////////
// getD3DResource
ID3D11Resource* RsContextD3D11::getD3DResource( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	return Entry.Resource_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DBuffer
ID3D11Buffer* RsContextD3D11::getD3DBuffer( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
#if !PSY_PRODUCTION
	D3D11_RESOURCE_DIMENSION ResDim;
	Entry.Resource_->GetType( &ResDim );
	BcAssert( ResDim == D3D11_RESOURCE_DIMENSION_BUFFER );
#endif
	return Entry.BufferResource_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DTexture1D
ID3D11Texture1D* RsContextD3D11::getD3DTexture1D( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
#if !PSY_PRODUCTION
	D3D11_RESOURCE_DIMENSION ResDim;
	Entry.Resource_->GetType( &ResDim );
	BcAssert( ResDim == D3D11_RESOURCE_DIMENSION_TEXTURE1D );
#endif
	return Entry.Texture1DResource_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DTexture2D
ID3D11Texture2D* RsContextD3D11::getD3DTexture2D( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
#if !PSY_PRODUCTION
	D3D11_RESOURCE_DIMENSION ResDim;
	Entry.Resource_->GetType( &ResDim );
	BcAssert( ResDim == D3D11_RESOURCE_DIMENSION_TEXTURE2D );
#endif
	return Entry.Texture2DResource_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DTexture3D
ID3D11Texture3D* RsContextD3D11::getD3DTexture3D( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
#if !PSY_PRODUCTION
	D3D11_RESOURCE_DIMENSION ResDim;
	Entry.Resource_->GetType( &ResDim );
	BcAssert( ResDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D );
#endif
	return Entry.Texture3DResource_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DShaderResourceView
ID3D11ShaderResourceView* RsContextD3D11::getD3DShaderResourceView( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	if( Entry.ShaderResourceView_ == nullptr )
	{
		//
		D3D11_RESOURCE_DIMENSION ResDim;
		Entry.Resource_->GetType( &ResDim );
		
		// Create shader resource view.
		D3D11_SHADER_RESOURCE_VIEW_DESC Desc;

		switch( ResDim )
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			{
				D3D11_TEXTURE1D_DESC TexDesc;
				Entry.Texture1DResource_->GetDesc( &TexDesc );
				Desc.Format = TexDesc.Format;
				Desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D;
				Desc.Texture1D.MipLevels = TexDesc.MipLevels;
				Desc.Texture1D.MostDetailedMip = 0;
			}
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				D3D11_TEXTURE2D_DESC TexDesc;
				Entry.Texture2DResource_->GetDesc( &TexDesc );
				Desc.Format = TexDesc.Format;
				Desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
				Desc.Texture2D.MipLevels = TexDesc.MipLevels;
				Desc.Texture2D.MostDetailedMip = 0;
			}
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			{
				D3D11_TEXTURE3D_DESC TexDesc;
				Entry.Texture3DResource_->GetDesc( &TexDesc );
				Desc.Format = TexDesc.Format;
				Desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE3D;
				Desc.Texture3D.MipLevels = TexDesc.MipLevels;
				Desc.Texture3D.MostDetailedMip = 0;
			}
			break;

		default:
			BcBreakpoint;
			return nullptr;
			break;
		}

		// Create shader resource view.
		HRESULT Result = Device_->CreateShaderResourceView( 
			Entry.Resource_,
			&Desc,
			&Entry.ShaderResourceView_ );
		BcAssert( SUCCEEDED( Result ) );
	}

	return Entry.ShaderResourceView_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DUnorderedAccessView
ID3D11UnorderedAccessView* RsContextD3D11::getD3DUnorderedAccessView( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	if( Entry.UnorderedAccessView_ == nullptr )
	{
		// Create unordered access view.
		BcBreakpoint;
	}

	return Entry.UnorderedAccessView_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DRenderTargetView
ID3D11RenderTargetView* RsContextD3D11::getD3DRenderTargetView( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	if( Entry.RenderTargetView_ == nullptr )
	{
		// Check resource dimensions.
		D3D11_RESOURCE_DIMENSION ResDim;
		Entry.Resource_->GetType( &ResDim );

		// Create render target view.
		D3D11_RENDER_TARGET_VIEW_DESC Desc;

		switch( ResDim )
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				D3D11_TEXTURE2D_DESC TexDesc;
				Entry.Texture2DResource_->GetDesc( &TexDesc );
				Desc.Format = TexDesc.Format;
				Desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
				Desc.Texture2D.MipSlice = 0;
			}
			break;

		default:
			BcBreakpoint;
			return nullptr;
			break;
		}

		HRESULT Result = Device_->CreateRenderTargetView(
			Entry.Resource_,
			&Desc,
			&Entry.RenderTargetView_ );
		BcAssert( SUCCEEDED( Result ) ); 
	}

	return Entry.RenderTargetView_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DDepthStencilView
ID3D11DepthStencilView* RsContextD3D11::getD3DDepthStencilView( BcU32 ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	if( Entry.DepthStencilView_ == nullptr )
	{
		// Check resource dimensions.
		D3D11_RESOURCE_DIMENSION ResDim;
		Entry.Resource_->GetType( &ResDim );

		// Create depth stencil view.
		D3D11_DEPTH_STENCIL_VIEW_DESC Desc;

		switch( ResDim )
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				D3D11_TEXTURE2D_DESC TexDesc;
				Entry.Texture2DResource_->GetDesc( &TexDesc );
				Desc.Format = TexDesc.Format;
				Desc.Texture2D.MipSlice = 0;
			}
			break;

		default:
			BcBreakpoint;
			return nullptr;
			break;
		}

		HRESULT Result = Device_->CreateDepthStencilView(
			Entry.Resource_,
			&Desc,
			&Entry.DepthStencilView_ );
		BcAssert( SUCCEEDED( Result ) ); 
	}

	return Entry.DepthStencilView_;
}

//////////////////////////////////////////////////////////////////////////
// generateInputLayoutHash
//virtual
BcU32 RsContextD3D11::generateInputLayoutHash() const
{
	BcU32 Hash = 0;
	BcU64 VertexDeclHandle = VertexDeclaration_->getHandle< BcU64 >();
	BcU64 VertexShaderHandle = 0;// Shaders_[ VERTEX ]->getHandle< BcU64 >();

	Hash = BcHash::GenerateCRC32( Hash, &VertexDeclHandle, sizeof( VertexDeclHandle ) );
	Hash = BcHash::GenerateCRC32( Hash, &VertexShaderHandle, sizeof( VertexShaderHandle ) );

	return Hash;
}