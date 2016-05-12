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
#include "System/Renderer/D3D12/RsLinearHeapAllocatorD3D12.h"

#include "System/Renderer/RsViewport.h"

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
	
	virtual OsClient* getClient() const override;
	virtual const RsFeatures& getFeatures() const override;

	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const override;
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const override;

	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	class RsFrameBuffer* getBackBuffer() const override;
	class RsFrameBuffer* beginFrame( BcU32 Width, BcU32 Height ) override;
	void endFrame() override;
	void takeScreenshot( RsScreenshotFunc ScreenshotFunc ) override;

	void clear( 
		const RsFrameBuffer* FrameBuffer,
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil ) override;
	void drawPrimitives( 
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding, 
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer, 
		const RsViewport* Viewport,
		const RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, 
		BcU32 VertexOffset, BcU32 NoofVertices,
		BcU32 FirstInstance, BcU32 NoofInstances ) override;
	void drawIndexedPrimitives( 
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding, 
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer,
		const RsViewport* Viewport,
		const RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, 
		BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset,
		BcU32 FirstInstance, BcU32 NoofInstances ) override;
	void copyTexture( RsTexture* SourceTexture, RsTexture* DestTexture ) override;

	void dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups );

	void beginQuery( class RsQueryHeap* QueryHeap, size_t Idx ) override;
	void endQuery( class RsQueryHeap* QueryHeap, size_t Idx ) override;
	bool isQueryResultAvailible( class RsQueryHeap* QueryHeap, size_t Idx ) override;
	void resolveQueries( class RsQueryHeap* QueryHeap, size_t Offset, size_t NoofQueries, BcU64* OutData ) override;

	void bindFrameBuffer( 
		const RsFrameBuffer* FrameBuffer, 
		const RsViewport* Viewport, 
		const RsScissorRect* ScissorRect );
	void bindInputAssembler( 
		RsTopologyType TopologyType, 
		const RsGeometryBinding* GeometryBinding );
	void bindGraphicsPSO( 
		RsTopologyType TopologyType,
		const RsGeometryBinding* GeometryBinding, 
		const RsProgram* Program,
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer );
	void bindComputePSO( 
		const RsProgram* Program );
	void bindDescriptorHeap(
		const RsProgramBinding* ProgramBinding );

	bool createRenderState(
		RsRenderState* RenderState ) override;
	bool destroyRenderState(
		RsRenderState* RenderState ) override;
	bool createSamplerState(
		RsSamplerState* SamplerState ) override;
	bool destroySamplerState(
		RsSamplerState* SamplerState ) override;

	bool createFrameBuffer( 
		class RsFrameBuffer* FrameBuffer ) override;
	bool destroyFrameBuffer( 
		class RsFrameBuffer* FrameBuffer ) override;

	bool createBuffer( 
		RsBuffer* Buffer ) override;
	bool destroyBuffer( 
		RsBuffer* Buffer ) override;
	bool updateBuffer( 
		RsBuffer* Buffer,
		BcSize Offset,
		BcSize Size,
		RsResourceUpdateFlags Flags,
		RsBufferUpdateFunc UpdateFunc ) override;

	bool createTexture( 
		class RsTexture* Texture ) override;
	bool destroyTexture( 
		class RsTexture* Texture ) override;
	bool updateTexture( 
		class RsTexture* Texture,
		const struct RsTextureSlice& Slice,
		RsResourceUpdateFlags Flags,
		RsTextureUpdateFunc UpdateFunc ) override;

	bool createShader( class RsShader* Shader ) override;
	bool destroyShader( class RsShader* Shader ) override;
	bool createProgram( class RsProgram* Program ) override;
	bool destroyProgram( class RsProgram* Program ) override;
	bool createProgramBinding( class RsProgramBinding* ProgramBinding ) override;
	bool destroyProgramBinding( class RsProgramBinding* ProgramBinding ) override;
	bool createGeometryBinding( class RsGeometryBinding* GeometryBinding ) override;
	bool destroyGeometryBinding( class RsGeometryBinding* GeometryBinding ) override;
	bool createVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) override;
	bool createQueryHeap( class RsQueryHeap* QueryHeap ) override;
	bool destroyQueryHeap( class RsQueryHeap* QueryHeap ) override;

	/**
	 * Delete resources for current frame.
	 */
	void destroyResources();

	/**
	 * Flush out command list, and call a function post execute.
	 */
	void flushCommandList( std::function< void() > PostExecute );

	/**
	 * Create graphics root signature.
	 */
	void createGraphicsRootSignature();

	/**
	 * Create compute root signature.
	 */
	void createComputeRootSignature();

	/**
	 * Create default pipeline state.
	 */
	void createDefaultPSO();

	/**
	 * Create command list data.
	 */
	void createCommandListData( size_t NoofBuffers );

	/**
	 * Recreate backbuffers.
	 */
	void recreateBackBuffers( BcU32 Width, BcU32 Height );

	/**
	 * Get current command list.
	 */
	ID3D12GraphicsCommandList* getCurrentCommandList();

	/**
	 * Get current upload allocator.
	 */
	class RsLinearHeapAllocatorD3D12* getCurrentUploadAllocator();

	/**
	 * Get delayed destroy list.
	 */
	std::vector< ComPtr< ID3D12Object > >& getDelayedDestroyList();


protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

private:
	RsContextD3D12* Parent_;
	OsClient* Client_;

	BcU32 InsideBeginEnd_;
	BcU32 Width_;
	BcU32 Height_;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc_;
	ComPtr< IDXGIFactory4 > Factory_;
	ComPtr< IDXGIAdapter > Adapter_;
	ComPtr< IDXGISwapChain3 > SwapChain_;
	ComPtr< ID3D12Device > Device_;
	D3D_FEATURE_LEVEL FeatureLevel_;

	RsFeatures Features_;

	/// Command queue.
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc_;
	ComPtr< ID3D12CommandQueue > CommandQueue_;

	/// CommandListData
	/// TODO: Factor this stuff out into a new interface to allow for concurrency.
	struct CommandListData
	{
		CommandListData():
			CommandAllocator_(),
			CommandList_(),
			UploadAllocator_(),
			CompleteFence_(),
			CompletionValue_()
		{
		}

		CommandListData( CommandListData&& Other )
		{
			std::swap( CommandAllocator_, Other.CommandAllocator_ );
			std::swap( CommandList_, Other.CommandList_ );
			std::swap( UploadAllocator_, Other.UploadAllocator_ );
			std::swap( CompleteFence_, Other.CompleteFence_ );
			std::swap( CompletionValue_, Other.CompletionValue_ );
		}

		CommandListData& operator == ( CommandListData&& Other )
		{
			std::swap( CommandAllocator_, Other.CommandAllocator_ );
			std::swap( CommandList_, Other.CommandList_ );
			std::swap( UploadAllocator_, Other.UploadAllocator_ );
			std::swap( CompleteFence_, Other.CompleteFence_ );
			std::swap( CompletionValue_, Other.CompletionValue_ );
		}

		/// Command allocator.
		ComPtr< ID3D12CommandAllocator > CommandAllocator_;

		/// Graphics command list.
		ComPtr< ID3D12GraphicsCommandList > CommandList_;

		/// Objects to destroy after command list execution.
		std::vector< ComPtr< ID3D12Object > > ObjectsToDestroy_; 

		// Memory management.
		// TODO: To save memory overall, perhaps have one global one which is reset on frame 0?
		std::unique_ptr< class RsLinearHeapAllocatorD3D12 > UploadAllocator_;

		// Completion fence.
		ComPtr< ID3D12Fence > CompleteFence_;

		// Completion value to wait for.
		BcU64 CompletionValue_;
	};

	// Per frame.
	std::vector< CommandListData > CommandListDatas_;
	int CurrentCommandListData_;
	HANDLE WaitOnCommandListEvent_;

	/// Presenting.
	BcU64 FrameCounter_;
	BcU64 FlushCounter_;
	BcU32 NumSwapBuffers_;
	BcU32 CurrentSwapBuffer_;
	
	/// Graphics pipeline state management.
	ComPtr< ID3D12RootSignature > GraphicsRootSignature_;
	ComPtr< ID3D12RootSignature > ComputeRootSignature_;
	ComPtr< ID3D12PipelineState > DefaultPSO_;
	std::unique_ptr< RsPipelineStateCacheD3D12 > PSOCache_;
	RsGraphicsPipelineStateDescD3D12 GraphicsPSODesc_;
	RsComputePipelineStateDescD3D12 ComputePSODesc_;

	const RsProgram* BoundProgram_ = nullptr;
	const RsGeometryBinding* BoundGeometryBinding_ = nullptr;
	BcU32 BoundFrameBufferFormatHash_ = 0;

	const class RsFrameBuffer* BoundFrameBuffer_ = nullptr;
	RsViewport BoundViewport_ = RsViewport();
	RsScissorRect BoundScissorRect_ = RsScissorRect();
	std::array< D3D12_VIEWPORT, MAX_RENDER_TARGETS > BoundViewports_ = {};
	std::array< D3D12_RECT, MAX_RENDER_TARGETS > BoundScissorRects_ = {};

	// Buffer views.
	std::array< D3D12_VERTEX_BUFFER_VIEW, MAX_VERTEX_STREAMS > BoundVertexBufferViews_;
	D3D12_INDEX_BUFFER_VIEW BoundIndexBufferView_;
	
	/// Backbuffer.
	std::vector< class RsFrameBuffer* > BackBufferFB_; // TODO: Need multiple.
	std::vector< class RsTexture* > BackBufferRT_;	// TODO: Need multiple.
	class RsTexture* BackBufferDS_;

	BcThreadId OwningThread_;
	BcBool ScreenshotRequested_;


};

#endif
