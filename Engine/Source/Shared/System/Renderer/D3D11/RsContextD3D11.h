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
	
	BcU32 getWidth() const;
	BcU32 getHeight() const;

	BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const;
	RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const;
	void takeScreenshot();
	void setViewport( class RsViewport& Viewport );

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force = BcFalse );
	BcS32 getRenderState( RsRenderStateType State ) const;
	void setTextureState( BcU32 Sampler, class RsTexture* pTexture, const RsTextureParams& Params, BcBool Force = BcFalse );
	void setProgram( class RsProgram* Program );
	void setIndexBuffer( class RsBuffer* IndexBuffer );
	void setVertexBuffer( 
		BcU32 StreamIdx, 
		class RsBuffer* VertexBuffer,
		BcU32 Stride );
	void setUniformBuffer( 
		BcU32 SlotIdx, 
		class RsBuffer* UniformBuffer );
	void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration );
	void clear( const RsColour& Colour );
	void drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices );
	void drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset );

	bool createBuffer( 
		RsBuffer* Buffer );
	bool destroyBuffer( 
		RsBuffer* Buffer );
	bool updateBuffer( 
		RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc );

	bool createTexture( 
		class RsTexture* Texture );
	bool destroyTexture( 
		class RsTexture* Texture );
	bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc );
		
	void flushState();


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
