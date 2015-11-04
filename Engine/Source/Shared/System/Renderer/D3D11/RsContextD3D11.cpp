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
#include "System/Renderer/D3D11/RsProgramD3D11.h"

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
#include "Base/BcProfiler.h"

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
static BcU32 GetBindFlags( RsResourceBindFlags Flags )
{
	BcU32 RetVal = 0;
	if( ( Flags & RsResourceBindFlags::VERTEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		RetVal |= D3D11_BIND_VERTEX_BUFFER;
	}
	if( ( Flags & RsResourceBindFlags::INDEX_BUFFER ) != RsResourceBindFlags::NONE )
	{
		RetVal |= D3D11_BIND_INDEX_BUFFER;
	}
	if( ( Flags & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE )
	{
		RetVal |= D3D11_BIND_CONSTANT_BUFFER;
	}
	if( ( Flags & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		RetVal |= D3D11_BIND_UNORDERED_ACCESS;
	}
	if( ( Flags & RsResourceBindFlags::STREAM_OUTPUT ) != RsResourceBindFlags::NONE )
	{
		RetVal |= D3D11_BIND_STREAM_OUTPUT;
	}
	if( ( Flags & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		RetVal |= D3D11_BIND_RENDER_TARGET;
	}
	if( ( Flags & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		RetVal |= D3D11_BIND_DEPTH_STENCIL;
	}
	return RetVal;
}

// Texture formats
static const DXGI_FORMAT gTextureFormats[] =
{
	DXGI_FORMAT_UNKNOWN,

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
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::ETC1,

	// Depth.
	DXGI_FORMAT_R16_TYPELESS,			// RsTextureFormat::D16,
	DXGI_FORMAT_R24G8_TYPELESS,			// RsTextureFormat::D24,
	DXGI_FORMAT_R32_TYPELESS,			// RsTextureFormat::D32,
	DXGI_FORMAT_R24G8_TYPELESS,			// RsTextureFormat::D24S8,
};

// Depth stencil view formats.
static const DXGI_FORMAT gDSVFormats[] =
{
	DXGI_FORMAT_UNKNOWN,

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
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::ETC1,
										// Depth.
	DXGI_FORMAT_D16_UNORM,				// RsTextureFormat::D16,
	DXGI_FORMAT_D24_UNORM_S8_UINT,		// RsTextureFormat::D24,
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::D32,
	DXGI_FORMAT_D24_UNORM_S8_UINT,		// RsTextureFormat::D24S8,
};

// Shader resource view formats.
static const DXGI_FORMAT gSRVFormats[] = 
{
	DXGI_FORMAT_UNKNOWN,

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
	DXGI_FORMAT_UNKNOWN,				// RsTextureFormat::ETC1,
	// Depth.
	DXGI_FORMAT_R16_UNORM,				// RsTextureFormat::D16,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,	// RsTextureFormat::D24,
	DXGI_FORMAT_R32_FLOAT,				// RsTextureFormat::D32,
	DXGI_FORMAT_R24_UNORM_X8_TYPELESS,	// RsTextureFormat::D24S8,
};

static const D3D11_PRIMITIVE_TOPOLOGY gTopologyType[] =
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

static const D3D11_FILL_MODE gFillMode[] =
{
	D3D11_FILL_SOLID,
	D3D11_FILL_WIREFRAME,
};

static const D3D11_CULL_MODE gCullMode[] =
{
	D3D11_CULL_NONE,
	D3D11_CULL_BACK,
	D3D11_CULL_FRONT,
};

static const D3D11_COMPARISON_FUNC gCompareFunc[] =
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

static const D3D11_STENCIL_OP gStencilOp[] =
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

static const D3D11_TEXTURE_ADDRESS_MODE gTextureAddressMode[] =
{
	D3D11_TEXTURE_ADDRESS_WRAP,		// RsTextureSamplingMode::WRAP
	D3D11_TEXTURE_ADDRESS_MIRROR,	// RsTextureSamplingMode::MIRROR
	D3D11_TEXTURE_ADDRESS_CLAMP,	// RsTextureSamplingMode::CLAMP
	D3D11_TEXTURE_ADDRESS_BORDER,	// RsTextureSamplingMode::DECAL
};

static const D3D11_BLEND gBlendType[] =
{
    D3D11_BLEND_ZERO,
    D3D11_BLEND_ONE,
    D3D11_BLEND_SRC_COLOR,
    D3D11_BLEND_INV_SRC_COLOR,
    D3D11_BLEND_SRC_ALPHA,
    D3D11_BLEND_INV_SRC_ALPHA,
    D3D11_BLEND_DEST_COLOR,
    D3D11_BLEND_INV_DEST_COLOR,
    D3D11_BLEND_DEST_ALPHA,
    D3D11_BLEND_INV_DEST_ALPHA,
};

static const D3D11_BLEND_OP gBlendOp[] =
{
    D3D11_BLEND_OP_ADD,
    D3D11_BLEND_OP_SUBTRACT,
    D3D11_BLEND_OP_REV_SUBTRACT,
    D3D11_BLEND_OP_MIN,
    D3D11_BLEND_OP_MAX,
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
	InsideBeginEnd_( 0 ),
	Width_( 0 ),
	Height_( 0 ),
	Adapter_( nullptr ),
	Device_( nullptr ),
	Context_( nullptr ),
	BackBufferRT_( nullptr ),
	BackBufferDS_( nullptr ),
	ScreenshotRequested_( BcFalse ),
	OwningThread_( BcErrorCode ),
	UniformBuffersDirty_( BcTrue ),
	TexturesDirty_( BcTrue ),
	FrameBuffer_( nullptr ),
	FrameBufferDirty_( BcTrue )
{
	IndexBuffer_ = nullptr;
	D3DIndexBuffer_ = nullptr;
	BcMemZero( &VertexBuffers_[ 0 ], sizeof( VertexBuffers_ ) );
	BcMemZero( &D3DVertexBuffers_[ 0 ], sizeof( D3DVertexBuffers_ ) );
	BcMemZero( &D3DVertexBufferStrides_[ 0 ], sizeof( D3DVertexBufferStrides_ ) );
	BcMemZero( &D3DVertexBufferOffsets_[ 0 ], sizeof( D3DVertexBufferOffsets_ ) );
	BcMemZero( &UniformBuffers_[ 0 ], sizeof( UniformBuffers_ ) );
	BcMemZero( &D3DConstantBuffers_[ 0 ], sizeof( D3DConstantBuffers_ ) );
	BcMemZero( &Textures_[ 0 ], sizeof( Textures_ ) );
	BcMemZero( &D3DShaderResourceViews_[ 0 ], sizeof( D3DShaderResourceViews_ ) );
	BcMemZero( &D3DSamplerStates_[ 0 ], sizeof( D3DSamplerStates_ ) );

	InputLayoutDirty_ = BcFalse;
	Program_ = nullptr;
	VertexDeclaration_ = nullptr;
	TopologyType_ = RsTopologyType::INVALID;

	FrameCounter_ = 0;

	BoundRenderState_ = nullptr;
	
	BcMemZero( &RenderTargetViews_, sizeof( RenderTargetViews_ ) );
	BcMemZero( &DepthStencilView_, sizeof( DepthStencilView_ ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsContextD3D11::~RsContextD3D11()
{

}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextD3D11::getClient() const
{
	return pClient_;
}

//////////////////////////////////////////////////////////////////////////
// getFeatures
//virtual
const RsFeatures& RsContextD3D11::getFeatures() const
{
	return Features_;
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextD3D11::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
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
// getWidth
//virtual
BcU32 RsContextD3D11::getWidth() const
{
	BcAssert( InsideBeginEnd_ == 1 );
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextD3D11::getHeight() const
{
	BcAssert( InsideBeginEnd_ == 1 );
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// beginFrame
void RsContextD3D11::beginFrame( BcU32 Width, BcU32 Height )
{
	BcAssert( InsideBeginEnd_ == 0 );
	++InsideBeginEnd_;

	// Resize swap chain.
	if( Width_ != Width || Height_ != Height )
	{
		// Default formats.
		auto BackBufferRTFormat = RsTextureFormat::R8G8B8A8; // TODO: Get for BackBufferDesc.Format...
		auto BackBufferDSFormat = RsTextureFormat::D24S8;

		// Free old buffers.
		if( BackBufferRT_ != nullptr )
		{
			delete BackBufferRT_;
			BackBufferRT_ = nullptr;
			delD3DResource( BackBufferRTResourceIdx_ );
			BackBufferRTResourceIdx_ = BcErrorCode;
		}

		if( BackBufferDS_ != nullptr )
		{
			delete BackBufferDS_;
			BackBufferDS_ = nullptr;
			delD3DResource( BackBufferDSResourceIdx_ );
			BackBufferDSResourceIdx_ = BcErrorCode;
		}

		// Resize buffers.
		HRESULT Result = SwapChain_->ResizeBuffers( 
			SwapChainDesc_.BufferCount, Width, Height, 
			SwapChainDesc_.BufferDesc.Format, 
			SwapChainDesc_.Flags );
		BcAssert( SUCCEEDED( Result ) );
		
		// Get back buffer from swap chain.
		SwapChain_->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer_ );
		D3D11_TEXTURE2D_DESC BackBufferDesc;
		BackBuffer_->GetDesc( &BackBufferDesc );
		BackBufferRTResourceIdx_ = addD3DResource( 
			BackBuffer_,
			BackBufferDesc.Format,
			BackBufferDesc.Format,
			BackBufferDesc.Format );

		// Create back buffer RT.
		BackBufferRT_ = new RsTexture(
				this,
				RsTextureDesc( 
					RsTextureType::TEX2D,
					RsResourceCreationFlags::STATIC, 
					RsResourceBindFlags::RENDER_TARGET,
					BackBufferRTFormat, 1,
					Width,
					Height,
					1 ) );
		BackBufferRT_->setHandle< size_t >( BackBufferRTResourceIdx_ );

		// Create back buffer DS.
		BackBufferDS_ = new RsTexture(
				this,
				RsTextureDesc( 
					RsTextureType::TEX2D,
					RsResourceCreationFlags::STATIC, 
					RsResourceBindFlags::DEPTH_STENCIL,
					BackBufferDSFormat, 1,
					Width,
					Height,
					1 ) );
		const auto& TextureDesc = BackBufferDS_->getDesc();
		D3D11_TEXTURE2D_DESC Desc;
		Desc.Width = TextureDesc.Width_;
		Desc.Height = TextureDesc.Height_;
		Desc.MipLevels = TextureDesc.Levels_;
		Desc.ArraySize = 1;
		Desc.Format = gTextureFormats[ (BcU32)TextureDesc.Format_ ];
		Desc.SampleDesc.Count = 1;
		Desc.SampleDesc.Quality = 0;
		Desc.Usage = D3D11_USAGE_DEFAULT;
		Desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		Desc.CPUAccessFlags = 0;
		Desc.MiscFlags = 0;

		ID3D11Texture2D* D3DTexture = nullptr;
 		Result = Device_->CreateTexture2D( &Desc, nullptr, &D3DTexture );
		BcAssert( SUCCEEDED( Result ) );
		BackBufferDS_->setHandle( addD3DResource( 
			D3DTexture,
			gTextureFormats[ (BcU32)TextureDesc.Format_ ],
			gDSVFormats[ (BcU32)TextureDesc.Format_ ],
			gSRVFormats[ (BcU32)TextureDesc.Format_ ] ) );
		BackBufferDSResourceIdx_ = BackBufferDS_->getHandle< BcU32 >();

		RenderTargetViews_.fill( nullptr );
		DepthStencilView_ = nullptr;
	}

	Width_ = Width;
	Height_ = Height;

	setDefaultState();
	setFrameBuffer( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextD3D11::endFrame()
{
	BcAssert( InsideBeginEnd_ == 1 );
	--InsideBeginEnd_;

	SwapChain_->Present( 0, 0 );

	++FrameCounter_;
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextD3D11::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
	BcBreakpoint;
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
	SwapChainDesc_.OutputWindow = (HWND)pClient->getWindowHandle();
	SwapChainDesc_.SampleDesc.Count = 1;
	SwapChainDesc_.SampleDesc.Quality = 0;
    SwapChainDesc_.Windowed = TRUE;
	SwapChainDesc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Flags.
	UINT CreateDeviceFlags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#if PSY_DEBUG
	CreateDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create device and swap chain.
	FeatureLevel_ = D3D_FEATURE_LEVEL_11_1;
#if PLATFORM_WINDOWS
	HRESULT Result = D3D11CreateDeviceAndSwapChain( 
		Adapter_.Get(),
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		CreateDeviceFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&SwapChainDesc_,
		&SwapChain_,
		&Device_,
		&FeatureLevel_,
		&Context_ );
	BcAssert( SUCCEEDED( Result ) ); 
#else
	BcBreakpoint;
#endif // PLATFORM_WINDOWS

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	// Check threading feature.
	D3D11_FEATURE_DATA_THREADING FeatureThreading;
	Device_->CheckFeatureSupport(
		D3D11_FEATURE_THREADING,
		&FeatureThreading,
		sizeof( FeatureThreading ) );

	// Fill in features.
	D3D11_FEATURE_DATA_D3D9_OPTIONS D3D9Options;
	Device_->CheckFeatureSupport(
		D3D11_FEATURE_D3D9_OPTIONS,
		&D3D9Options,
		sizeof( D3D9Options ) );

	Features_.MRT_ = true;
	Features_.DepthTextures_ = true;
	Features_.NPOTTextures_ = D3D9Options.FullNonPow2TextureSupport ? true : false;
	Features_.SeparateBlendState_ = true;
	Features_.AnisotropicFiltering_ = true;
	Features_.AntialiasedLines_ = true;
	Features_.Texture1D_ = true;
	Features_.Texture2D_ = true;
	Features_.Texture3D_ = true;
	Features_.TextureCube_ = true;

	// Compute shader support.
	Features_.ComputeShaders_ = FeatureLevel_ >= D3D_FEATURE_LEVEL_11_0;

	for( int Format = 0; Format < (int)RsTextureFormat::MAX; ++Format )
	{
		DXGI_FORMAT DXTextureFormat = gTextureFormats[ Format ];
		DXGI_FORMAT DXRTVFormat = gTextureFormats[ Format ];
		DXGI_FORMAT DXDSVFormat = gDSVFormats[ Format ];
		if( DXTextureFormat != DXGI_FORMAT_UNKNOWN )
		{
			UINT FormatSupport = 0;
			if( SUCCEEDED( Device_->CheckFormatSupport( DXTextureFormat, &FormatSupport ) ) )
			{
				if( FormatSupport & D3D11_FORMAT_SUPPORT_TEXTURE2D )
				{
					Features_.TextureFormat_[ Format ] = true;
				}
				if( FormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET )
				{
					Features_.RenderTargetFormat_[ Format ] = true;
				}
			}
		}
		if( DXRTVFormat != DXGI_FORMAT_UNKNOWN )
		{
			UINT FormatSupport = 0;
			if( SUCCEEDED( Device_->CheckFormatSupport( DXRTVFormat, &FormatSupport ) ) )
			{
				if( FormatSupport & D3D11_FORMAT_SUPPORT_RENDER_TARGET )
				{
					Features_.RenderTargetFormat_[ Format ] = true;
				}
			}
		}
		if( DXDSVFormat != DXGI_FORMAT_UNKNOWN )
		{
			UINT FormatSupport = 0;
			if( SUCCEEDED( Device_->CheckFormatSupport( DXDSVFormat, &FormatSupport ) ) )
			{
				if( FormatSupport & D3D11_FORMAT_SUPPORT_DEPTH_STENCIL )
				{
					Features_.DepthStencilTargetFormat_[ Format ] = true;
				}
			}
		}
	}

	setDefaultState();
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
	destroyTexture( BackBufferRT_ );
	delete BackBufferRT_;
	BackBufferRT_ = nullptr;

	destroyTexture( BackBufferDS_ );
	delete BackBufferDS_;
	BackBufferDS_ = nullptr;

	BackBuffer_ = nullptr;

	// Verify all resources have been freed.
#if !defined( PSY_PRODUCTION )
	for( const auto& ResourceView : ResourceViewCache_ )
	{
		BcAssert( ResourceView.Resource_ == nullptr );
	}
#endif

	ResourceViewCache_.clear();
	ResourceViewCacheFreeIdx_.clear();

	InputLayoutMap_.clear();
	BlendStateCache_.clear();
	RasterizerStateCache_.clear();
	DepthStencilStateCache_.clear();
	SamplerStateCache_.clear();

	SwapChain_ = nullptr;
	Device_ = nullptr;
	Context_ = nullptr;
}

//////////////////////////////////////////////////////////////////////////
// setDefaultState
void RsContextD3D11::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
#if 0
	// Blend state.
	BlendState_.AlphaToCoverageEnable = FALSE;
	BlendState_.IndependentBlendEnable = FALSE;
	const D3D11_RENDER_TARGET_BLEND_DESC DefaultRenderTargetBlendDesc =
	{
		FALSE,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD,
		D3D11_COLOR_WRITE_ENABLE_ALL,
	};
	for (UINT i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		BlendState_.RenderTarget[ i ] = DefaultRenderTargetBlendDesc;
	}

	// Rasterizer state.
	RasterizerState_.FillMode = D3D11_FILL_SOLID;
	RasterizerState_.CullMode = D3D11_CULL_NONE;
	RasterizerState_.FrontCounterClockwise = FALSE;
	RasterizerState_.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
	RasterizerState_.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
	RasterizerState_.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	RasterizerState_.DepthClipEnable = TRUE;
	RasterizerState_.ScissorEnable = FALSE;
	RasterizerState_.MultisampleEnable = FALSE;
	RasterizerState_.AntialiasedLineEnable = TRUE;

	// Depth stencil state.
	DepthStencilState_.DepthEnable = TRUE;
	DepthStencilState_.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	DepthStencilState_.DepthFunc = D3D11_COMPARISON_LESS;
	DepthStencilState_.StencilEnable = FALSE;
	DepthStencilState_.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	DepthStencilState_.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	const D3D11_DEPTH_STENCILOP_DESC defaultStencilOp =
		{ D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_STENCIL_OP_KEEP, D3D11_COMPARISON_ALWAYS };
	DepthStencilState_.FrontFace = defaultStencilOp;
	DepthStencilState_.BackFace = defaultStencilOp;
#endif
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextD3D11::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//PSY_LOG( "WARNING: RsContextD3D11::invalidateRenderState unimplemented\n" );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextD3D11::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//PSY_LOG( "WARNING: RsContextD3D11::invalidateTextureState unimplemented\n" );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D11::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
			
	BoundRenderState_ = RenderState;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextD3D11::setSamplerState( BcU32 Handle, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	auto D3DSamplerState = SamplerStateCache_[ SamplerState->getHandle< BcU32 >() ];
	
	// Bind for each shader based on specified handle.
	if( Handle != BcErrorCode )
	{
		auto& SamplerStateSlot = D3DSamplerStates_[ Handle ];
		if( D3DSamplerState.Sampler_ != SamplerStateSlot.Sampler_ )
		{
			TexturesDirty_ = BcTrue;
			SamplerStateSlot.Dirty_ = BcTrue;
		}
		SamplerStateSlot.Sampler_ = D3DSamplerState.Sampler_;
		SamplerStateSlot.Dirty_ = BcTrue;
	}
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextD3D11::setSamplerState( BcU32 Handle, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextD3D11::setTexture( BcU32 Handle, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( pTexture != nullptr )
	{
		BcAssertMsg( ( pTexture->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE,
			"Texture can't be bound as a shader resource. Has it been created with RsResourceBindFlags::SHADER_RESOURCE?" );
	}

	// Find shader resource view.
	ID3D11ShaderResourceView* ShaderResourceView = 
		pTexture ? getD3DShaderResourceView( pTexture->getHandle< BcU32 >() ) : nullptr;

	// Bind for each shader based on specified handle.
	if( Handle != BcErrorCode )
	{
		if( pTexture != Textures_[ Handle ] )
		{
			TexturesDirty_ = BcTrue;
		}
		auto& ShaderResourceViewSlot = D3DShaderResourceViews_[ Handle ];
		Textures_[ Handle ] = pTexture;
		ShaderResourceViewSlot.Dirty_ = BcTrue; //ShaderResourceViewSlot.SRV_ != ShaderResourceView;
		ShaderResourceViewSlot.SRV_ = ShaderResourceView;
	}
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextD3D11::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	InputLayoutDirty_ |= Program_ != Program; // TODO: Only check vertex shader.
	ProgramDirty_ |= Program_ != Program;
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
	BcAssert( UniformBuffer != nullptr );

	// Bind for each shader based on specified handle.
	if( Handle != BcErrorCode )
	{
		if( UniformBuffers_[ Handle ] != UniformBuffer )
		{
			UniformBuffersDirty_ = BcTrue;
		}

		UniformBuffers_[ Handle ] = UniformBuffer;
		auto D3DConstantBuffer = getD3DBuffer( UniformBuffer->getHandle< BcU32 >() );
		auto& D3DConstantBufferSlot = D3DConstantBuffers_[ Handle ];
		D3DConstantBufferSlot.Dirty_ = BcTrue;//D3DConstantBuffer != D3DConstantBufferSlot.Buffer_;
		D3DConstantBufferSlot.Buffer_ = D3DConstantBuffer;
	}
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextD3D11::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	InputLayoutDirty_ |= VertexDeclaration_ != VertexDeclaration;
	VertexDeclaration_ = VertexDeclaration;
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextD3D11::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcU32 Width = Width_;
	BcU32 Height = Height_;

	if( FrameBuffer_ != FrameBuffer )
	{
		FrameBuffer_ = FrameBuffer;
		FrameBufferDirty_ = BcTrue;
	}

	if( FrameBuffer != nullptr )
	{
		const auto& Desc = FrameBuffer->getDesc();

		for( size_t Idx = 0; Idx < Desc.RenderTargets_.size(); ++Idx )
		{
			auto* Tex = Desc.RenderTargets_[ Idx ];
			ID3D11RenderTargetView* RTV = nullptr;
			if( Tex != nullptr )
			{
				RTV = getD3DRenderTargetView( Tex->getHandle< size_t >() );
			}

			if( RenderTargetViews_[ Idx ] != RTV )
			{
				RenderTargetViews_[ Idx ] = RTV;
			}
		}

		ID3D11DepthStencilView* DSV = nullptr;
		if( Desc.DepthStencilTarget_ != nullptr )
		{
			DSV = getD3DDepthStencilView( Desc.DepthStencilTarget_->getHandle< size_t >() );
		}

		if( DepthStencilView_ != DSV )
		{
			DepthStencilView_ = DSV;
		}

		Width = Desc.RenderTargets_[ 0 ]->getDesc().Width_;
		Height = Desc.RenderTargets_[ 0 ]->getDesc().Height_;
	}
	else
	{
		RenderTargetViews_.fill( nullptr );
		RenderTargetViews_[ 0 ] = getD3DRenderTargetView( BackBufferRTResourceIdx_ );
		DepthStencilView_ = getD3DDepthStencilView( BackBufferDSResourceIdx_ );
	}

	setViewport( RsViewport( 0, 0, Width, Height ) );
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextD3D11::clear( 
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	if( EnableClearColour )
	{
		for( auto RenderTargetView : RenderTargetViews_ )
		{
			if( RenderTargetView != nullptr )
			{
				FLOAT D3DColour[4] = { Colour.r(), Colour.g(), Colour.b(), Colour.a() };
				Context_->ClearRenderTargetView( RenderTargetView, D3DColour );
			}
		}
	}
	
	if( DepthStencilView_ != nullptr &&
		( EnableClearDepth || EnableClearStencil ) )
	{
		Context_->ClearDepthStencilView( 
			DepthStencilView_,
			( EnableClearDepth ? D3D11_CLEAR_DEPTH : 0 ) | 
			( EnableClearStencil ? D3D11_CLEAR_STENCIL : 0 ), 
			1.0f, 0 );
	}
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextD3D11::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	PSY_PROFILE_FUNCTION;
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
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	// Check topology type.
	TopologyType_ = PrimitiveType;

	flushState();
	Context_->DrawIndexed( NoofIndices, IndexOffset, VertexOffset );
}


//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextD3D11::setViewport( const class RsViewport& Viewport )
{
	PSY_PROFILE_FUNCTION;
	D3D11_VIEWPORT D3DViewport;
	D3DViewport.Width = (FLOAT)Viewport.width();
	D3DViewport.Height = (FLOAT)Viewport.height();
	D3DViewport.TopLeftX = (FLOAT)Viewport.x();
	D3DViewport.TopLeftY = (FLOAT)Viewport.y();
	D3DViewport.MinDepth = 0.0f;
	D3DViewport.MaxDepth = 1.0f;
	Context_->RSSetViewports( 1, &D3DViewport );
}


//////////////////////////////////////////////////////////////////////////
// setScissorRect
void RsContextD3D11::setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height )
{
	PSY_PROFILE_FUNCTION;
	D3D11_RECT D3DRect;
	D3DRect.left = X;
	D3DRect.top = Y;
	D3DRect.right = X + Width;
	D3DRect.bottom = Y + Height;
	Context_->RSSetScissorRects( 1, &D3DRect );
}

//////////////////////////////////////////////////////////////////////////
// dispatchCompute
void RsContextD3D11::dispatchCompute( class RsProgram* Program, RsDispatchBindings& Bindings, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups )
{
	// Bind compute shader.
	const auto& Shaders = Program->getShaders();
	for( auto* Shader : Shaders )
	{
		const auto& Desc = Shader->getDesc();
		switch( Desc.ShaderType_ )
		{
		case RsShaderType::COMPUTE:
			Context_->CSSetShader( Shader->getHandle< ID3D11ComputeShader* >(), nullptr, 0 );
			break;
		default:
			break;
		}
	}
	Context_->VSSetShader( nullptr, nullptr, 0 );
	Context_->HSSetShader( nullptr, nullptr, 0 );
	Context_->DSSetShader( nullptr, nullptr, 0 );
	Context_->GSSetShader( nullptr, nullptr, 0 );
	Context_->PSSetShader( nullptr, nullptr, 0 );
				
	// Bind stuff.
	for( BcU32 Idx = 0; Idx < Bindings.Buffers_.size(); ++Idx )
	{
		auto Buffer = Bindings.Buffers_[ Idx ];
		if( Buffer )
		{
			ID3D11UnorderedAccessView* UnorderedAccessView = 
				Buffer ? getD3DUnorderedAccessView( Buffer->getHandle< BcU32 >() ) : nullptr;
			Context_->CSSetUnorderedAccessViews( Idx, 1, &UnorderedAccessView, nullptr );
		}
	}

	// Dispatch.
	Context_->Dispatch( XGroups, YGroups, ZGroups );

	// Unbind stuff.
	// TODO: Optimise./ 
	for( BcU32 Idx = 0; Idx < Bindings.Buffers_.size(); ++Idx )
	{
		ID3D11UnorderedAccessView* UnorderedAccessView = nullptr;
		Context_->CSSetUnorderedAccessViews( Idx, 1, &UnorderedAccessView, nullptr );
	}

	// Program dirty, need to rebind it later.
	// Once stateless, won't be a problem.
	ProgramDirty_ = BcTrue;

	// Uniform buffers dirty, need to rebind later.
	UniformBuffersDirty_ = BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
bool RsContextD3D11::createRenderState(
	RsRenderState* RenderState )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& Desc = RenderState->getDesc();

	CD3D11_BLEND_DESC Blend( D3D11_DEFAULT );
	CD3D11_RASTERIZER_DESC Rasterizer( D3D11_DEFAULT );
	CD3D11_DEPTH_STENCIL_DESC DepthStencil( D3D11_DEFAULT );

	// Blend state.
	Blend.AlphaToCoverageEnable = FALSE;
	Blend.IndependentBlendEnable = TRUE;
	for( size_t Idx = 0; Idx < Desc.BlendState_.RenderTarget_.size(); ++Idx )
	{
		auto SrcBlendState = Desc.BlendState_.RenderTarget_[ Idx ];
		Blend.RenderTarget[ Idx ].BlendEnable = SrcBlendState.Enable_ ? TRUE : FALSE;
		Blend.RenderTarget[ Idx ].SrcBlend = gBlendType[ (size_t)SrcBlendState.SrcBlend_ ];
		Blend.RenderTarget[ Idx ].DestBlend = gBlendType[ (size_t)SrcBlendState.DestBlend_ ];
		Blend.RenderTarget[ Idx ].BlendOp = gBlendOp[ (size_t)SrcBlendState.BlendOp_ ];
		Blend.RenderTarget[ Idx ].SrcBlendAlpha = gBlendType[ (size_t)SrcBlendState.SrcBlendAlpha_ ];
		Blend.RenderTarget[ Idx ].DestBlendAlpha = gBlendType[ (size_t)SrcBlendState.DestBlendAlpha_ ];
		Blend.RenderTarget[ Idx ].BlendOpAlpha = gBlendOp[ (size_t)SrcBlendState.BlendOpAlpha_ ];
		Blend.RenderTarget[ Idx ].RenderTargetWriteMask = (UINT8)SrcBlendState.WriteMask_;
	}

	// Rasterizer state.
	auto SrcRasterizerState = Desc.RasteriserState_;
	Rasterizer.FillMode = gFillMode[ (size_t)SrcRasterizerState.FillMode_ ];
	Rasterizer.CullMode = gCullMode[ (size_t)SrcRasterizerState.CullMode_ ];
	Rasterizer.FrontCounterClockwise = FALSE;
	Rasterizer.DepthBias = (INT)SrcRasterizerState.DepthBias_;
	Rasterizer.SlopeScaledDepthBias = SrcRasterizerState.SlopeScaledDepthBias_;
	Rasterizer.DepthClipEnable = SrcRasterizerState.DepthClipEnable_ ? TRUE : FALSE;
	Rasterizer.ScissorEnable = SrcRasterizerState.ScissorEnable_ ? TRUE : FALSE;
	Rasterizer.AntialiasedLineEnable = SrcRasterizerState.AntialiasedLineEnable_ ? TRUE : FALSE;

	// Depth stencil state.
	auto SrcDepthStencilState = Desc.DepthStencilState_;
	DepthStencil.DepthEnable = SrcDepthStencilState.DepthTestEnable_ ? TRUE : FALSE;
	DepthStencil.DepthWriteMask = SrcDepthStencilState.DepthWriteEnable_ ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	DepthStencil.DepthFunc = gCompareFunc[ (size_t)SrcDepthStencilState.DepthFunc_ ];
	DepthStencil.StencilEnable = SrcDepthStencilState.StencilEnable_ ? TRUE : FALSE;
	DepthStencil.StencilReadMask = SrcDepthStencilState.StencilRead_;
	DepthStencil.StencilWriteMask = SrcDepthStencilState.StencilWrite_;
	DepthStencil.FrontFace.StencilFailOp = gStencilOp[ (size_t)SrcDepthStencilState.StencilFront_.Fail_ ];
	DepthStencil.FrontFace.StencilDepthFailOp = gStencilOp[ (size_t)SrcDepthStencilState.StencilFront_.DepthFail_ ];
	DepthStencil.FrontFace.StencilPassOp = gStencilOp[ (size_t)SrcDepthStencilState.StencilFront_.Pass_ ];
	DepthStencil.FrontFace.StencilFunc = gCompareFunc[ (size_t)SrcDepthStencilState.StencilFront_.Func_ ];
	DepthStencil.BackFace.StencilFailOp = gStencilOp[ (size_t)SrcDepthStencilState.StencilBack_.Fail_ ];
	DepthStencil.BackFace.StencilDepthFailOp = gStencilOp[ (size_t)SrcDepthStencilState.StencilBack_.DepthFail_ ];
	DepthStencil.BackFace.StencilPassOp = gStencilOp[ (size_t)SrcDepthStencilState.StencilBack_.Pass_ ];
	DepthStencil.BackFace.StencilFunc = gCompareFunc[ (size_t)SrcDepthStencilState.StencilBack_.Func_ ];
	
	// Create if not in caches.
	const BcU32 BlendStateHash = BcHash::GenerateCRC32( 0, &Blend, sizeof( Blend ) );
	const BcU32 RasterizerStateHash = BcHash::GenerateCRC32( 0, &Rasterizer, sizeof( Rasterizer ) );
	const BcU32 DepthStencilStateHash = BcHash::GenerateCRC32( 0, &DepthStencil, sizeof( DepthStencil ) );
	RenderStateInternal FinalRenderState;

	// Blend state.
	auto FoundBlendState = BlendStateCache_.find( BlendStateHash );
	if( FoundBlendState == BlendStateCache_.end() )
	{
		auto RetVal = Device_->CreateBlendState( &Blend, &FinalRenderState.Blend_ );
		if( FAILED( RetVal ) )
		{
			BcBreakpoint;
			return false;
		}
		BlendStateCache_.insert( std::make_pair( BlendStateHash, FinalRenderState.Blend_ ) );
	}
	else
	{
		FinalRenderState.Blend_ = FoundBlendState->second;
	}
	
	// Rasterizer.
	auto FoundRasterizerState = RasterizerStateCache_.find( RasterizerStateHash );
	if( FoundRasterizerState == RasterizerStateCache_.end() )
	{
		auto RetVal = Device_->CreateRasterizerState( &Rasterizer, &FinalRenderState.Rasterizer_ );
		if( FAILED( RetVal ) )
		{
			BcBreakpoint;
			return false;
		}
		RasterizerStateCache_.insert( std::make_pair( RasterizerStateHash, FinalRenderState.Rasterizer_ ) );
	}
	else
	{
		FinalRenderState.Rasterizer_ = FoundRasterizerState->second;
	}

	// Depth stencil.
	auto FoundDepthStencilState = DepthStencilStateCache_.find( DepthStencilStateHash );
	if( FoundDepthStencilState == DepthStencilStateCache_.end() )
	{
		auto RetVal = Device_->CreateDepthStencilState( &DepthStencil, &FinalRenderState.DepthStencil_ );
		if( FAILED( RetVal ) )
		{
			BcBreakpoint;
			return false;
		}
		DepthStencilStateCache_.insert( std::make_pair( DepthStencilStateHash, FinalRenderState.DepthStencil_ ) );
	}
	else
	{
		FinalRenderState.DepthStencil_ = FoundDepthStencilState->second;
	}

	// Look in cache.
	BcU32 RenderStateHash = BcHash::GenerateCRC32( 0, &FinalRenderState, sizeof( FinalRenderState ) );
	auto FoundRenderState = RenderStateCache_.find( RenderStateHash );
	if( FoundRenderState == RenderStateCache_.end() )
	{
		RenderStateCache_.insert( std::make_pair( RenderStateHash, FinalRenderState ) );
	}

	// Store in render state.
	RenderState->setHandle( RenderStateHash );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextD3D11::destroyRenderState(
	RsRenderState* RenderState )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextD3D11::createSamplerState(
	RsSamplerState* SamplerState )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	CD3D11_SAMPLER_DESC SamplerStateDesc( D3D11_DEFAULT );

	auto Desc = SamplerState->getDesc();

	// Err, maybe use the D3D11 stuff for simplicity.
	if( Desc.MinFilter_ == RsTextureFilteringMode::NEAREST &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		SamplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::NEAREST_MIPMAP_LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		SamplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		SamplerStateDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::NEAREST )
	{
		SamplerStateDesc.Filter = D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::LINEAR )
	{
		SamplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}
	else if( Desc.MinFilter_ == RsTextureFilteringMode::LINEAR_MIPMAP_LINEAR &&
		Desc.MagFilter_ == RsTextureFilteringMode::LINEAR )
	{
		SamplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	}
	else
	{
		// Fallback to something sensible.
		SamplerStateDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	}

	SamplerStateDesc.AddressU = gTextureAddressMode[ (BcU32)Desc.AddressU_ ];
	SamplerStateDesc.AddressV = gTextureAddressMode[ (BcU32)Desc.AddressV_ ];
	SamplerStateDesc.AddressW = gTextureAddressMode[ (BcU32)Desc.AddressW_ ];
	SamplerStateDesc.MipLODBias = 0.0f;
	SamplerStateDesc.MaxAnisotropy = 1;
	SamplerStateDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SamplerStateDesc.BorderColor[ 0 ] = 0.0f;
	SamplerStateDesc.BorderColor[ 1 ] = 0.0f;
	SamplerStateDesc.BorderColor[ 2 ] = 0.0f;
	SamplerStateDesc.BorderColor[ 3 ] = 0.0f;
	SamplerStateDesc.MinLOD = -FLT_MAX;
	SamplerStateDesc.MaxLOD = FLT_MAX;

	// Hash and look in map for already existing sampler state object.
	BcU32 SamplerStateHash = BcHash::GenerateCRC32( 0, &SamplerStateDesc, sizeof( SamplerStateDesc ) );
	auto FoundSamplerState = SamplerStateCache_.find( SamplerStateHash );
	if( FoundSamplerState == SamplerStateCache_.end() )
	{
		SamplerStateInternal State;
		State.LastFrameUsed_ = 0;
		Device_->CreateSamplerState( &SamplerStateDesc, &State.Sampler_ );
		SamplerStateCache_[ SamplerStateHash ] = State;
	}

	SamplerState->setHandle( SamplerStateHash );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextD3D11::destroySamplerState(
	RsSamplerState* SamplerState )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextD3D11::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextD3D11::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextD3D11::createBuffer( 
	class RsBuffer* Buffer )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	//
	const auto& BufferDesc = Buffer->getDesc();

	// Buffer desc.
	D3D11_BUFFER_DESC Desc;
	Desc.Usage = D3D11_USAGE_DEFAULT;			// TODO.
	Desc.ByteWidth = static_cast< UINT >( BcPotRoundUp( BufferDesc.SizeBytes_, 16 ) );
	Desc.BindFlags = GetBindFlags( BufferDesc.BindFlags_ );
	Desc.CPUAccessFlags = 0;
	Desc.MiscFlags = 0;
	Desc.StructureByteStride = BufferDesc.StructureStride_;
	
	// Dynamic? Setup CPU_WRITE + DYNAMIC.
	if( ( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
	{
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.Usage = D3D11_USAGE_DYNAMIC;
	}
	// Stream? Setup CPU_WRITE + DYNAMIC.
	else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
	{
		Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Desc.Usage = D3D11_USAGE_DYNAMIC;
	}

	// If unordered access, allow raw views, and setup if a structured buffer or not.
	if( ( BufferDesc.BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE )
	{
		if( BufferDesc.StructureStride_ > 0 )
		{
			Desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		}
		else
		{
			Desc.MiscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
		}
	}

	// Generate buffers.
	ID3D11Buffer* D3DBuffer = nullptr; 

	HRESULT Result = Device_->CreateBuffer( &Desc, nullptr, &D3DBuffer );
	if( Result == S_OK )
	{
		Buffer->setHandle( addD3DResource( 
			D3DBuffer,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN ) );
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
	PSY_PROFILE_FUNCTION;
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
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& BufferDesc = Buffer->getDesc();
	ID3D11Buffer* D3DBuffer = getD3DBuffer( Buffer->getHandle< BcU32 >() ); 
	ComPtr<ID3D11Buffer> StagingD3DBuffer( D3DBuffer );
	if( D3DBuffer != nullptr )
	{
		bool RequireStaging = ( BufferDesc.Flags_ & RsResourceCreationFlags::STATIC ) != RsResourceCreationFlags::NONE;
		if( RequireStaging )
		{
			D3D11_BUFFER_DESC StagingDesc;
			StagingDesc.Usage = D3D11_USAGE_STAGING;
			StagingDesc.ByteWidth = static_cast< UINT >( BcPotRoundUp( BufferDesc.SizeBytes_, 16 ) );
			StagingDesc.BindFlags = 0;
			StagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			StagingDesc.MiscFlags = 0;
			StagingDesc.StructureByteStride = 0;
			HRESULT Result = Device_->CreateBuffer( &StagingDesc, nullptr, StagingD3DBuffer.ReleaseAndGetAddressOf() );
			if( FAILED( Result ) )
			{
				BcBreakpoint;
				return false;
			}
		}

		D3D11_MAP MapType = RequireStaging ? D3D11_MAP_WRITE : D3D11_MAP_WRITE_DISCARD;
		BcAssert( Offset == 0 );
		D3D11_MAPPED_SUBRESOURCE SubRes = { nullptr, 0, 0 };
		HRESULT RetVal = Context_->Map( StagingD3DBuffer.Get(), 0, MapType, 0, &SubRes );
		if( SUCCEEDED( RetVal ) )
		{
			RsBufferLock Lock = 
			{
				(BcU8*)SubRes.pData + Offset
			};
			UpdateFunc( Buffer, Lock );
			Context_->Unmap( StagingD3DBuffer.Get(), 0 );
		}
		else
		{
			BcBreakpoint;
		}

		if( RequireStaging )
		{
			Context_->CopyResource( D3DBuffer, StagingD3DBuffer.Get() );
		}

		return true;
	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextD3D11::createTexture( 
	class RsTexture* Texture )
{
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	HRESULT Result = 0;
	//
	const auto& TextureDesc = Texture->getDesc();

	BcAssert( (UINT)RsResourceBindFlags::VERTEX_BUFFER == D3D11_BIND_VERTEX_BUFFER );
	BcAssert( (UINT)RsResourceBindFlags::INDEX_BUFFER == D3D11_BIND_INDEX_BUFFER );
	BcAssert( (UINT)RsResourceBindFlags::UNIFORM_BUFFER == D3D11_BIND_CONSTANT_BUFFER );
	BcAssert( (UINT)RsResourceBindFlags::SHADER_RESOURCE == D3D11_BIND_SHADER_RESOURCE );
	BcAssert( (UINT)RsResourceBindFlags::STREAM_OUTPUT == D3D11_BIND_STREAM_OUTPUT );
	BcAssert( (UINT)RsResourceBindFlags::RENDER_TARGET == D3D11_BIND_RENDER_TARGET );
	BcAssert( (UINT)RsResourceBindFlags::DEPTH_STENCIL == D3D11_BIND_DEPTH_STENCIL );
	BcAssert( (UINT)RsResourceBindFlags::UNORDERED_ACCESS == D3D11_BIND_UNORDERED_ACCESS );
	UINT BindFlagsD3D = (UINT)TextureDesc.BindFlags_;

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
			Desc.BindFlags = BindFlagsD3D;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture1D* D3DTexture = nullptr;
			Result = Device_->CreateTexture1D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );
			
			Texture->setHandle( addD3DResource( 
				D3DTexture,
				gTextureFormats[ (BcU32)TextureDesc.Format_ ],
				gDSVFormats[ (BcU32)TextureDesc.Format_ ],
				gSRVFormats[ (BcU32)TextureDesc.Format_ ] ) );
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
			Desc.BindFlags = BindFlagsD3D;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture2D* D3DTexture = nullptr;
 			Result = Device_->CreateTexture2D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );

			Texture->setHandle( addD3DResource( 
				D3DTexture,
				gTextureFormats[ (BcU32)TextureDesc.Format_ ],
				gDSVFormats[ (BcU32)TextureDesc.Format_ ],
				gSRVFormats[ (BcU32)TextureDesc.Format_ ] ) );
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
			Desc.BindFlags = BindFlagsD3D;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture3D* D3DTexture = nullptr;
			Result = Device_->CreateTexture3D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );

			Texture->setHandle( addD3DResource( 
				D3DTexture,
				gTextureFormats[ (BcU32)TextureDesc.Format_ ],
				gDSVFormats[ (BcU32)TextureDesc.Format_ ],
				gSRVFormats[ (BcU32)TextureDesc.Format_ ] ) );
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
			Desc.BindFlags = BindFlagsD3D;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			ID3D11Texture2D* D3DTexture = nullptr;
			Result = Device_->CreateTexture2D( &Desc, nullptr, &D3DTexture );
			BcAssert( SUCCEEDED( Result ) );

			Texture->setHandle( addD3DResource( 
				D3DTexture,
				gTextureFormats[ (BcU32)TextureDesc.Format_ ],
				gDSVFormats[ (BcU32)TextureDesc.Format_ ],
				gSRVFormats[ (BcU32)TextureDesc.Format_ ] ) );
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
	PSY_PROFILE_FUNCTION;
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
	PSY_PROFILE_FUNCTION;
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	const auto& Desc = Texture->getDesc();
	BcU32 Width = BcMax( 1, Desc.Width_ >> Slice.Level_ );
	BcU32 Height = BcMax( 1, Desc.Height_ >> Slice.Level_ );
	BcU32 Depth = BcMax( 1, Desc.Depth_ >> Slice.Level_ );
	BcU32 TextureDataSize = RsTextureFormatSize( Desc.Format_, Width, Height, Depth, 1 );
	std::vector< BcU8 > TextureData( TextureDataSize );
	ID3D11Resource* D3DResource = getD3DResource( Texture->getHandle< BcU32 >() );

	// Update texture.
	const auto BlockInfo = RsTextureBlockInfo( Desc.Format_ );
	RsTextureLock Lock;
	Lock.Buffer_ = &TextureData[ 0 ];
	Lock.Pitch_ = ( ( Width / BlockInfo.Width_ ) * BlockInfo.Bits_ ) / 8;
	Lock.SlicePitch_ = ( ( Width / BlockInfo.Width_ ) * ( Height / BlockInfo.Height_ ) * BlockInfo.Bits_ ) / 8;

	// Update.
	UpdateFunc( Texture, Lock );

	BcU32 SubResource = Slice.Level_;
	
	// Calculate cubemap subresource.
	if( Desc.Type_ == RsTextureType::TEXCUBE )
	{
		BcAssert( Slice.Face_ != RsTextureFace::NONE );
		SubResource = Slice.Level_ * 6 + ( (BcU32)Slice.Face_ - 1 );
	}

	// Buffer sub resource.
	switch( Desc.Type_ )
	{
	case RsTextureType::TEX1D:
		{
			Context_->UpdateSubresource(
				D3DResource,
				SubResource,
				nullptr,
				&TextureData[ 0 ],
				Lock.Pitch_,
				Lock.SlicePitch_ );
		}
		break;

	case RsTextureType::TEX2D:
		{
			Context_->UpdateSubresource(
				D3DResource,
				SubResource,
				nullptr,
				&TextureData[ 0 ],
				Lock.Pitch_,
				Lock.SlicePitch_ );
		}
		break;

	case RsTextureType::TEX3D:
		{
			Context_->UpdateSubresource(
				D3DResource,
				Slice.Level_,
				nullptr,
				&TextureData[ 0 ],
				Lock.Pitch_,
				Lock.SlicePitch_ );
		}
		break;

	case RsTextureType::TEXCUBE:
		{
			Context_->UpdateSubresource(
				D3DResource,
				SubResource,
				nullptr,
				&TextureData[ 0 ],
				Lock.Pitch_,
				Lock.SlicePitch_ );
		}
		break;
	};

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextD3D11::createShader(
	class RsShader* Shader )
{
	PSY_PROFILE_FUNCTION;
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

	case RsShaderType::PIXEL:
		{
			Result = Device_->CreatePixelShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11PixelShader** >( &D3DShader ) );
		}
		break;

	case RsShaderType::HULL:
		{
			Result = Device_->CreateHullShader(
				Shader->getData(),
				Shader->getDataSize(),
				nullptr,
				reinterpret_cast< ID3D11HullShader** >( &D3DShader ) );
		}
		break;

	case RsShaderType::DOMAIN:
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
	PSY_PROFILE_FUNCTION;
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
	PSY_PROFILE_FUNCTION;
	Program->setHandle( new RsProgramD3D11( Program, Device_.Get() ) );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextD3D11::destroyProgram(
	class RsProgram* Program )
{
	PSY_PROFILE_FUNCTION;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
bool RsContextD3D11::createVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration )
{
	PSY_PROFILE_FUNCTION;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyVertexDeclaration
bool RsContextD3D11::destroyVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration  )
{
	PSY_PROFILE_FUNCTION;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D11::flushState()
{
	PSY_PROFILE_FUNCTION;

	// Cache stage resource setters in arrays for quick look up.
	static decltype( &ID3D11DeviceContext::VSSetShaderResources ) SRVSetters[] = {
		&ID3D11DeviceContext::VSSetShaderResources,
		&ID3D11DeviceContext::PSSetShaderResources,
		&ID3D11DeviceContext::HSSetShaderResources,
		&ID3D11DeviceContext::DSSetShaderResources,
		&ID3D11DeviceContext::GSSetShaderResources,
		&ID3D11DeviceContext::CSSetShaderResources,
	};

	static decltype( &ID3D11DeviceContext::VSSetSamplers ) SamplerSetters[] = {
		&ID3D11DeviceContext::VSSetSamplers,
		&ID3D11DeviceContext::PSSetSamplers,
		&ID3D11DeviceContext::HSSetSamplers,
		&ID3D11DeviceContext::DSSetSamplers,
		&ID3D11DeviceContext::GSSetSamplers,
		&ID3D11DeviceContext::CSSetSamplers,
	};
	
	static decltype( &ID3D11DeviceContext::VSSetConstantBuffers ) ConstSetters[] = {
		&ID3D11DeviceContext::VSSetConstantBuffers,
		&ID3D11DeviceContext::PSSetConstantBuffers,
		&ID3D11DeviceContext::HSSetConstantBuffers,
		&ID3D11DeviceContext::DSSetConstantBuffers,
		&ID3D11DeviceContext::GSSetConstantBuffers,
		&ID3D11DeviceContext::CSSetConstantBuffers,
	};

	HRESULT Result = 0;

	// Set render targets.
	if( FrameBufferDirty_ )
	{
		PSY_PROFILER_SECTION( FBRoot, "FB" );
		Context_->OMSetRenderTargets( static_cast< UINT >( RenderTargetViews_.size() ), &RenderTargetViews_[ 0 ], DepthStencilView_ );
		FrameBufferDirty_ = BcFalse;
	}

	// Bind shaders.
	std::array< bool, (BcU32)RsShaderType::MAX > HasShaders = { false };
	if( Program_ != nullptr && ProgramDirty_ )
	{
		PSY_PROFILER_SECTION( ShadersRoot, "Shaders" );
		const auto& Shaders = Program_->getShaders();
		for( auto* Shader : Shaders )
		{
			const auto& Desc = Shader->getDesc();
			BcU32 ShaderTypeIdx = (BcU32)Desc.ShaderType_;
			HasShaders[ ShaderTypeIdx ] = true;
			switch( Desc.ShaderType_ )
			{
			case RsShaderType::VERTEX:
				Context_->VSSetShader( Shader->getHandle< ID3D11VertexShader* >(), nullptr, 0 );
				break;

			case RsShaderType::HULL:
				Context_->HSSetShader( Shader->getHandle< ID3D11HullShader* >(), nullptr, 0 );
				break;

			case RsShaderType::DOMAIN:
				Context_->DSSetShader( Shader->getHandle< ID3D11DomainShader* >(), nullptr, 0 );
				break;

			case RsShaderType::GEOMETRY:
				Context_->GSSetShader( Shader->getHandle< ID3D11GeometryShader* >(), nullptr, 0 );
				break;

			case RsShaderType::PIXEL:
				Context_->PSSetShader( Shader->getHandle< ID3D11PixelShader* >(), nullptr, 0 );
				break;

			case RsShaderType::COMPUTE:
				Context_->CSSetShader( Shader->getHandle< ID3D11ComputeShader* >(), nullptr, 0 );
				break;
			}
		}

		// Disable unused pipeline stages.
		for( BcU32 Idx = 0; Idx < (BcU32)RsShaderType::MAX; ++Idx )
		{
			if( HasShaders[ Idx ] == false )
			{
				switch( (RsShaderType)Idx )
				{
				case RsShaderType::VERTEX:
					Context_->VSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::HULL:
					Context_->HSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::DOMAIN:
					Context_->DSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::GEOMETRY:
					Context_->GSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::PIXEL:
					Context_->PSSetShader( nullptr, nullptr, 0 );
					break;
				case RsShaderType::COMPUTE:
					Context_->CSSetShader( nullptr, nullptr, 0 );
					break;
				}
			}
		}

		ProgramDirty_ = BcFalse;
	}

	// Bind up uniform buffers
	if( UniformBuffersDirty_ )
	{
		PSY_PROFILER_SECTION( ShaderRoot, "Uniforms" );
		RsProgramD3D11* ProgramD3D11 = Program_->getHandle< RsProgramD3D11* >();
		const auto& Shaders = Program_->getShaders();
		for( auto* Shader : Shaders )
		{
			const auto& Desc = Shader->getDesc();
			BcU32 ShaderTypeIdx = (BcU32)Desc.ShaderType_;
			for( BcU32 Idx = 0; Idx < D3DConstantBuffers_.size(); ++Idx )
			{
				BcU32 Slot = ProgramD3D11->getCBSlot( Desc.ShaderType_, Idx );
				if( Slot != BcErrorCode )
				{
					auto& D3DConstantBufferSlot = D3DConstantBuffers_[ Idx ];
					if( D3DConstantBufferSlot.Dirty_ )
					{
						((Context_.Get())->*ConstSetters[ ShaderTypeIdx ])( 
							Slot,
							1,							
							&D3DConstantBufferSlot.Buffer_ );
						D3DConstantBufferSlot.Dirty_ = BcFalse;
					}
				}
			}
		}

		UniformBuffersDirty_ = BcFalse;
	}

	if( TexturesDirty_ )
	{
		PSY_PROFILER_SECTION( ShaderRoot, "Shader resources + samplers" );
		RsProgramD3D11* ProgramD3D11 = Program_->getHandle< RsProgramD3D11* >();
		const auto& Shaders = Program_->getShaders();
		for( auto* Shader : Shaders )
		{
			const auto& Desc = Shader->getDesc();
			BcU32 ShaderTypeIdx = (BcU32)Desc.ShaderType_;
			for( BcU32 Idx = 0; Idx < D3DShaderResourceViews_.size(); ++Idx )
			{
				BcU32 SRVSlot = ProgramD3D11->getSRVSlot( Desc.ShaderType_, Idx );
				if( SRVSlot != BcErrorCode )
				{
					auto& D3DShaderResourceViewSlot = D3DShaderResourceViews_[ Idx ];
					if( D3DShaderResourceViewSlot.Dirty_ )
					{
						((Context_.Get())->*SRVSetters[ ShaderTypeIdx ])( 
							SRVSlot,
							1,							
							&D3DShaderResourceViewSlot.SRV_ );
						D3DShaderResourceViewSlot.Dirty_ = BcFalse;
					}
				}

				BcU32 SamplerSlot = SRVSlot; // TODO: ProgramD3D11->getSamplerSlot( Desc.ShaderType_, Idx );
				if( SamplerSlot != BcErrorCode )
				{
					auto& D3DSamplerStateSlot = D3DSamplerStates_[ Idx ];
					if( D3DSamplerStateSlot.Dirty_ )
					{
						((Context_.Get())->*SamplerSetters[ ShaderTypeIdx ])( 
							SRVSlot,
							1,
							&D3DSamplerStateSlot.Sampler_ );
						D3DSamplerStateSlot.Dirty_ = BcFalse;
					}
				}
			}
		}

		TexturesDirty_ = BcFalse;
	}

	// Input layout.
	if( InputLayoutDirty_ )
	{
		PSY_PROFILER_SECTION( ILRoot, "Input layout" );

		// Grab current input layout.
		BcU32 InputLayoutHash = generateInputLayoutHash();

		auto FoundInputLayout = InputLayoutMap_.find( InputLayoutHash );
		if( FoundInputLayout == InputLayoutMap_.end() )
		{
			const auto& Desc = VertexDeclaration_->getDesc();
			const auto& VertexAttributes = Program_->getVertexAttributeList();

			// Find vertex shader.
			RsShader* VertexShader = nullptr;
			const auto& Shaders = Program_->getShaders();
			for( auto* Shader : Shaders )
			{
				if( Shader->getDesc().ShaderType_ == RsShaderType::VERTEX )
				{
					VertexShader = Shader;
					break;
				}
			}

			// Create input layout for current setup.
			// For missing vertex attributes, we just fudge them to be zero offset.
			std::vector< D3D11_INPUT_ELEMENT_DESC > ElementDescs;
			ElementDescs.reserve( Desc.Elements_.size() );

			for( const auto& VertexAttribute : VertexAttributes )
			{
				auto FoundElement = std::find_if( Desc.Elements_.begin(), Desc.Elements_.end(),
					[ &VertexAttribute ]( const RsVertexElement& Element )
					{
						return ( Element.Usage_ == VertexAttribute.Usage_ &&
							Element.UsageIdx_ == VertexAttribute.UsageIdx_ );
					} );

				// Force to an element with zero offset if we can't find a valid one.
				// TODO: Find a better approach.
				if( FoundElement == Desc.Elements_.end() )
				{
					FoundElement = std::find_if( Desc.Elements_.begin(), Desc.Elements_.end(),
					[]( const RsVertexElement& Element )
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
			
			Result = Device_->CreateInputLayout( 
				&ElementDescs[ 0 ], 
				static_cast< UINT >( ElementDescs.size() ), 
				VertexShader->getData(), 
				VertexShader->getDataSize(), 
				&InputLayoutEntry.InputLayout_ );
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

		InputLayoutDirty_ = BcFalse;
	}

	// Set buffers.
	// TODO: Redundancy.
	Context_->IASetIndexBuffer( 
		D3DIndexBuffer_,
		DXGI_FORMAT_R16_UINT, // TODO: Investigate correct formats.
		0 );

	Context_->IASetVertexBuffers( 
		0, 
		static_cast< UINT >( D3DVertexBuffers_.size() ),
		&D3DVertexBuffers_[ 0 ],
		&D3DVertexBufferStrides_[ 0 ],
		&D3DVertexBufferOffsets_[ 0 ] );

	if( BoundRenderState_ != nullptr )
	{
		const auto RenderStateHash = BoundRenderState_->getHandle< BcU32 >();
		auto& RenderStateInternal = RenderStateCache_[ RenderStateHash ];

		FLOAT Factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		Context_->OMSetBlendState( RenderStateInternal.Blend_, Factor, 0xffffffff );
		Context_->RSSetState( RenderStateInternal.Rasterizer_ );
		Context_->OMSetDepthStencilState( RenderStateInternal.DepthStencil_, BoundRenderState_->getDesc().DepthStencilState_.StencilRef_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// addD3DResource
size_t RsContextD3D11::addD3DResource( 
		ID3D11Resource* D3DResource,
		DXGI_FORMAT ResourceFormat,
		DXGI_FORMAT DSVFormat,
		DXGI_FORMAT SRVFormat )
{
	ResourceViewCacheEntry Entry = {};

	// If the cache is empty, push another entry into it.
	if( ResourceViewCacheFreeIdx_.size() == 0 )
	{
		ResourceViewCacheFreeIdx_.push_back( ResourceViewCache_.size() );
		ResourceViewCache_.push_back( Entry );
	}

	// Get entry from back of the cache, and pop it off.
	size_t EntryIdx = ResourceViewCacheFreeIdx_.back();
	ResourceViewCacheFreeIdx_.pop_back();

	// Setup entry.
	Entry.Resource_ = D3DResource;
	Entry.ResourceFormat_ = ResourceFormat;
	Entry.DSVFormat_ = DSVFormat;
	Entry.SRVFormat_ = SRVFormat;
	
	// Store in cache.
	ResourceViewCache_[ EntryIdx ] = Entry;

	return EntryIdx;
}

//////////////////////////////////////////////////////////////////////////
// delD3DResource
void RsContextD3D11::delD3DResource( size_t ResourceIdx )
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
ID3D11Resource* RsContextD3D11::getD3DResource( size_t ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	return Entry.Resource_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DBuffer
ID3D11Buffer* RsContextD3D11::getD3DBuffer( size_t ResourceIdx )
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
ID3D11Texture1D* RsContextD3D11::getD3DTexture1D( size_t ResourceIdx )
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
ID3D11Texture2D* RsContextD3D11::getD3DTexture2D( size_t ResourceIdx )
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
ID3D11Texture3D* RsContextD3D11::getD3DTexture3D( size_t ResourceIdx )
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
ID3D11ShaderResourceView* RsContextD3D11::getD3DShaderResourceView( size_t ResourceIdx )
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
				Desc.Format = Entry.SRVFormat_;
				Desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE1D;
				Desc.Texture1D.MipLevels = TexDesc.MipLevels;
				Desc.Texture1D.MostDetailedMip = 0;
			}
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				D3D11_TEXTURE2D_DESC TexDesc;
				Entry.Texture2DResource_->GetDesc( &TexDesc );
				Desc.Format = Entry.SRVFormat_;
				if( ( TexDesc.MiscFlags & D3D11_RESOURCE_MISC_TEXTURECUBE ) == 0 )
				{
					Desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
					Desc.Texture2D.MipLevels = TexDesc.MipLevels;
					Desc.Texture2D.MostDetailedMip = 0;
				}
				else
				{
					Desc.ViewDimension = D3D_SRV_DIMENSION_TEXTURECUBE;
					Desc.TextureCube.MipLevels = TexDesc.MipLevels;
					Desc.TextureCube.MostDetailedMip = 0;
				}
			}
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			{
				D3D11_TEXTURE3D_DESC TexDesc;
				Entry.Texture3DResource_->GetDesc( &TexDesc );
				Desc.Format = Entry.SRVFormat_;
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
		BcUnusedVar( Result );
	}

	return Entry.ShaderResourceView_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DUnorderedAccessView
ID3D11UnorderedAccessView* RsContextD3D11::getD3DUnorderedAccessView( size_t ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	if( Entry.UnorderedAccessView_ == nullptr )
	{
		//
		D3D11_RESOURCE_DIMENSION ResDim;
		Entry.Resource_->GetType( &ResDim );
		
		// Create unordered access view.
		D3D11_UNORDERED_ACCESS_VIEW_DESC Desc;

		switch( ResDim )
		{
		case D3D11_RESOURCE_DIMENSION_BUFFER:
			{
				D3D11_BUFFER_DESC BufferDesc;
				Entry.BufferResource_->GetDesc( &BufferDesc );
				Desc.Format = 
					( BufferDesc.StructureByteStride == 0 ? 
						DXGI_FORMAT_R32_TYPELESS : 
						DXGI_FORMAT_UNKNOWN );
				Desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				Desc.Buffer.FirstElement = 0;
				Desc.Buffer.NumElements = BufferDesc.ByteWidth / 
					( BufferDesc.StructureByteStride == 0 ? 
						4 : 
						BufferDesc.StructureByteStride );
				Desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
			}
			break;

		default:
			BcBreakpoint;
			return nullptr;
			break;
		}

		// Create unordered access view.
		HRESULT Result = Device_->CreateUnorderedAccessView( 
			Entry.Resource_,
			&Desc,
			&Entry.UnorderedAccessView_ );
 		BcAssert( SUCCEEDED( Result ) );
		BcUnusedVar( Result );
	}

	return Entry.UnorderedAccessView_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DRenderTargetView
ID3D11RenderTargetView* RsContextD3D11::getD3DRenderTargetView( size_t ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	if( Entry.RenderTargetView_ == nullptr )
	{
		// Check resource dimensions.
		D3D11_RESOURCE_DIMENSION ResDim;
		Entry.Resource_->GetType( &ResDim );

		// Create render target view.
		D3D11_RENDER_TARGET_VIEW_DESC Desc;
		BcMemZero( &Desc, sizeof( Desc ) );

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
		BcUnusedVar( Result );
	}

	return Entry.RenderTargetView_;
}

//////////////////////////////////////////////////////////////////////////
// getD3DDepthStencilView
ID3D11DepthStencilView* RsContextD3D11::getD3DDepthStencilView( size_t ResourceIdx )
{
	auto& Entry = ResourceViewCache_[ ResourceIdx ];
	if( Entry.DepthStencilView_ == nullptr )
	{
		// Check resource dimensions.
		D3D11_RESOURCE_DIMENSION ResDim;
		Entry.Resource_->GetType( &ResDim );

		// Create depth stencil view.
		D3D11_DEPTH_STENCIL_VIEW_DESC Desc;
		BcMemZero( &Desc, sizeof( Desc ) );

		switch( ResDim )
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			{
				D3D11_TEXTURE2D_DESC TexDesc;
				Entry.Texture2DResource_->GetDesc( &TexDesc );
				Desc.Format = Entry.DSVFormat_;
				Desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
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
		BcUnusedVar( Result );
	}

	return Entry.DepthStencilView_;
}

//////////////////////////////////////////////////////////////////////////
// generateInputLayoutHash
//virtual
BcU32 RsContextD3D11::generateInputLayoutHash() const
{
	return Program_->getInputLayoutHash() ^ VertexDeclaration_->getInputLayoutHash();
}