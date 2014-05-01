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

#include "System/Os/OsClient.h"
#include "System/Os/OsClientWindows.h"

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// Direct3D 11 library.
#pragma comment (lib, "d3d11.lib")

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
// swapBuffers
void RsContextD3D11::swapBuffers()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	SwapChain_->Present( 0, 0 );
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextD3D11::takeScreenshot()
{
	ScreenshotRequested_ = BcTrue;
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
	SwapChain_->Release();
	Device_->Release();
	Context_->Release();
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
void RsContextD3D11::setRenderState( eRsRenderState State, BcS32 Value, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// getRenderState
BcS32 RsContextD3D11::getRenderState( eRsRenderState State ) const
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcBreakpoint;

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// setTextureState
void RsContextD3D11::setTextureState( BcU32 Sampler, RsTexture* pTexture, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextD3D11::flushState()
{
	BcBreakpoint;
}

