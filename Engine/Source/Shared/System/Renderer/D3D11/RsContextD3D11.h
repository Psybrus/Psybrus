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
	
	OsClient* getClient() const override;
	const RsFeatures& getFeatures() const override;

	BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const override;
	RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const override;

	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	void beginFrame( BcU32 Width, BcU32 Height ) override;
	void endFrame() override;
	void takeScreenshot( RsScreenshotFunc ScreenshotFunc ) override;

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

	void setViewport( const class RsViewport& Viewport ) override;
	void setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height ) override;

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
	
	bool createVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration  ) override;

	void flushState();


protected:
	virtual void						create();
	virtual void						update();
	virtual void						destroy();	

private:
	RsContextD3D11* pParent_;
	OsClient* pClient_;

	BcU32 InsideBeginEnd_;
	BcU32 Width_;
	BcU32 Height_;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc_;
	BcComRef< IDXGIAdapter > Adapter_;
	BcComRef< IDXGISwapChain > SwapChain_;
	BcComRef< ID3D11Device > Device_;
	BcComRef< ID3D11DeviceContext > Context_;
	D3D_FEATURE_LEVEL FeatureLevel_;

	RsFeatures Features_;

	BcComRef<ID3D11Texture2D> BackBuffer_;
	RsTexture* BackBufferRT_;
	RsTexture* BackBufferDS_;
	size_t BackBufferRTResourceIdx_;
	size_t BackBufferDSResourceIdx_;

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

		DXGI_FORMAT ResourceFormat_;
		DXGI_FORMAT DSVFormat_;
		DXGI_FORMAT SRVFormat_;

		ID3D11ShaderResourceView* ShaderResourceView_;
		ID3D11UnorderedAccessView* UnorderedAccessView_;
		ID3D11RenderTargetView* RenderTargetView_;
		ID3D11DepthStencilView* DepthStencilView_;
	};

	std::vector< ResourceViewCacheEntry > ResourceViewCache_;
	std::vector< size_t > ResourceViewCacheFreeIdx_;

	size_t addD3DResource( 
		ID3D11Resource* D3DResource, 
		DXGI_FORMAT ResourceFormat,
		DXGI_FORMAT DSVFormat,
		DXGI_FORMAT SRVFormat );
	void delD3DResource( size_t ResourceIdx );
	ID3D11Resource* getD3DResource( size_t ResourceIdx );
	ID3D11Buffer* getD3DBuffer( size_t ResourceIdx );
	ID3D11Texture1D* getD3DTexture1D( size_t ResourceIdx );
	ID3D11Texture2D* getD3DTexture2D( size_t ResourceIdx );
	ID3D11Texture3D* getD3DTexture3D( size_t ResourceIdx );
	ID3D11ShaderResourceView* getD3DShaderResourceView( size_t ResourceIdx );
	ID3D11UnorderedAccessView* getD3DUnorderedAccessView( size_t ResourceIdx );
	ID3D11RenderTargetView* getD3DRenderTargetView( size_t ResourceIdx );
	ID3D11DepthStencilView* getD3DDepthStencilView( size_t ResourceIdx );
	
	// Index buffers.
	RsBuffer* IndexBuffer_;
	ID3D11Buffer* D3DIndexBuffer_;

	// Vertex buffers.
	std::array< RsBuffer*, MAX_VERTEX_STREAMS > VertexBuffers_;
	std::array< ID3D11Buffer*, MAX_VERTEX_STREAMS > D3DVertexBuffers_;
	std::array< UINT, MAX_VERTEX_STREAMS > D3DVertexBufferStrides_;
	std::array< UINT, MAX_VERTEX_STREAMS > D3DVertexBufferOffsets_;

	// Constant buffers.
	typedef std::array< RsBuffer*, MAX_UNIFORM_SLOTS > UniformBufferSlots;
	typedef std::array< ID3D11Buffer*, MAX_UNIFORM_SLOTS > D3DConstantBufferSlots;
	typedef std::array< UniformBufferSlots, (size_t)RsShaderType::MAX > UniformBuffers;
	typedef std::array< D3DConstantBufferSlots, (size_t)RsShaderType::MAX > D3DConstantBuffers;
	
	UniformBuffers UniformBuffers_;
	D3DConstantBuffers D3DConstantBuffers_;

	// Shader resources.
	typedef std::array< RsTexture*, MAX_TEXTURE_SLOTS > TextureSlots;
	typedef std::array< ID3D11ShaderResourceView*, MAX_TEXTURE_SLOTS > D3DShaderResourceViewSlots;
	typedef std::array< TextureSlots, (size_t)RsShaderType::MAX > Textures;
	typedef std::array< D3DShaderResourceViewSlots, (size_t)RsShaderType::MAX > D3DShaderResourceViews;
	typedef std::array< ID3D11SamplerState*, MAX_TEXTURE_SLOTS > D3DSamplerStateSlots;
	typedef std::array< D3DSamplerStateSlots, (size_t)RsShaderType::MAX > D3DSamplerStates;

	Textures Textures_;
	D3DShaderResourceViews D3DShaderResourceViews_;
	D3DSamplerStates D3DSamplerStates_;

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

	typedef std::unordered_map< BcU32, BcComRef< ID3D11BlendState > > BlendStateCache;
	typedef std::unordered_map< BcU32, BcComRef< ID3D11RasterizerState > > RasterizerStateCache;
	typedef std::unordered_map< BcU32, BcComRef< ID3D11DepthStencilState > > DepthStencilStateCache;

	BlendStateCache BlendStateCache_;
	RasterizerStateCache RasterizerStateCache_;
	DepthStencilStateCache DepthStencilStateCache_;

	struct RenderStateInternal
	{
		BcComRef< ID3D11BlendState > Blend_;
		BcComRef< ID3D11RasterizerState > Rasterizer_;
		BcComRef< ID3D11DepthStencilState > DepthStencil_;
	};

	struct SamplerStateInternal
	{
		BcComRef< ID3D11SamplerState > Sampler_;
		size_t LastFrameUsed_;
	};

	typedef std::map< BcU32, RenderStateInternal > RenderStateCache;
	typedef std::map< BcU32, SamplerStateInternal > SamplerStateCache;

	BcU32 FrameCounter_;
	RenderStateCache RenderStateCache_;
	SamplerStateCache SamplerStateCache_;

	RsRenderState* BoundRenderState_;

	// Render targets.
	typedef std::array< ID3D11RenderTargetView*, 8 > RenderTargetViews;
	
	RenderTargetViews RenderTargetViews_;
	ID3D11DepthStencilView* DepthStencilView_;
};

#endif
