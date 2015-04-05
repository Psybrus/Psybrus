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
#include "System/Renderer/D3D12/RsPipelineStateCacheD3D12.h"
#include "System/Renderer/D3D12/RsDescriptorHeapCacheD3D12.h"

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
	
	bool createVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration  ) override;

	void flushState();

	/**
	 * Flush out command list.
	 * NOTE: This is temporary. Used until we utilise multiple command lists.
	 */
	void flushCommandList();

	/**
	 * (Re)create backbuffer.
	 */
	void recreateBackBuffer();

	/**
	 * Create default root signature.
	 */
	void createDefaultRootSignature();

	/**
	 * Create default pipeline state.
	 */
	void createDefaultPSO();

	/**
	 * Create command allocators.
	 */
	void createCommandAllocators();

	/**
	 * Create resource allocators.
	 */
	void createResourceAllocators();

	/**
	 * Create command lists.
	 */
	void createCommandLists();


protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

private:
	RsContextD3D12* Parent_;
	OsClient* Client_;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc_;
	ComPtr< IDXGIFactory > Factory_;
	ComPtr< IDXGIAdapter > Adapter_;
	ComPtr< IDXGISwapChain > SwapChain_;
	ComPtr< ID3D12Device > Device_;
	D3D_FEATURE_LEVEL FeatureLevel_;

	/// Command queue.
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc_;
	ComPtr< ID3D12CommandQueue > CommandQueue_;

	/// Command allocator.
	ComPtr< ID3D12CommandAllocator > CommandAllocator_;

	/// Graphics command list.
	ComPtr< ID3D12GraphicsCommandList > CommandList_;

	/// Presenting.
	ComPtr< ID3D12Fence > PresentFence_;
	HANDLE PresentEvent_;
	BcU64 FrameCounter_;
	BcU64 FlushCounter_;
	BcU32 NumSwapBuffers_;
	BcU32 LastSwapBuffer_;
	
	/// Graphics pipeline state management.
	ComPtr< ID3D12RootSignature > DefaultRootSignature_;
	ComPtr< ID3D12PipelineState > DefaultPSO_;
	std::unique_ptr< RsPipelineStateCacheD3D12 > PSOCache_;
	RsGraphicsPipelineStateDescD3D12 GraphicsPSODesc_;
	std::array< D3D12_VIEWPORT, MAX_RENDER_TARGETS > Viewports_;
	std::array< D3D12_RECT, MAX_RENDER_TARGETS > ScissorRects_;
	class RsFrameBuffer* FrameBuffer_;

	// Memory management.
	std::unique_ptr< class RsLinearHeapAllocatorD3D12 > UploadAllocator_;

	// Buffer views.
	std::array< D3D12_VERTEX_BUFFER_VIEW, MAX_VERTEX_STREAMS > VertexBufferViews_;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView_;
	
	// Descriptor heaps.
	std::unique_ptr< RsDescriptorHeapCacheD3D12 > DHCache_;
	RsDescriptorHeapSamplerStateDescArrayD3D12 SamplerStateDescs_;
	RsDescriptorHeapShaderResourceDescArrayD3D12 ShaderResourceDescs_;

	/// Backbuffer.
	class RsTexture* BackBufferRT_;
	class RsTexture* BackBufferDS_;
	class RsFrameBuffer* BackBufferFB_;

	BcThreadId OwningThread_;
	BcBool ScreenshotRequested_;


};

#endif
