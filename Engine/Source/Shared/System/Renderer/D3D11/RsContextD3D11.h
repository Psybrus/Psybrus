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

#include <unordered_map>

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
	void presentBackBuffer();
	void takeScreenshot();
	void setViewport( class RsViewport& Viewport );

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( RsRenderStateType State, BcS32 Value, BcBool Force = BcFalse );
	BcS32 getRenderState( RsRenderStateType State ) const;
	void setTextureState( BcU32 SlotIdx, class RsTexture* pTexture, const RsTextureParams& Params, BcBool Force = BcFalse );
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
	BcU32 BackBufferResourceIdx_;

	BcThreadId OwningThread_;
	BcBool ScreenshotRequested_;

	// Resource view cache.
	struct ResourceViewCacheEntry
	{
		union
		{
			ID3D11Resource* Resource_;
			ID3D11Buffer* BufferResource_;
			ID3D11Texture1D* Texture1DResource_;
			ID3D11Texture2D* Texture2DResource_;
			ID3D11Texture3D* Texture3DResource_;
		};

		ID3D11ShaderResourceView* ShaderResourceView_;
		ID3D11UnorderedAccessView* UnorderedAccessView_;
		ID3D11RenderTargetView* RenderTargetView_;
		ID3D11DepthStencilView* DepthStencilView_;
	};

	std::vector< ResourceViewCacheEntry > ResourceViewCache_;
	std::vector< BcU32 > ResourceViewCacheFreeIdx_;

	BcU32 addD3DResource( ID3D11Resource* D3DResource );
	void delD3DResource( BcU32 ResourceIdx );
	ID3D11Resource* getD3DResource( BcU32 ResourceIdx );
	ID3D11Buffer* getD3DBuffer( BcU32 ResourceIdx );
	ID3D11Texture1D* getD3DTexture1D( BcU32 ResourceIdx );
	ID3D11Texture2D* getD3DTexture2D( BcU32 ResourceIdx );
	ID3D11Texture3D* getD3DTexture3D( BcU32 ResourceIdx );
	ID3D11ShaderResourceView* getD3DShaderResourceView( BcU32 ResourceIdx );
	ID3D11UnorderedAccessView* getD3DUnorderedAccessView( BcU32 ResourceIdx );
	ID3D11RenderTargetView* getD3DRenderTargetView( BcU32 ResourceIdx );
	ID3D11DepthStencilView* getD3DDepthStencilView( BcU32 ResourceIdx );
	
	// Index buffers.
	RsBuffer* IndexBuffer_;
	ID3D11Buffer* D3DIndexBuffer_;

	// Vertex buffers.
	std::array< RsBuffer*, MAX_VERTEX_STREAMS > VertexBuffers_;
	std::array< ID3D11Buffer*, MAX_VERTEX_STREAMS > D3DVertexBuffers_;
	std::array< UINT, MAX_VERTEX_STREAMS > D3DVertexBufferStrides_;
	std::array< UINT, MAX_VERTEX_STREAMS > D3DVertexBufferOffsets_;

	// Constant buffers.
	std::array< RsBuffer*, MAX_UNIFORM_SLOTS > UniformBuffers_;
	std::array< ID3D11Buffer*, MAX_UNIFORM_SLOTS > D3DConstantBuffers_;
	
	// Shader resources.
	std::array< RsTexture*, MAX_TEXTURE_SLOTS > TextureResources_;
	std::array< ID3D11ShaderResourceView*, MAX_TEXTURE_SLOTS > D3DShaderResourceViews_;

	// Input layout, program, topology.
	BcBool InputLayoutChanged_;
	RsProgram* Program_;
	RsVertexDeclaration* VertexDeclaration_;
	RsTopologyType TopologyType_;

	// Input assembly mapping.
	struct InputLayout
	{
		BcComRef< ID3D11InputLayout > InputLayout_;
		BcU32 LastUsedFrame_;
	};

	BcU32 generateInputLayoutHash() const;

	typedef std::unordered_map< BcU32, InputLayout > InputLayoutMap; 
	InputLayoutMap InputLayoutMap_;

};


#endif
