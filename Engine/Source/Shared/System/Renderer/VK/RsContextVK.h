/**************************************************************************
*
* File:		RsContextVK.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __RSCONTEXTVK_H__
#define __RSCONTEXTVK_H__

#include "System/Renderer/RsContext.h"
#include "System/Renderer/VK/RsVK.h"

#include "Base/BcMisc.h"

#include <unordered_map>

//////////////////////////////////////////////////////////////////////////
// RsContextVK
class RsContextVK:
	public RsContext
{
public:
	RsContextVK( OsClient* pClient, RsContextVK* pParent );
	virtual ~RsContextVK();
	
	virtual OsClient* getClient() const override;
	virtual const RsFeatures& getFeatures() const override;

	virtual BcBool isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const override;
	virtual RsShaderCodeType maxShaderCodeType( RsShaderCodeType CodeType ) const override;

	BcU32 getWidth() const override;
	BcU32 getHeight() const override;
	class RsFrameBuffer* getBackBuffer() const override;
	RsFrameBuffer* beginFrame( BcU32 Width, BcU32 Height ) override;
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
		BcU32 VertexOffset, BcU32 NoofVertices ) override;
	void drawIndexedPrimitives( 
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding, 
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer,
		const RsViewport* Viewport,
		const RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, 
		BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset ) override;
	void copyTexture( RsTexture* SourceTexture, RsTexture* DestTexture ) override;

	void dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups );

	void bindFrameBuffer( 
		const RsFrameBuffer* FrameBuffer, 
		const RsViewport* Viewport, const RsScissorRect* ScissorRect,
		BcU32 NoofClearValues, const VkClearValue* ClearValues );
	void bindGraphicsPSO( 
		RsTopologyType TopologyType,
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding,
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer );

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

	bool createShader( class RsShader* Shader );
	bool destroyShader( class RsShader* Shader );

	bool createProgram( class RsProgram* Program );
	bool destroyProgram( class RsProgram* Program );
	bool createProgramBinding( class RsProgramBinding* ProgramBinding ) override;
	bool destroyProgramBinding( class RsProgramBinding* ProgramBinding ) override;
	bool createGeometryBinding( class RsGeometryBinding* GeometryBinding ) override;
	bool destroyGeometryBinding( class RsGeometryBinding* GeometryBinding ) override;
	bool createVertexDeclaration( class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration( class RsVertexDeclaration* VertexDeclaration  ) override;

	void flushState();


protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

	void createDescriptorLayouts();

private:
	RsContextVK* pParent_ = nullptr;
	OsClient* pClient_ = nullptr;
	BcU32 Width_ = 0;
	BcU32 Height_ = 0;
	BcThreadId OwningThread_ = 0;
	RsFeatures Features_;

	// Instance info.
	std::vector< VkLayerProperties > InstanceLayers_;
	std::vector< VkExtensionProperties > InstanceExtensions_;
	std::vector< VkLayerProperties > DeviceLayers_;
	std::vector< VkExtensionProperties > DeviceExtensions_;
	VkAllocationCallbacks* AllocationCallbacks_ = nullptr;
	VkInstance Instance_ = 0;
	std::vector< VkPhysicalDevice > PhysicalDevices_;
	VkDevice Device_ = 0;
	VkPhysicalDeviceProperties DeviceProps_ = {};
	std::vector< VkQueueFamilyProperties > DeviceQueueProps_;

	// KHR.
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR fpGetPhysicalDeviceSurfaceSupportKHR_ = nullptr;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR_ = nullptr;
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR_ = nullptr;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR_ = nullptr;

	PFN_vkCreateSwapchainKHR fpCreateSwapchainKHR_ = nullptr;
	PFN_vkDestroySwapchainKHR fpDestroySwapchainKHR_ = nullptr;
	PFN_vkGetSwapchainImagesKHR fpGetSwapchainImagesKHR_ = nullptr;
	PFN_vkAcquireNextImageKHR fpAcquireNextImageKHR_ = nullptr;
	PFN_vkQueuePresentKHR fpQueuePresentKHR_ = nullptr;
	VkSurfaceKHR WindowSurface_= {};
	VkSurfaceCapabilitiesKHR WindowSurfaceCapabilities_ = {};

	// Debug
	PFN_vkDbgCreateMsgCallback fpCreateMsgCallback_ = nullptr;
	PFN_vkDbgDestroyMsgCallback fpDestroyMsgCallback_ = nullptr;
	PFN_vkDbgMsgCallback fpBreakCallback_ = nullptr;
	VkDbgMsgCallback DebugCallback_ = 0;

	// Queues.
	VkQueue GraphicsQueue_ = nullptr;

	// Formats.
	std::vector< VkSurfaceFormatKHR > SurfaceFormats_;

	// Command pool & buffer.
	VkCommandPoolCreateInfo CommandPoolCreateInfo_ = {};
	VkCommandPool CommandPool_ = 0;
	VkCommandBufferAllocateInfo CommandBufferAllocateInfo_ = {};

	std::array< VkCommandBuffer, 2 > CommandBuffers_;
	BcU32 CurrentCommandBuffer_ = 0;
	VkCommandBuffer getCommandBuffer() { return CommandBuffers_[ CurrentCommandBuffer_ ]; }

	// Swap chain
	VkSwapchainCreateInfoKHR SwapChainCreateInfo_ = {};
	VkSwapchainKHR SwapChain_ = 0;
	std::vector< VkImage > SwapChainImages_;

	std::vector< class RsTexture* > SwapChainTextures_;
	RsTexture* DepthStencilTexture_;

	// Frame buffers.
	std::vector< class RsFrameBuffer* > FrameBuffers_ = {};
	uint32_t CurrentFrameBuffer_ = 0;
	const RsFrameBuffer* BoundFrameBuffer_ = nullptr;
	VkRenderPass BoundRenderPass_;

	// Internal utilities.
	std::unique_ptr< class RsAllocatorVK > Allocator_;

	// Inside frame?
	bool InsideBeginEndFrame_ = false;

	// Descriptor layout.
	VkDescriptorSetLayout GraphicsDescriptorSetLayout_;
	VkPipelineLayout GraphicsPipelineLayout_;
	VkDescriptorSetLayout ComputeDescriptorSetLayout_;
	VkPipelineLayout ComputePipelineLayout_;

	// PSO cache.
	using PSOBindingTuple = std::tuple< 
		RsTopologyType, 
		const RsGeometryBinding*,
		const RsProgram*, 
		const RsRenderState*, 
		const RsFrameBuffer*,
		VkRenderPass >;
	std::map< PSOBindingTuple, VkPipeline > PSOCache_;
	VkPipelineCache PipelineCache_;
};

#endif
