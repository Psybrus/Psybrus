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
#include "System/Renderer/RsVertexDeclaration.h"

#include "System/Os/OsClient.h"
#include "System/Os/OsClientWindows.h"

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// Direct3D 11 library.
#pragma comment (lib, "d3d11.lib")

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
	BcBreakpoint;
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
	return false;
}

//////////////////////////////////////////////////////////////////////////
// maxShaderCodeType
//virtual
RsShaderCodeType RsContextD3D11::maxShaderCodeType( RsShaderCodeType CodeType ) const
{
	// TODO: Use feature level.
	return RsShaderCodeType::D3D11_5_1;
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
//virtual
void RsContextD3D11::takeScreenshot()
{

}

//////////////////////////////////////////////////////////////////////////
// setViewport
//virtual
void RsContextD3D11::setViewport( class RsViewport& Viewport )
{

}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void RsContextD3D11::create()
{
	OsClientWindows* pClient = dynamic_cast< OsClientWindows* >( pClient_ );
	BcAssertMsg( pClient != nullptr, "Windows client is not being used!" );

	// Setup swap chain desc.
	BcMemZero( &SwapChainDesc_, sizeof( SwapChainDesc_ ) );
    SwapChainDesc_.BufferCount = 1;
    SwapChainDesc_.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    SwapChainDesc_.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc_.OutputWindow = pClient->getHWND();
    SwapChainDesc_.Windowed = TRUE;
	SwapChainDesc_.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Create device and swap chain.
	D3D11CreateDeviceAndSwapChain( Adapter_,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_SINGLETHREADED,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&SwapChainDesc_,
		&SwapChain_,
		&Device_,
		&FeatureLevel_,
		&Context_ );

	// Get back buffer from swap chain.
	SwapChain_->GetBuffer( 0, __uuidof(ID3D11Texture2D), (void**)&BackBuffer_ );
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsContextD3D11::update()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
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

	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextD3D11::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextD3D11::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextD3D11::setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextD3D11::getRenderState( RsRenderStateType State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcBreakpoint;

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

	// TODO.
	BcBreakpoint;
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
	BcU32 SlotIdx, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	UniformBuffers_[ SlotIdx ] = UniformBuffer;
	D3DConstantBuffers_[ SlotIdx ] = getD3DBuffer( UniformBuffer->getHandle< BcU32 >() );
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
	BcBreakpoint;
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
	Desc.ByteWidth =BufferDesc.SizeBytes_;
	Desc.BindFlags = gBufferType[ (BcU32)BufferDesc.Type_ ];
	Desc.CPUAccessFlags = 0;
	Desc.MiscFlags = 0;
	
	if( ( BufferDesc.Flags_ & RsResourceCreationFlags::DYNAMIC ) != RsResourceCreationFlags::NONE )
	{
		Desc.Usage = D3D11_USAGE_DYNAMIC;
	}
	else if( ( BufferDesc.Flags_ & RsResourceCreationFlags::STREAM ) != RsResourceCreationFlags::NONE )
	{
		Desc.Usage = D3D11_USAGE_DYNAMIC;
	}

	// Generate buffers.
	ID3D11Buffer* D3DBuffer = nullptr; 

	HRESULT Result = Device_->CreateBuffer( &Desc, nullptr, &D3DBuffer );
	if( Result == S_OK )
	{
		Buffer->setHandle( addD3DResource( D3DBuffer ) );
		return true;
	}

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
			D3D11_MAP_WRITE;		// TODO: Select more optimal map type.
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
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

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
			Device_->CreateTexture1D( &Desc, nullptr, &D3DTexture );

			Texture->setHandle( addD3DResource( D3DTexture ) );
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
			Desc.SampleDesc.Count = 0;
			Desc.SampleDesc.Quality = 0;
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture2D* D3DTexture = nullptr;
			Device_->CreateTexture2D( &Desc, nullptr, &D3DTexture );

			Texture->setHandle( addD3DResource( D3DTexture ) );
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
			Device_->CreateTexture3D( &Desc, nullptr, &D3DTexture );

			Texture->setHandle( addD3DResource( D3DTexture ) );
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
			Desc.SampleDesc.Count = 0;
			Desc.SampleDesc.Quality = 0;
			Desc.Usage = D3D11_USAGE_DEFAULT;
			Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;

			ID3D11Texture2D* D3DTexture = nullptr;
			Device_->CreateTexture2D( &Desc, nullptr, &D3DTexture );

			Texture->setHandle( addD3DResource( D3DTexture ) );
		}
		break;
	}
	
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

	BcBreakpoint;
	return false;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D11::flushState()
{
	if( InputLayoutChanged_ )
	{
		// Grab current input layout.
		BcU32 InputLayoutHash = generateInputLayoutHash();

		auto FoundInputLayout = InputLayoutMap_.find( InputLayoutHash );
		if( FoundInputLayout == InputLayoutMap_.end() )
		{
			// Create input layout for current setup.
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

	// Bind all constant buffers to all stages.
	// TODO: Hook them up seperately as required.
	Context_->VSSetConstantBuffers( 
		0,
		D3DConstantBuffers_.size(),
		&D3DConstantBuffers_[ 0 ] );
	Context_->DSSetConstantBuffers( 
		0,
		D3DConstantBuffers_.size(),
		&D3DConstantBuffers_[ 0 ] );
	Context_->HSSetConstantBuffers( 
		0,
		D3DConstantBuffers_.size(),
		&D3DConstantBuffers_[ 0 ] );
	Context_->GSSetConstantBuffers( 
		0,
		D3DConstantBuffers_.size(),
		&D3DConstantBuffers_[ 0 ] );
	Context_->PSSetConstantBuffers( 
		0,
		D3DConstantBuffers_.size(),
		&D3DConstantBuffers_[ 0 ] );

	// Bind all shader resources.
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