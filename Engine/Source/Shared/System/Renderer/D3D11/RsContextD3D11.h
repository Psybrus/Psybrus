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

#include "Base/BcComRef.h"
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

protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

private:
	RsContextD3D11* pParent_;
	OsClient* pClient_;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc_;
	BcComRef<IDXGIAdapter> Adapter_;
	BcComRef<IDXGISwapChain> SwapChain_;
	BcComRef<ID3D11Device> Device_;
	BcComRef<ID3D11DeviceContext> Context_;
	D3D_FEATURE_LEVEL FeatureLevel_;

	BcComRef<ID3D11Texture2D> BackBuffer_;

	BcThreadId OwningThread_;
	BcBool ScreenshotRequested_;
};


#endif
