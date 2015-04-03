/**************************************************************************
*
* File:		RsContextD3D12.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXTD3D12_H__
#define __RSCONTEXTD3D12_H__

#include "System/Renderer/RsContext.h"
#include "System/Renderer/D3D12/RsD3D12.h"

#include "Base/BcMisc.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// RsContextD3D12
class RsContextD3D12:
	public RsContext
{
public:
	RsContextD3D12( OsClient* pClient, RsContextD3D12* pParent );
	virtual ~RsContextD3D12();
	
	virtual BcU32 getWidth() const override;
	virtual BcU32 getHeight() const override;
	virtual OsClient* getClient() const override;

	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const override;
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const override;

	void presentBackBuffer();
	void takeScreenshot();
	void setViewport( class RsViewport& Viewport );

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( class RsRenderState* RenderState );
	void setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState );
	void setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force = BcFalse );
	BcS32 getRenderState( RsRenderStateType State ) const;
	void setSamplerState( BcU32 SlotIdx, const RsTextureParams& Params, BcBool Force = BcFalse );
	void setTexture( BcU32 SlotIdx, class RsTexture* pTexture, BcBool Force = BcFalse );
	void setProgram( class RsProgram* Program );
	void setIndexBuffer( class RsBuffer* IndexBuffer );
	void setVertexBuffer( 
		BcU32 StreamIdx, 
		class RsBuffer* VertexBuffer,
		BcU32 Stride );
	void setUniformBuffer( 
		BcU32 Handle, 
		class RsBuffer* UniformBuffer );
	void setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration );
	void setFrameBuffer( class RsFrameBuffer* FrameBuffer );
	void clear(
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil );

	void drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices );
	void drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset );

	bool createRenderState(
		RsRenderState* RenderState );
	bool destroyRenderState(
		RsRenderState* RenderState );
	bool createSamplerState(
		RsSamplerState* SamplerState );
	bool destroySamplerState(
		RsSamplerState* SamplerState );

	bool createFrameBuffer( 
		class RsFrameBuffer* FrameBuffer );
	bool destroyFrameBuffer( 
		class RsFrameBuffer* FrameBuffer );

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

	bool createShader(
		class RsShader* Shader );
	bool destroyShader(
		class RsShader* Shader );

	bool createProgram(
		class RsProgram* Program );
	bool destroyProgram(
		class RsProgram* Program );
	
	void flushState();


protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

private:
	RsContextD3D12* pParent_;
	OsClient* pClient_;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc_;
	ComPtr< IDXGIAdapter > Adapter_;
	ComPtr< IDXGISwapChain > SwapChain_;
	ComPtr< ID3D12Device > Device_;
	D3D_FEATURE_LEVEL FeatureLevel_;
	BcU32 FrameCounter_;

	RsTexture* BackBufferRT_;
	RsTexture* BackBufferDS_;
	size_t BackBufferRTResourceIdx_;
	size_t BackBufferDSResourceIdx_;

	BcThreadId OwningThread_;
	BcBool ScreenshotRequested_;


};

#endif
