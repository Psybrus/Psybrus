/**************************************************************************
*
* File:		RsContextD3D11.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXTD3D11_H__
#define __RSCONTEXTD3D11_H__

#include "System/Renderer/RsContext.h"
#include "System/Renderer/D3D11/RsD3D11.h"

#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////////
// RsContextD3D11
class RsContextD3D11:
	public RsContext
{
public:
	RsContextD3D11( OsClient* pClient, RsContextD3D11* pParent );
	virtual ~RsContextD3D11();
	
	virtual BcU32						getWidth() const;
	virtual BcU32						getHeight() const;

	void								swapBuffers();
	void								takeScreenshot();

	void								setDefaultState();
	void								invalidateRenderState();
	void								invalidateTextureState();
	void								setRenderState( eRsRenderState State, BcS32 Value, BcBool Force = BcFalse );
	BcS32								getRenderState( eRsRenderState State ) const;
	void								setTextureState( BcU32 Sampler, class RsTexture* pTexture, const RsTextureParams& Params, BcBool Force = BcFalse );
	void								flushState();

protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

private:
	RsContextD3D11* pParent_;
	OsClient* pClient_;

	IDXGIAdapter* Adapter_;
	DXGI_SWAP_CHAIN_DESC SwapChainDesc_;
	IDXGISwapChain* SwapChain_;
	ID3D11Device* Device_;
	ID3D11DeviceContext* Context_;
	D3D_FEATURE_LEVEL FeatureLevel_;
	ID3D11Texture2D* BackBuffer_;

	BcThreadId OwningThread_;
	BcBool ScreenshotRequested_;
};


#endif
