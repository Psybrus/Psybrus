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
	void beginFrame( BcU32 Width, BcU32 Height ) override;
	void endFrame() override;
	void takeScreenshot( RsScreenshotFunc ScreenshotFunc ) override;
	void setViewport( const class RsViewport& Viewport ) override;
	void setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height ) override;

	void setDefaultState();
	void invalidateRenderState();
	void invalidateTextureState();
	void setRenderState( class RsRenderState* RenderState );
	void setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState );
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
	
	bool createVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration ) override;
	bool destroyVertexDeclaration(
		class RsVertexDeclaration* VertexDeclaration  ) override;

	void flushState();


protected:
	virtual void create();
	virtual void update();
	virtual void destroy();	

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
	VkInstance Instance_ = 0;
	std::vector< VkPhysicalDevice > PhysicalDevices_;
	VkDevice Device_ = 0;
	VkPhysicalDeviceProperties DeviceProps_ = {};
	std::vector< VkPhysicalDeviceQueueProperties > DeviceQueueProps_;

	// WSI.
	PFN_vkGetPhysicalDeviceSurfaceSupportWSI fpGetPhysicalDeviceSurfaceSupportWSI_ = nullptr;
	PFN_vkGetSurfaceInfoWSI fpGetSurfaceInfoWSI_ = nullptr;
	PFN_vkCreateSwapChainWSI fpCreateSwapChainWSI_ = nullptr;
	PFN_vkDestroySwapChainWSI fpDestroySwapChainWSI_ = nullptr;
	PFN_vkGetSwapChainInfoWSI fpGetSwapChainInfoWSI_ = nullptr;
	PFN_vkAcquireNextImageWSI fpAcquireNextImageWSI_ = nullptr;
	PFN_vkQueuePresentWSI fpQueuePresentWSI_ = nullptr;
	VkSurfaceDescriptionWindowWSI WindowSurfaceDesc_ = {};

	// Debug
	PFN_vkDbgCreateMsgCallback fpCreateMsgCallback_ = nullptr;
	PFN_vkDbgDestroyMsgCallback fpDestroyMsgCallback_ = nullptr;
	PFN_vkDbgMsgCallback fpBreakCallback_ = nullptr;
	VkDbgMsgCallback DebugCallback_ = 0;

	// Queues.
	VkQueue GraphicsQueue_ = nullptr;

	// Formats.
	std::vector< VkSurfaceFormatPropertiesWSI > SurfaceFormats_;

	// Command pool & buffer.
	VkCmdPoolCreateInfo CommandPoolCreateInfo_ = {};
	VkCmdPool CommandPool_ = 0;
	VkCmdBufferCreateInfo CommandBufferCreateInfo_ = {};


	VkCmdBuffer CommandBuffer_ = 0;

	// Swap chain
	VkSwapChainCreateInfoWSI SwapChainCreateInfo_ = {};
	VkSwapChainWSI SwapChain_ = 0;
	std::vector< VkSwapChainImagePropertiesWSI > SwapChainImages_;

	std::vector< class RsTexture* > SwapChainTextures_;
	RsTexture* DepthStencilTexture_;

	// Frame buffers.
	std::vector< class RsFrameBuffer* > FrameBuffers_ = {};
	uint32_t CurrentFrameBuffer_ = 0;

	// Internal utilities.
	std::unique_ptr< class RsAllocatorVK > Allocator_;

	// Inside frame?
	bool InsideBeginEndFrame_ = false;
};

#endif
