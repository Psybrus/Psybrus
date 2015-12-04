/**************************************************************************
*
* File:		RsContextVK.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/VK/RsContextVK.h"

#include "System/Renderer/VK/RsAllocatorVK.h"
#include "System/Renderer/VK/RsBufferVK.h"
#include "System/Renderer/VK/RsCommandBufferVK.h"
#include "System/Renderer/VK/RsFrameBufferVK.h"
#include "System/Renderer/VK/RsTextureVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsRenderState.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsShader.h"
#include "System/Renderer/RsTexture.h"
#include "System/Renderer/RsVertexDeclaration.h"
#include "System/Renderer/RsViewport.h"


#include "Base/BcMath.h"

#include "System/Os/OsClient.h"

#include "Import/Img/Img.h"

//////////////////////////////////////////////////////////////////////////
// Utility
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
{                                                                       \
	fp##entrypoint##_ = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk"#entrypoint); \
	if (fp##entrypoint##_ == NULL) {                                 \
		BcAssertMsg( false, "vkGetInstanceProcAddr failed to find vk"#entrypoint,  \
				 "vkGetInstanceProcAddr Failure");                      \
	}                                                                   \
}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
{                                                                       \
	fp##entrypoint##_ = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint);   \
	if (fp##entrypoint##_ == NULL) {                                 \
		BcAssertMsg( false, "vkGetDeviceProcAddr failed to find vk"#entrypoint,    \
				 "vkGetDeviceProcAddr Failure");                        \
	}                                                                   \
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsContextVK::RsContextVK( OsClient* pClient, RsContextVK* pParent ):
	RsContext( pParent ),
	pParent_( pParent ),
	pClient_( pClient )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsContextVK::~RsContextVK()
{

}

//////////////////////////////////////////////////////////////////////////
// getClient
//virtual
OsClient* RsContextVK::getClient() const
{
	return pClient_;
}

//////////////////////////////////////////////////////////////////////////
// getFeatures
//virtual
const RsFeatures& RsContextVK::getFeatures() const
{
	return Features_;
}

//////////////////////////////////////////////////////////////////////////
// isShaderCodeTypeSupported
//virtual
BcBool RsContextVK::isShaderCodeTypeSupported( RsShaderCodeType CodeType ) const
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// maxShaderCodeType
//virtual
RsShaderCodeType RsContextVK::maxShaderCodeType( RsShaderCodeType CodeType ) const
{
	return RsShaderCodeType::MAX;
}

//////////////////////////////////////////////////////////////////////////
// getWidth
//virtual
BcU32 RsContextVK::getWidth() const
{
	return Width_;
}

//////////////////////////////////////////////////////////////////////////
// getHeight
//virtual
BcU32 RsContextVK::getHeight() const
{
	return Height_;
}

//////////////////////////////////////////////////////////////////////////
// getBackBuffer
RsFrameBuffer* RsContextVK::getBackBuffer() const
{
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// beginFrame
RsFrameBuffer* RsContextVK::beginFrame( BcU32 Width, BcU32 Height )
{
	BcAssert( !InsideBeginEndFrame_ );
	InsideBeginEndFrame_ = true;

	Width_ = Width;
	Height_ = Height;

	// Command buffer setup.
	VkCmdBufferBeginInfo CommandBufferInfo = {};
	CommandBufferInfo.sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO;
	CommandBufferInfo.pNext = nullptr;
	CommandBufferInfo.flags = VK_CMD_BUFFER_OPTIMIZE_SMALL_BATCH_BIT | VK_CMD_BUFFER_OPTIMIZE_ONE_TIME_SUBMIT_BIT;

	// Set render pass for frame buffer.
	RsFrameBuffer* FrameBuffer = FrameBuffers_[ CurrentFrameBuffer_ ];
	RsFrameBufferVK* FrameBufferVK = FrameBuffer->getHandle< RsFrameBufferVK* >();

	VkClearValue ClearValues[ 2 ];
	ClearValues[ 0 ].color.float32[ 0 ] = 1.0f;
	ClearValues[ 0 ].color.float32[ 1 ] = 0.0f;
	ClearValues[ 0 ].color.float32[ 2 ] = 1.0f;
	ClearValues[ 0 ].color.float32[ 3 ] = 1.0f;
	ClearValues[ 1 ].depthStencil.depth = 0.0f;
	ClearValues[ 1 ].depthStencil.stencil = 0;

	VkRenderPassBeginInfo BeginInfo = {};
	BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	BeginInfo.pNext = nullptr;
	BeginInfo.renderPass = FrameBufferVK->getRenderPass();
	BeginInfo.framebuffer = FrameBufferVK->getFrameBuffer();
	BeginInfo.renderArea.offset.x = 0;
	BeginInfo.renderArea.offset.y = 0;
	BeginInfo.renderArea.extent.width = FrameBuffer->getDesc().RenderTargets_[ 0 ]->getDesc().Width_;
	BeginInfo.renderArea.extent.height = FrameBuffer->getDesc().RenderTargets_[ 0 ]->getDesc().Height_;
	BeginInfo.clearValueCount = 2;
	BeginInfo.pClearValues = ClearValues;

	// Begin render pass.
	vkCmdBeginRenderPass( CommandBuffer_, &BeginInfo, VK_RENDER_PASS_CONTENTS_INLINE );

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextVK::endFrame()
{
	BcAssert( InsideBeginEndFrame_ );
	InsideBeginEndFrame_ = false;

	// End last pass and 
	vkCmdEndRenderPass( CommandBuffer_ );

	// End command buffer.
	auto RetVal = vkEndCommandBuffer( CommandBuffer_ );
	BcAssert( !RetVal );

	// Create semaphore to wait for queue to complete.
	VkSemaphore PresentCompleteSemaphore;
	VkSemaphoreCreateInfo PresentCompleteSemaphoreCreateInfo = {};
	PresentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	PresentCompleteSemaphoreCreateInfo.pNext = nullptr;
	PresentCompleteSemaphoreCreateInfo.flags = 0;

	RetVal = vkCreateSemaphore( Device_, &PresentCompleteSemaphoreCreateInfo, &PresentCompleteSemaphore );
	BcAssert( !RetVal );

	// Get the index of the next available swapchain image.
	RetVal = fpAcquireNextImageKHR_( Device_, SwapChain_,
		UINT64_MAX,
		PresentCompleteSemaphore,
		&CurrentFrameBuffer_ );
	BcAssert( !RetVal );

	// Wait for the present complete semaphore to be signaled to ensure
	// that the image won't be rendered to until the presentation
	// engine has fully released ownership to the application, and it is
	// okay to render to the image.
	vkQueueWaitSemaphore( GraphicsQueue_, PresentCompleteSemaphore );

	// Submit queue.
	VkFence NullFence = { VK_NULL_HANDLE };
	RetVal = vkQueueSubmit( GraphicsQueue_, 1, &CommandBuffer_, NullFence );
	BcAssert( !RetVal );

	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = nullptr;
	PresentInfo.swapchainCount = 1;
	PresentInfo.swapchains = &SwapChain_;
	PresentInfo.imageIndices = &CurrentFrameBuffer_;

	RetVal = fpQueuePresentKHR_( GraphicsQueue_, &PresentInfo );
	BcAssert( !RetVal );

	RetVal = vkQueueWaitIdle( GraphicsQueue_ );
	BcAssert( !RetVal );

	vkDestroySemaphore( Device_, PresentCompleteSemaphore );

	// Begin command buffer.
	VkCmdBufferBeginInfo CommandBufferInfo = {};
	CommandBufferInfo.sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO;
	CommandBufferInfo.pNext = nullptr;
	CommandBufferInfo.flags = VK_CMD_BUFFER_OPTIMIZE_SMALL_BATCH_BIT | VK_CMD_BUFFER_OPTIMIZE_ONE_TIME_SUBMIT_BIT;
	RetVal = vkBeginCommandBuffer( CommandBuffer_, &CommandBufferInfo );
	BcAssert( !RetVal );

}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextVK::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextVK::create()
{
	VkResult RetVal = VK_SUCCESS;

	// Get owning thread so we can check we are being called
	// from the appropriate thread later.
	OwningThread_ = BcCurrentThreadId();

	// Enabled layers + extensions.
	std::vector< const char* > EnabledInstanceLayers;
	std::vector< const char* > EnabledInstanceExtensions;
	std::vector< const char* > EnabledDeviceLayers;
	std::vector< const char* > EnabledDeviceExtensions;

	// Grab layers.
	uint32_t InstanceLayerCount = 0;
	if( ( RetVal = vkEnumerateInstanceLayerProperties( &InstanceLayerCount, nullptr ) ) == VK_SUCCESS )
	{
		InstanceLayers_.resize( InstanceLayerCount );
		RetVal = vkEnumerateInstanceLayerProperties( &InstanceLayerCount, InstanceLayers_.data() );
		BcAssert( !RetVal );
#if 1
		for( const auto& InstanceLayer : InstanceLayers_ )
		{
			if( strstr( InstanceLayer.layerName, "MemTracker" ) )
			{
				EnabledInstanceLayers.push_back( InstanceLayer.layerName );
			}
			if( strstr( InstanceLayer.layerName, "ObjectTracker" ) )
			{
				EnabledInstanceLayers.push_back( InstanceLayer.layerName );
			}
			if( strstr( InstanceLayer.layerName, "ParamChecker" ) )
			{
				EnabledInstanceLayers.push_back( InstanceLayer.layerName );
			}
			if( strstr( InstanceLayer.layerName, "ShaderChecker" ) )
			{
				EnabledInstanceLayers.push_back( InstanceLayer.layerName );
			}
			if( strstr( InstanceLayer.layerName, "Threading" ) )
			{
				EnabledInstanceLayers.push_back( InstanceLayer.layerName );
			}
			if( strstr( InstanceLayer.layerName, "DrawState" ) )
			{
				EnabledInstanceLayers.push_back( InstanceLayer.layerName );
			}
		}
#endif
	}
	else
	{
		// TODO: Error message.
		BcBreakpoint;
		return;
	}

	// Grab extensions.
	bool DebugEnabled = false;
	uint32_t InstanceExtensionCount = 0;
	if( ( RetVal = vkEnumerateInstanceExtensionProperties( nullptr, &InstanceExtensionCount, nullptr ) ) == VK_SUCCESS )
	{
		InstanceExtensions_.resize( InstanceExtensionCount );
		RetVal = vkEnumerateInstanceExtensionProperties( nullptr, &InstanceExtensionCount, InstanceExtensions_.data() );
		BcAssert( !RetVal );

		for( const auto& InstanceExtension : InstanceExtensions_ )
		{
			if( strstr( InstanceExtension.extName, "VK_WSI_swapchain" ) )
			{
				EnabledInstanceExtensions.push_back( InstanceExtension.extName );
			}
			else if( strstr( InstanceExtension.extName, "DEBUG_REPORT" ) )
			{
				EnabledInstanceExtensions.push_back( InstanceExtension.extName );
				DebugEnabled = true;
			}
		}
	}
	else
	{
		// TODO: Error message.
		BcBreakpoint;
		return;
	}

	// Setup application.
	VkApplicationInfo AppInfo = {};
	AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo.pNext = nullptr;
	AppInfo.pAppName = "Psybrus";
	AppInfo.appVersion = 0;
	AppInfo.pEngineName = "Psybrus";
	AppInfo.engineVersion = 0;
	AppInfo.apiVersion = VK_API_VERSION;

	VkInstanceCreateInfo InstanceCreateInfo = {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pNext = nullptr;
	InstanceCreateInfo.pAppInfo = &AppInfo;
	InstanceCreateInfo.pAllocCb = nullptr;
	InstanceCreateInfo.layerCount = EnabledInstanceLayers.size();
	InstanceCreateInfo.ppEnabledLayerNames = EnabledInstanceLayers.size() > 0 ? EnabledInstanceLayers.data() : nullptr;
	InstanceCreateInfo.extensionCount = EnabledInstanceExtensions.size();
	InstanceCreateInfo.ppEnabledExtensionNames = EnabledInstanceExtensions.data();

	// Create instance.
	if( ( RetVal = vkCreateInstance( &InstanceCreateInfo, &Instance_ ) ) == VK_SUCCESS )
	{
		// Enumerate physical devices.
		uint32_t GPUCount = 256;
		if( ( RetVal = vkEnumeratePhysicalDevices( Instance_, &GPUCount, nullptr ) ) == VK_SUCCESS )
		{
			BcAssert( GPUCount > 0 );
			PhysicalDevices_.resize( GPUCount );
			RetVal = vkEnumeratePhysicalDevices( Instance_, &GPUCount, PhysicalDevices_.data() );
			BcAssert( !RetVal );
		}
		else
		{
			// TODO: Error message.
			BcBreakpoint;
			return;
		}

		// Get device layers.
		uint32_t DeviceLayerCount = 0;
		if( ( RetVal = vkEnumerateDeviceLayerProperties( PhysicalDevices_[ 0 ], &DeviceLayerCount, nullptr ) ) == VK_SUCCESS )
		{
			DeviceLayers_.resize( DeviceLayerCount );
			RetVal = vkEnumerateDeviceLayerProperties( PhysicalDevices_[ 0 ], &DeviceLayerCount, DeviceLayers_.data() );
			BcAssert( !RetVal );
#if 1
			for( const auto& DeviceLayer : DeviceLayers_ )
			{
				if( strstr( DeviceLayer.layerName, "Threading" ) )
				{
					EnabledDeviceLayers.push_back( DeviceLayer.layerName );
				}
				if( strstr( DeviceLayer.layerName, "MemTracker" ) )
				{
					EnabledDeviceLayers.push_back( DeviceLayer.layerName );
				}
				if( strstr( DeviceLayer.layerName, "ObjectTracker" ) )
				{
					EnabledDeviceLayers.push_back( DeviceLayer.layerName );
				}
				if( strstr( DeviceLayer.layerName, "ParamChecker" ) )
				{
					EnabledDeviceLayers.push_back( DeviceLayer.layerName );
				}
				if( strstr( DeviceLayer.layerName, "ShaderChecker" ) )
				{
					EnabledDeviceLayers.push_back( DeviceLayer.layerName );
				}
				if( strstr( DeviceLayer.layerName, "DrawState" ) )
				{
					EnabledDeviceLayers.push_back( DeviceLayer.layerName );
				}
			}
#endif
		}
		else
		{
			// TODO: Error message.
			BcBreakpoint;
			return;
		}

		// Get device extensions.
		uint32_t DeviceExtensionCount = 0;
		if( ( RetVal = vkEnumerateDeviceExtensionProperties( PhysicalDevices_[ 0 ], nullptr, &DeviceExtensionCount, nullptr ) ) == VK_SUCCESS )
		{
			DeviceExtensions_.resize( DeviceExtensionCount );
			RetVal = vkEnumerateDeviceExtensionProperties( PhysicalDevices_[ 0 ], nullptr, &DeviceExtensionCount, DeviceExtensions_.data() );
			BcAssert( !RetVal );

			for( const auto& DeviceExtension : DeviceExtensions_ )
			{
				if( strstr( DeviceExtension.extName, "VK_EXT_KHR_device_swapchain" ) )
				{
					EnabledDeviceExtensions.push_back( DeviceExtension.extName );
				}
			}
		}
		else
		{
			// TODO: Error message.
			BcBreakpoint;
			return;
		}

		// Setup debug callback.
		if( DebugEnabled )
		{
			fpCreateMsgCallback_ = (PFN_vkDbgCreateMsgCallback) vkGetInstanceProcAddr( Instance_, "vkDbgCreateMsgCallback" );
			fpDestroyMsgCallback_ = (PFN_vkDbgDestroyMsgCallback) vkGetInstanceProcAddr( Instance_, "vkDbgDestroyMsgCallback" );
			fpBreakCallback_ = (PFN_vkDbgMsgCallback) vkGetInstanceProcAddr( Instance_, "vkDbgBreakCallback" );

			auto DebugFunc = [](
					VkFlags                             msgFlags,
					VkDbgObjectType                     objType,
					uint64_t                            srcObject,
					size_t                              location,
					int32_t                             msgCode,
					const char*                         pLayerPrefix,
					const char*                         pMsg,
					void*                               pUserData )->VkBool32
				{
					PSY_LOGSCOPEDCATEGORY( "Vulkan" );

					RsContextVK* Context = reinterpret_cast< RsContextVK* >( pUserData );
					std::unique_ptr< char[] > Message( new char[ strlen( pMsg ) + 100 ] );
					BcAssert( Message.get() );

					if( msgFlags & VK_DBG_REPORT_ERROR_BIT )
					{
						sprintf( Message.get(), "ERROR: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg );
					} 
					else if( msgFlags & VK_DBG_REPORT_WARN_BIT )
					{
						// We know that we're submitting queues without fences, ignore this warning
						if( strstr(pMsg, "vkQueueSubmit parameter, VkFence fence, is null pointer") )
						{
							return 1;
						}
						sprintf( Message.get(), "WARNING: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg );
					}
					else if( msgFlags & VK_DBG_REPORT_INFO_BIT )
					{
						sprintf( Message.get(), "INFO: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg );
					} 
					else if( msgFlags & VK_DBG_REPORT_PERF_WARN_BIT )
					{
						sprintf( Message.get(), "PERF: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg );
					} 
					else if( msgFlags & VK_DBG_REPORT_DEBUG_BIT )
					{
						sprintf( Message.get(), "DEBUG: [%s] Code %d : %s", pLayerPrefix, msgCode, pMsg );
					} 
					else
					{
						return 1;
					}

					PSY_LOG( Message.get() );
					return 1;
				};
			RetVal = fpCreateMsgCallback_(
				Instance_,
				VK_DBG_REPORT_ERROR_BIT | VK_DBG_REPORT_WARN_BIT | VK_DBG_REPORT_PERF_WARN_BIT | VK_DBG_REPORT_DEBUG_BIT,
				DebugFunc, this,
				&DebugCallback_ );
			BcAssert( !RetVal );
		}

		// Create first device.
		VkDeviceQueueCreateInfo DeviceQueueCreateInfo;
		DeviceQueueCreateInfo.queueFamilyIndex = 0;
		DeviceQueueCreateInfo.queueCount = 1;

		VkDeviceCreateInfo DeviceCreateInfo = {};
		DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DeviceCreateInfo.pNext = nullptr;
		DeviceCreateInfo.queueRecordCount = 1;
		DeviceCreateInfo.pRequestedQueues = &DeviceQueueCreateInfo;
		DeviceCreateInfo.layerCount = EnabledDeviceLayers.size();
		DeviceCreateInfo.ppEnabledLayerNames = EnabledDeviceLayers.size() > 0 ? EnabledDeviceLayers.data() : nullptr;
		DeviceCreateInfo.extensionCount = EnabledDeviceExtensions.size();
		DeviceCreateInfo.ppEnabledExtensionNames = EnabledDeviceExtensions.data();
		DeviceCreateInfo.pEnabledFeatures = nullptr;

		if( ( RetVal = vkCreateDevice( PhysicalDevices_[ 0 ], &DeviceCreateInfo, &Device_ ) ) == VK_SUCCESS )
		{
			// Get fps.
			GET_INSTANCE_PROC_ADDR( Instance_, GetPhysicalDeviceSurfaceSupportKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetSurfacePropertiesKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetSurfaceFormatsKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetSurfacePresentModesKHR );
			GET_DEVICE_PROC_ADDR( Device_, CreateSwapchainKHR );
			GET_DEVICE_PROC_ADDR( Device_, DestroySwapchainKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetSwapchainImagesKHR );
			GET_DEVICE_PROC_ADDR( Device_, AcquireNextImageKHR );
			GET_DEVICE_PROC_ADDR( Device_, QueuePresentKHR );

			//
			RetVal = vkGetPhysicalDeviceProperties( PhysicalDevices_[ 0 ], &DeviceProps_ );
			BcAssert( !RetVal );
			uint32_t DeviceQueueCount = 0;
			RetVal = vkGetPhysicalDeviceQueueFamilyProperties( PhysicalDevices_[ 0 ], &DeviceQueueCount, nullptr );
			BcAssert( !RetVal );
			DeviceQueueProps_.resize( DeviceQueueCount );
			RetVal = vkGetPhysicalDeviceQueueFamilyProperties( PhysicalDevices_[ 0 ], &DeviceQueueCount, DeviceQueueProps_.data() );
			BcAssert( !RetVal );
		}
		else
		{
			// TODO: Error message.
			BcBreakpoint;
			return;
		}

		// Setup windowing.
		WindowSurfaceDesc_.sType = VK_STRUCTURE_TYPE_SURFACE_DESCRIPTION_WINDOW_KHR;
		WindowSurfaceDesc_.pNext = nullptr;
#ifdef PLATFORM_WINDOWS
		WindowSurfaceDesc_.platform = VK_PLATFORM_WIN32_KHR;
		WindowSurfaceDesc_.pPlatformHandle = ::GetModuleHandle( nullptr );
		WindowSurfaceDesc_.pPlatformWindow = pClient_->getWindowHandle();
#else  // PLATFORM_WINDOWS
		BcBreakpoint;
#endif // PLATFORM_WINDOWS

		// Find queue that can present.
		// TODO: Find queue that supports graphics & present.
		uint32_t FoundGraphicsQueue = UINT32_MAX;
		for( uint32_t Idx = 0; Idx < DeviceQueueProps_.size(); ++Idx )
		{
			VkBool32 SupportsPresent = 0;
			if( ( RetVal = fpGetPhysicalDeviceSurfaceSupportKHR_( PhysicalDevices_[ 0 ], Idx, (VkSurfaceDescriptionKHR*)&WindowSurfaceDesc_, &SupportsPresent ) ) == VK_SUCCESS )
			{
				if( SupportsPresent )
				{
					FoundGraphicsQueue = Idx;
					break;
				}
			}
			else
			{
				// TODO: Error message.
				BcBreakpoint;
				return;
			}
		}

		// Get queue.
		RetVal = vkGetDeviceQueue( Device_, FoundGraphicsQueue, 0, &GraphicsQueue_ );
		BcAssert( !RetVal );

		

		// Get formats.
		uint32_t FormatCount = 0;
		if( ( RetVal = fpGetSurfaceFormatsKHR_( Device_, (VkSurfaceDescriptionKHR*)&WindowSurfaceDesc_, &FormatCount, nullptr ) ) == VK_SUCCESS )
		{
			SurfaceFormats_.resize( FormatCount );
			RetVal = fpGetSurfaceFormatsKHR_( Device_, (VkSurfaceDescriptionKHR*)&WindowSurfaceDesc_, &FormatCount, SurfaceFormats_.data() );
			BcAssert( !RetVal );
		}
		else
		{
			// TODO: Error message.
			return; 
		}

		// Check format.
		if (SurfaceFormats_.size() == 1 && SurfaceFormats_[ 0 ].format == VK_FORMAT_UNDEFINED)
		{
			SurfaceFormats_[ 0 ].format = VK_FORMAT_B8G8R8A8_UNORM;
		}

		// Create allocator.
		Allocator_.reset( new RsAllocatorVK( PhysicalDevices_[ 0 ], Device_ ) );

		// Command pool & buffer.
		CommandPoolCreateInfo_.sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO;
		CommandPoolCreateInfo_.pNext = nullptr;
		CommandPoolCreateInfo_.queueFamilyIndex = FoundGraphicsQueue;
		CommandPoolCreateInfo_.flags = 0;

		RetVal = vkCreateCommandPool( Device_, &CommandPoolCreateInfo_, &CommandPool_ );
		BcAssert( !RetVal );

		CommandBufferCreateInfo_.sType = VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO;
		CommandBufferCreateInfo_.pNext = nullptr;
		CommandBufferCreateInfo_.cmdPool = CommandPool_;
		CommandBufferCreateInfo_.level = VK_CMD_BUFFER_LEVEL_PRIMARY;
		CommandBufferCreateInfo_.flags = 0;

		RetVal = vkCreateCommandBuffer( Device_, &CommandBufferCreateInfo_, &CommandBuffer_ );
		BcAssert( !RetVal );

		// Command buffer setup.
		VkCmdBufferBeginInfo CommandBufferInfo = {};
		CommandBufferInfo.sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO;
		CommandBufferInfo.pNext = nullptr;
		CommandBufferInfo.flags = VK_CMD_BUFFER_OPTIMIZE_SMALL_BATCH_BIT | VK_CMD_BUFFER_OPTIMIZE_ONE_TIME_SUBMIT_BIT;

		// Begin command buffer.
		auto RetVal = vkBeginCommandBuffer( CommandBuffer_, &CommandBufferInfo );
		BcAssert( !RetVal );


		// Swap chain.
		// TODO: Get present mode info and try to use mailbox, then try immediate, then try FIFO.
		// TODO: Get present mode info to determine correct number of iamges. Use 2 for now.
		// TODO: Get present mode info to get pre transform.
		VkPresentModeKHR SwapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		uint32_t DesiredNumberOfSwapChainImages = 2;
		VkSurfaceTransformKHR PreTransform = VK_SURFACE_TRANSFORM_NONE_KHR;


		SwapChainCreateInfo_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		SwapChainCreateInfo_.pNext = nullptr;
		SwapChainCreateInfo_.pSurfaceDescription = (const VkSurfaceDescriptionKHR *)&WindowSurfaceDesc_;
		SwapChainCreateInfo_.minImageCount = DesiredNumberOfSwapChainImages;
		SwapChainCreateInfo_.imageFormat = SurfaceFormats_[ 0 ].format;
		SwapChainCreateInfo_.imageColorSpace = SurfaceFormats_[ 0 ].colorSpace;
		SwapChainCreateInfo_.imageExtent.width = pClient_->getWidth();
		SwapChainCreateInfo_.imageExtent.height = pClient_->getHeight();
		SwapChainCreateInfo_.imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		SwapChainCreateInfo_.preTransform = PreTransform;
		SwapChainCreateInfo_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SwapChainCreateInfo_.queueFamilyCount = 0;
		SwapChainCreateInfo_.pQueueFamilyIndices = nullptr;
		SwapChainCreateInfo_.imageArraySize = 1;
		SwapChainCreateInfo_.presentMode = SwapChainPresentMode;
		SwapChainCreateInfo_.oldSwapchain.handle = 0;
		SwapChainCreateInfo_.clipped = true;

		RetVal = fpCreateSwapchainKHR_( Device_, &SwapChainCreateInfo_, &SwapChain_ );
		BcAssert( !RetVal );

		uint32_t SwapChainImagesSize = 0;
		RetVal = fpGetSwapchainImagesKHR_( Device_, SwapChain_, &SwapChainImagesSize, nullptr );
		BcAssert( !RetVal );

		size_t SwapChainImageCount = SwapChainImagesSize;
		SwapChainImages_.resize( SwapChainImageCount );
		RetVal = fpGetSwapchainImagesKHR_( Device_, SwapChain_, &SwapChainImagesSize, SwapChainImages_.data() );
		BcAssert( !RetVal );

		SwapChainTextures_.resize( SwapChainImageCount );
		for( BcU32 Idx = 0; Idx < SwapChainTextures_.size(); ++Idx )
		{
			auto& SwapChainTexture = SwapChainTextures_[ Idx ];

			// Setup texture descriptor.
			RsTextureDesc Desc(
				RsTextureType::TEX2D,
				RsResourceCreationFlags::STATIC,
				RsResourceBindFlags::RENDER_TARGET,
				RsUtilsVK::GetTextureFormat(  SurfaceFormats_[ 0 ].format ),
				1,
				pClient_->getWidth(),
				pClient_->getHeight() );

			// Create texture.
			SwapChainTexture = new RsTexture( this, Desc );

			// Manually create the swapchain texture, it's a special case as it takes an image already.
			RsTextureVK* TextureVK = new RsTextureVK( SwapChainTexture, Device_, Allocator_.get(), SwapChainImages_[ Idx ] );
			TextureVK->setImageLayout( CommandBuffer_, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );
		}

		// Depth buffer.
		{
			// Setup texture descriptor.
			RsTextureDesc Desc(
				RsTextureType::TEX2D,
				RsResourceCreationFlags::STATIC,
				RsResourceBindFlags::DEPTH_STENCIL,
				RsTextureFormat::D24S8,
				1,
				pClient_->getWidth(),
				pClient_->getHeight() );

			// Create texture.
			DepthStencilTexture_ = new RsTexture( this, Desc );
			createTexture( DepthStencilTexture_ );
		}

		// Frame buffers.
		FrameBuffers_.resize( SwapChainImageCount );
		for( BcU32 Idx = 0; Idx < FrameBuffers_.size(); ++Idx )
		{
			auto& FrameBuffer = FrameBuffers_[ Idx ];

			// Setup frame buffer descriptor.
			RsFrameBufferDesc Desc( 1 );
			Desc.setRenderTarget( 0, SwapChainTextures_[ Idx ] );
			Desc.setDepthStencilTarget( DepthStencilTexture_ );

			// Create frame buffer.
			FrameBuffer = new RsFrameBuffer( this, Desc );
			createFrameBuffer( FrameBuffer );
		}

		// Fill out features (temp work).
		Features_.MRT_ = true;
		Features_.DepthTextures_ = true;
		Features_.NPOTTextures_ = true;
		Features_.SeparateBlendState_ = true;
		Features_.AnisotropicFiltering_ = true;
		Features_.AntialiasedLines_ = true;
		Features_.Texture1D_ = true;
		Features_.Texture2D_ = true;
		Features_.Texture3D_ = true;
		Features_.TextureCube_ = true;

		// TODO: Check formats properly.
			Features_.TextureFormat_[ (int)RsTextureFormat::R8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8B8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R8G8B8A8 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::DXT1 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::DXT3 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::DXT5 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D16 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D24 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D32 ] = true;
			Features_.TextureFormat_[ (int)RsTextureFormat::D24S8 ] = true;

			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8B8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R8G8B8A8 ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R16FG16FB16FA16F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32F ] = true;
			Features_.RenderTargetFormat_[ (int)RsTextureFormat::R32FG32FB32FA32F ] = true;

			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D16 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D32 ] = true;
			Features_.DepthStencilTargetFormat_[ (int)RsTextureFormat::D24S8 ] = true;

	}
	else
	{
		// TODO: Error message.
		BcBreakpoint;
		return;
	}


}

//////////////////////////////////////////////////////////////////////////
// update
void RsContextVK::update()
{
}

//////////////////////////////////////////////////////////////////////////
// destroy
void RsContextVK::destroy()
{
	// Destroy framebuffers.
	for( auto FrameBuffer : FrameBuffers_ )
	{
		destroyFrameBuffer( FrameBuffer );
	}

	// Destroy textures.
	for( auto Texture : SwapChainTextures_ )
	{
		destroyTexture( Texture );
	}
	destroyTexture( DepthStencilTexture_ );

	// Destroy everything else.
	vkDestroyCommandBuffer( Device_, CommandBuffer_ );
	CommandBuffer_ = 0;

	vkDestroyCommandPool( Device_, CommandPool_ );
	CommandPool_ = 0;

	vkDestroyDevice( Device_ );
	Device_ = 0;

	vkDestroyInstance( Instance_ );
	Instance_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextVK::clear( 
	const RsFrameBuffer* FrameBuffer,
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( FrameBuffer );
	BcUnusedVar( Colour );
	BcUnusedVar( EnableClearColour );
	BcUnusedVar( EnableClearDepth );
	BcUnusedVar( EnableClearStencil );
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextVK::drawPrimitives( 
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding, 
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer, 
		const RsViewport* Viewport,
		const RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, 
		BcU32 VertexOffset, BcU32 NoofVertices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextVK::drawIndexedPrimitives( 
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding, 
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer,
		const RsViewport* Viewport,
		const RsScissorRect* ScissorRect,
		RsTopologyType TopologyType, 
		BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// copyTexture
void RsContextVK::copyTexture( RsTexture* SourceTexture, RsTexture* DestTexture )
{
}

//////////////////////////////////////////////////////////////////////////
// dispatchCompute
void RsContextVK::dispatchCompute( class RsProgramBinding* ProgramBinding, BcU32 XGroups, BcU32 YGroups, BcU32 ZGroups )
{
}

//////////////////////////////////////////////////////////////////////////
// createRenderState
bool RsContextVK::createRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( RenderState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyRenderState
bool RsContextVK::destroyRenderState(
	RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( RenderState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createSamplerState
bool RsContextVK::createSamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( SamplerState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextVK::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( SamplerState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createFrameBuffer
bool RsContextVK::createFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	
	RsFrameBufferVK* FrameBufferVK = new RsFrameBufferVK( FrameBuffer, Device_ );
	
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextVK::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RsFrameBufferVK* FrameBufferVK = FrameBuffer->getHandle< RsFrameBufferVK* >();
	BcAssert( FrameBufferVK );
	delete FrameBufferVK;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// createBuffer
bool RsContextVK::createBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextVK::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateBuffer
bool RsContextVK::updateBuffer( 
	class RsBuffer* Buffer,
	BcSize Offset,
	BcSize Size,
	RsResourceUpdateFlags Flags,
	RsBufferUpdateFunc UpdateFunc )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Buffer );
	BcUnusedVar( Offset );
	BcUnusedVar( Size );
	BcUnusedVar( Flags );
	BcUnusedVar( UpdateFunc );
	std::unique_ptr< BcU8[] > TempBuffer;
	TempBuffer.reset( new BcU8[ Buffer->getDesc().SizeBytes_ ] );
	RsBufferLock Lock = { TempBuffer.get() };
	UpdateFunc( Buffer, Lock );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createTexture
bool RsContextVK::createTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	
	const auto& Desc = Texture->getDesc();
	RsTextureVK* TextureVK = new RsTextureVK( Texture, Device_, Allocator_.get() );
	
	// Determine image aspect + layout.
	VkImageAspect Aspect = VK_IMAGE_ASPECT_COLOR;
	VkImageLayout Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	if( ( Desc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		Aspect = VK_IMAGE_ASPECT_COLOR;
		Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else if( ( Desc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		Aspect = VK_IMAGE_ASPECT_DEPTH;
		Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	else if( ( Desc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		Aspect = VK_IMAGE_ASPECT_COLOR;
		Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	// Set image layout.
	TextureVK->setImageLayout( CommandBuffer_, Aspect, Layout );

	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextVK::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );

	RsTextureVK* TextureVK = Texture->getHandle< RsTextureVK* >();
	BcAssert( TextureVK );
	delete TextureVK;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// updateTexture
bool RsContextVK::updateTexture( 
	class RsTexture* Texture,
	const struct RsTextureSlice& Slice,
	RsResourceUpdateFlags Flags,
	RsTextureUpdateFunc UpdateFunc )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Texture );
	BcUnusedVar( Slice );
	BcUnusedVar( Flags );
	BcUnusedVar( UpdateFunc );
	const auto& TextureDesc = Texture->getDesc();
	std::unique_ptr< BcU8[] > TextureData;
		BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
		BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
		BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );
		BcU32 DataSize = RsTextureFormatSize( 
			TextureDesc.Format_,
			Width,
			Height,
			Depth,
			1 );
	TextureData.reset( new BcU8[ DataSize ] );
	const auto BlockInfo = RsTextureBlockInfo( TextureDesc.Format_ );
	RsTextureLock Lock;
	Lock.Buffer_ = TextureData.get();
	Lock.Pitch_ = ( ( Width / BlockInfo.Width_ ) * BlockInfo.Bits_ ) / 8;
	Lock.SlicePitch_ = ( ( Width / BlockInfo.Width_ ) * ( Height / BlockInfo.Height_ ) * BlockInfo.Bits_ ) / 8;

	UpdateFunc( Texture, Lock );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createShader
bool RsContextVK::createShader(
	class RsShader* Shader )
{
	BcUnusedVar( Shader );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyShader
bool RsContextVK::destroyShader(
	class RsShader* Shader )
{
	BcUnusedVar( Shader );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgram
bool RsContextVK::createProgram(
	class RsProgram* Program )
{
	BcUnusedVar( Program );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextVK::destroyProgram(
	class RsProgram* Program )
{
	BcUnusedVar( Program );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgramBinding
bool RsContextVK::createProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgramBinding
bool RsContextVK::destroyProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createGeometryBinding
bool RsContextVK::createGeometryBinding( class RsGeometryBinding* GeometryBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyGeometryBinding
bool RsContextVK::destroyGeometryBinding( class RsGeometryBinding* GeometryBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createVertexDeclaration
bool RsContextVK::createVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyVertexDeclaration
bool RsContextVK::destroyVertexDeclaration(
	class RsVertexDeclaration* VertexDeclaration  )
{
	return true;
}

//////////////////////////////////////////////////////////////////////////
// flushState
//virtual
void RsContextVK::flushState()
{
}
