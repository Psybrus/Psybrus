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

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextD3D11::RsContextD3D11( OsClient* pClient, RsContextD3D11* pParent ):
	RsContext( pParent ),
	pParent_( pParent ),
	pClient_( pClient ),
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

	BcBreakpoint;
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
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void RsContextD3D11::update()
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void RsContextD3D11::destroy()
{
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

