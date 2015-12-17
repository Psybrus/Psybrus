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
#include "System/Renderer/VK/RsProgramVK.h"
#include "System/Renderer/VK/RsProgramBindingVK.h"
#include "System/Renderer/VK/RsTextureVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsFrameBuffer.h"
#include "System/Renderer/RsGeometryBinding.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsProgramBinding.h"
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
	if( CodeType == RsShaderCodeType::SPIRV )
	{
		return true;
	}
	return false;
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
	return FrameBuffers_[ CurrentFrameBuffer_ ];
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
	VkCommandBufferBeginInfo CommandBufferInfo = {};
	CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferInfo.pNext = nullptr;
	CommandBufferInfo.flags = 0;

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextVK::endFrame()
{
	BcAssert( InsideBeginEndFrame_ );
	InsideBeginEndFrame_ = false;

	bindFrameBuffer( nullptr, nullptr, nullptr, 0, nullptr );

	// End command buffer.
	VK( vkEndCommandBuffer( getCommandBuffer() ) );

	// Create semaphore to wait for queue to complete.
	VkSemaphore PresentCompleteSemaphore;
	VkSemaphoreCreateInfo PresentCompleteSemaphoreCreateInfo = {};
	PresentCompleteSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	PresentCompleteSemaphoreCreateInfo.pNext = nullptr;
	PresentCompleteSemaphoreCreateInfo.flags = 0;

	VK( vkCreateSemaphore( Device_, &PresentCompleteSemaphoreCreateInfo, AllocationCallbacks_, &PresentCompleteSemaphore ) );

	// Get the index of the next available swapchain image.
	VK( fpAcquireNextImageKHR_( Device_, SwapChain_,
		UINT64_MAX,
		PresentCompleteSemaphore,
		nullptr,
		&CurrentFrameBuffer_ ) );

	VK( vkQueueWaitIdle( GraphicsQueue_ ) );

	// Submit queue.
	auto CommandBuffers = getCommandBuffer();
	VkSubmitInfo SubmitInfo = {};
	SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	SubmitInfo.pNext = nullptr;
	SubmitInfo.waitSemaphoreCount = 0;
	SubmitInfo.pWaitSemaphores = nullptr;
	SubmitInfo.commandBufferCount = 1;
	SubmitInfo.pCommandBuffers = &CommandBuffers;
	SubmitInfo.signalSemaphoreCount = 0;
	SubmitInfo.pSignalSemaphores = nullptr;
	VkFence NullFence = { VK_NULL_HANDLE };
	VK( vkQueueSubmit( GraphicsQueue_, 1, &SubmitInfo, NullFence ) );

	VkPresentInfoKHR PresentInfo = {};
	PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	PresentInfo.pNext = nullptr;
	PresentInfo.swapchainCount = 1;
	PresentInfo.pSwapchains = &SwapChain_;
	PresentInfo.pImageIndices = &CurrentFrameBuffer_;

	VK( fpQueuePresentKHR_( GraphicsQueue_, &PresentInfo ) );
	VK( vkQueueWaitIdle( GraphicsQueue_ ) );

	vkDestroySemaphore( Device_, PresentCompleteSemaphore, AllocationCallbacks_ );

	// Next command buffer.
	CurrentCommandBuffer_ = 1 - CurrentCommandBuffer_; 

	// Begin command buffer.
	VkCommandBufferBeginInfo CommandBufferInfo = {};
	CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	CommandBufferInfo.pNext = nullptr;
	CommandBufferInfo.flags = 0;
	VK( vkBeginCommandBuffer( getCommandBuffer(), &CommandBufferInfo ) );
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
	if( VK( vkEnumerateInstanceLayerProperties( &InstanceLayerCount, nullptr ) ) == VK_SUCCESS )
	{
		InstanceLayers_.resize( InstanceLayerCount );
		VK( vkEnumerateInstanceLayerProperties( &InstanceLayerCount, InstanceLayers_.data() ) );
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
	if( VK( vkEnumerateInstanceExtensionProperties( nullptr, &InstanceExtensionCount, nullptr ) ) == VK_SUCCESS )
	{
		InstanceExtensions_.resize( InstanceExtensionCount );
		VK( vkEnumerateInstanceExtensionProperties( nullptr, &InstanceExtensionCount, InstanceExtensions_.data() ) );

		for( const auto& InstanceExtension : InstanceExtensions_ )
		{
			if( strstr( InstanceExtension.extensionName, "VK_WSI_swapchain" ) )
			{
				EnabledInstanceExtensions.push_back( InstanceExtension.extensionName );
			}
			else if( strstr( InstanceExtension.extensionName, "DEBUG_REPORT" ) )
			{
				EnabledInstanceExtensions.push_back( InstanceExtension.extensionName );
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
	AppInfo.pApplicationName = "Psybrus";
	AppInfo.applicationVersion = 0;
	AppInfo.pEngineName = "Psybrus";
	AppInfo.engineVersion = 0;
	AppInfo.apiVersion = VK_API_VERSION;

	VkInstanceCreateInfo InstanceCreateInfo = {};
	InstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo.pNext = nullptr;
	InstanceCreateInfo.pApplicationInfo = &AppInfo;
	InstanceCreateInfo.enabledLayerNameCount = EnabledInstanceLayers.size();
	InstanceCreateInfo.ppEnabledLayerNames = EnabledInstanceLayers.size() > 0 ? EnabledInstanceLayers.data() : nullptr;
	InstanceCreateInfo.enabledExtensionNameCount = EnabledInstanceExtensions.size();
	InstanceCreateInfo.ppEnabledExtensionNames = EnabledInstanceExtensions.data();

	// Create instance.
	if( VK( vkCreateInstance( &InstanceCreateInfo, AllocationCallbacks_, &Instance_ ) ) == VK_SUCCESS )
	{
		// Enumerate physical devices.
		uint32_t GPUCount = 256;
		if( VK( vkEnumeratePhysicalDevices( Instance_, &GPUCount, nullptr ) ) == VK_SUCCESS )
		{
			BcAssert( GPUCount > 0 );
			PhysicalDevices_.resize( GPUCount );
			VK( vkEnumeratePhysicalDevices( Instance_, &GPUCount, PhysicalDevices_.data() ) );
		}
		else
		{
			// TODO: Error message.
			BcBreakpoint;
			return;
		}

		// Get device layers.
		uint32_t DeviceLayerCount = 0;
		if( VK( vkEnumerateDeviceLayerProperties( PhysicalDevices_[ 0 ], &DeviceLayerCount, nullptr ) ) == VK_SUCCESS )
		{
			DeviceLayers_.resize( DeviceLayerCount );
			VK( vkEnumerateDeviceLayerProperties( PhysicalDevices_[ 0 ], &DeviceLayerCount, DeviceLayers_.data() ) );
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
		if( VK( vkEnumerateDeviceExtensionProperties( PhysicalDevices_[ 0 ], nullptr, &DeviceExtensionCount, nullptr ) ) == VK_SUCCESS )
		{
			DeviceExtensions_.resize( DeviceExtensionCount );
			VK( vkEnumerateDeviceExtensionProperties( PhysicalDevices_[ 0 ], nullptr, &DeviceExtensionCount, DeviceExtensions_.data() ) );

			for( const auto& DeviceExtension : DeviceExtensions_ )
			{
				if( strstr( DeviceExtension.extensionName, "VK_EXT_KHR_device_swapchain" ) )
				{
					EnabledDeviceExtensions.push_back( DeviceExtension.extensionName );
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

					std::stringstream LogStream( Message.get() );
					std::string LogLine;
					while( std::getline( LogStream, LogLine, '\n' ) )
					{
						PSY_LOG( LogLine.c_str() );
					}
					return 1;
				};
			VK( fpCreateMsgCallback_(
				Instance_,
				VK_DBG_REPORT_ERROR_BIT | VK_DBG_REPORT_WARN_BIT | VK_DBG_REPORT_PERF_WARN_BIT | VK_DBG_REPORT_DEBUG_BIT,
				DebugFunc, this,
				&DebugCallback_ ) );
		}

		// Create first device.
		VkDeviceQueueCreateInfo DeviceQueueCreateInfo;
		DeviceQueueCreateInfo.queueFamilyIndex = 0;
		DeviceQueueCreateInfo.queueCount = 1;

		VkDeviceCreateInfo DeviceCreateInfo = {};
		DeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DeviceCreateInfo.pNext = nullptr;
		DeviceCreateInfo.queueCreateInfoCount = 1;
		DeviceCreateInfo.pQueueCreateInfos = &DeviceQueueCreateInfo;
		DeviceCreateInfo.enabledLayerNameCount = EnabledDeviceLayers.size();
		DeviceCreateInfo.ppEnabledLayerNames = EnabledDeviceLayers.size() > 0 ? EnabledDeviceLayers.data() : nullptr;
		DeviceCreateInfo.enabledExtensionNameCount = EnabledDeviceExtensions.size();
		DeviceCreateInfo.ppEnabledExtensionNames = EnabledDeviceExtensions.data();
		DeviceCreateInfo.pEnabledFeatures = nullptr;

		if( VK( vkCreateDevice( PhysicalDevices_[ 0 ], &DeviceCreateInfo, AllocationCallbacks_, &Device_ ) ) == VK_SUCCESS )
		{
			// Get fps.
			GET_INSTANCE_PROC_ADDR( Instance_, GetPhysicalDeviceSurfaceSupportKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetPhysicalDeviceSurfaceCapabilitiesKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetPhysicalDeviceSurfaceFormatsKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetPhysicalDeviceSurfacePresentModesKHR );
			GET_DEVICE_PROC_ADDR( Device_, CreateSwapchainKHR );
			GET_DEVICE_PROC_ADDR( Device_, DestroySwapchainKHR );
			GET_DEVICE_PROC_ADDR( Device_, GetSwapchainImagesKHR );
			GET_DEVICE_PROC_ADDR( Device_, AcquireNextImageKHR );
			GET_DEVICE_PROC_ADDR( Device_, QueuePresentKHR );

			//
			vkGetPhysicalDeviceProperties( PhysicalDevices_[ 0 ], &DeviceProps_ );
			uint32_t DeviceQueueCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties( PhysicalDevices_[ 0 ], &DeviceQueueCount, nullptr );
			DeviceQueueProps_.resize( DeviceQueueCount );
			vkGetPhysicalDeviceQueueFamilyProperties( PhysicalDevices_[ 0 ], &DeviceQueueCount, DeviceQueueProps_.data() );
		}
		else
		{
			// TODO: Error message.
			BcBreakpoint;
			return;
		}

		// Setup windowing.
#if PLATFORM_WINDOWS
		VK( vkCreateWin32SurfaceKHR( Instance_, 
			::GetModuleHandle( nullptr ),
			(HWND)pClient_->getWindowHandle(),
			AllocationCallbacks_,
			&WindowSurface_ ) );
#endif

		// Find queue that can present.
		// TODO: Find queue that supports graphics & present.
		uint32_t FoundGraphicsQueue = UINT32_MAX;
		for( uint32_t Idx = 0; Idx < DeviceQueueProps_.size(); ++Idx )
		{
			VkBool32 SupportsPresent = 0;
			if( ( RetVal = fpGetPhysicalDeviceSurfaceSupportKHR_( PhysicalDevices_[ 0 ], Idx, WindowSurface_, &SupportsPresent ) ) == VK_SUCCESS )
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
		vkGetDeviceQueue( Device_, FoundGraphicsQueue, 0, &GraphicsQueue_ );
		
		// Get formats.
		uint32_t FormatCount = 0;
		if( VK( fpGetPhysicalDeviceSurfaceFormatsKHR_( PhysicalDevices_[ 0 ], WindowSurface_, &FormatCount, nullptr ) ) == VK_SUCCESS )
		{
			SurfaceFormats_.resize( FormatCount );
			VK( fpGetPhysicalDeviceSurfaceFormatsKHR_( PhysicalDevices_[ 0 ], WindowSurface_, &FormatCount, SurfaceFormats_.data() ) );
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
		CommandPoolCreateInfo_.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		CommandPoolCreateInfo_.pNext = nullptr;
		CommandPoolCreateInfo_.queueFamilyIndex = FoundGraphicsQueue;
		CommandPoolCreateInfo_.flags = 0;

		VK( vkCreateCommandPool( Device_, &CommandPoolCreateInfo_, AllocationCallbacks_, &CommandPool_ ) );

		CommandBufferAllocateInfo_.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		CommandBufferAllocateInfo_.pNext = nullptr;
		CommandBufferAllocateInfo_.commandPool = CommandPool_;
		CommandBufferAllocateInfo_.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		CommandBufferAllocateInfo_.bufferCount = 2;

		VK( vkAllocateCommandBuffers( Device_, &CommandBufferAllocateInfo_, &CommandBuffers_[ 0 ] ) );

		// Command buffer setup.
		VkCommandBufferBeginInfo CommandBufferInfo = {};
		CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		CommandBufferInfo.pNext = nullptr;
		CommandBufferInfo.flags = 0;

		// Begin command buffer.
		VK( vkBeginCommandBuffer( getCommandBuffer(), &CommandBufferInfo ) );

		// Swap chain.
		// TODO: Get present mode info and try to use mailbox, then try immediate, then try FIFO.
		// TODO: Get present mode info to determine correct number of iamges. Use 2 for now.
		// TODO: Get present mode info to get pre transform.
		VkPresentModeKHR SwapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		uint32_t DesiredNumberOfSwapChainImages = 2;
		VkSurfaceTransformFlagBitsKHR PreTransform = VK_SURFACE_TRANSFORM_NONE_BIT_KHR;

		SwapChainCreateInfo_.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		SwapChainCreateInfo_.pNext = nullptr;
		SwapChainCreateInfo_.surface = WindowSurface_;
		SwapChainCreateInfo_.minImageCount = DesiredNumberOfSwapChainImages;
		SwapChainCreateInfo_.imageFormat = SurfaceFormats_[ 0 ].format;
		SwapChainCreateInfo_.imageColorSpace = SurfaceFormats_[ 0 ].colorSpace;
		SwapChainCreateInfo_.imageExtent.width = pClient_->getWidth();
		SwapChainCreateInfo_.imageExtent.height = pClient_->getHeight();
		SwapChainCreateInfo_.preTransform = PreTransform;
		SwapChainCreateInfo_.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		SwapChainCreateInfo_.queueFamilyIndexCount = 0;
		SwapChainCreateInfo_.pQueueFamilyIndices = nullptr;
		SwapChainCreateInfo_.imageArrayLayers = 1;
		SwapChainCreateInfo_.presentMode = SwapChainPresentMode;
		SwapChainCreateInfo_.oldSwapchain = 0;
		SwapChainCreateInfo_.clipped = true;

		VK( fpCreateSwapchainKHR_( Device_, &SwapChainCreateInfo_, nullptr, &SwapChain_ ) );

		uint32_t SwapChainImagesSize = 0;
		VK( fpGetSwapchainImagesKHR_( Device_, SwapChain_, &SwapChainImagesSize, nullptr ) );

		size_t SwapChainImageCount = SwapChainImagesSize;
		SwapChainImages_.resize( SwapChainImageCount );
		VK( fpGetSwapchainImagesKHR_( Device_, SwapChain_, &SwapChainImagesSize, SwapChainImages_.data() ) );

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
			TextureVK->setImageLayout( getCommandBuffer(), VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL );
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

	createDescriptorLayouts();
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
	vkFreeCommandBuffers( Device_, CommandPool_, 2, CommandBuffers_.data() );
	vkDestroyCommandPool( Device_, CommandPool_, AllocationCallbacks_ );
	CommandPool_ = 0;

	vkDestroyDevice( Device_, AllocationCallbacks_ );
	Device_ = 0;

	vkDestroyInstance( Instance_, AllocationCallbacks_ );
	Instance_ = 0;
}

//////////////////////////////////////////////////////////////////////////
// createDescriptorLayouts
void RsContextVK::createDescriptorLayouts()
{
	VkResult RetVal = VK_SUCCESS;

	VkDescriptorSetLayoutBinding BaseLayoutBindings[4];
	BaseLayoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	BaseLayoutBindings[0].descriptorCount = 1;
	BaseLayoutBindings[0].stageFlags = 0;
	BaseLayoutBindings[0].pImmutableSamplers = nullptr;

	BaseLayoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	BaseLayoutBindings[1].descriptorCount = 1;
	BaseLayoutBindings[1].stageFlags = 0;
	BaseLayoutBindings[1].pImmutableSamplers = nullptr;

	BaseLayoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	BaseLayoutBindings[2].descriptorCount = 1;
	BaseLayoutBindings[2].stageFlags = 0;
	BaseLayoutBindings[2].pImmutableSamplers = nullptr;

	BaseLayoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	BaseLayoutBindings[3].descriptorCount = 1;
	BaseLayoutBindings[3].stageFlags = 0;
	BaseLayoutBindings[3].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutBinding LayoutBindings[16*4];

	const VkShaderStageFlagBits StageBits[] = 
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT
	};

	BcU32 LayoutBindingIdx = 0;
	for( size_t Idx = 0; Idx < 16; ++Idx )
	{
		LayoutBindings[ LayoutBindingIdx ] = BaseLayoutBindings[0];
		LayoutBindings[ LayoutBindingIdx ].stageFlags = VK_SHADER_STAGE_ALL;
		++LayoutBindingIdx;
	}
#if 0
	for( size_t Idx = 0; Idx < 16; ++Idx )
	{
		LayoutBindings[ LayoutBindingIdx ] = BaseLayoutBindings[1];
		LayoutBindings[ LayoutBindingIdx ].stageFlags = VK_SHADER_STAGE_ALL;
		++LayoutBindingIdx;
	}
	for( size_t Idx = 0; Idx < 16; ++Idx )
	{
		LayoutBindings[ LayoutBindingIdx ] = BaseLayoutBindings[2];
		LayoutBindings[ LayoutBindingIdx ].stageFlags = VK_SHADER_STAGE_ALL;
		++LayoutBindingIdx;
	}
	for( size_t Idx = 0; Idx < 16; ++Idx )
	{
		LayoutBindings[ LayoutBindingIdx ] = BaseLayoutBindings[3];
		LayoutBindings[ LayoutBindingIdx ].stageFlags = VK_SHADER_STAGE_ALL;
		++LayoutBindingIdx;
	}
#endif
	VkDescriptorSetLayoutCreateInfo DescriptorLayout;
	DescriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	DescriptorLayout.pNext = NULL;
	DescriptorLayout.bindingCount = LayoutBindingIdx;
	DescriptorLayout.pBinding = LayoutBindings;

	VK( vkCreateDescriptorSetLayout( Device_, &DescriptorLayout, AllocationCallbacks_, &GraphicsDescriptorSetLayout_ ) );

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo;
	PipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	PipelineLayoutCreateInfo.pNext = nullptr;
	PipelineLayoutCreateInfo.setLayoutCount = 1;
	PipelineLayoutCreateInfo.pSetLayouts = &GraphicsDescriptorSetLayout_;
	PipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	PipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
	VK( vkCreatePipelineLayout( Device_, &PipelineLayoutCreateInfo, AllocationCallbacks_, &GraphicsPipelineLayout_ ) );
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

	if( FrameBuffer == nullptr )
	{
		FrameBuffer = getBackBuffer();
	}
	
	const auto& Desc = FrameBuffer->getDesc();

	if( EnableClearColour && EnableClearDepth && EnableClearStencil )
	{
		BcU32 NoofClearValues = 0;
		std::array< VkClearValue, 8 > ClearValues;

		for( size_t Idx = 0; Idx < Desc.RenderTargets_.size(); ++Idx )
		{
			ClearValues[ NoofClearValues ].color.float32[0] = Colour.r();
			ClearValues[ NoofClearValues ].color.float32[1] = Colour.g();
			ClearValues[ NoofClearValues ].color.float32[2] = Colour.b();
			ClearValues[ NoofClearValues ].color.float32[3] = Colour.a();
			++NoofClearValues;
		}

		if( Desc.DepthStencilTarget_ )
		{
			ClearValues[ NoofClearValues ].depthStencil.depth = 1.0f;
			ClearValues[ NoofClearValues ].depthStencil.stencil = 0;
			++NoofClearValues;
		}

		bindFrameBuffer( FrameBuffer, nullptr, nullptr, NoofClearValues, ClearValues.data() );
	}
	else
	{
		BcAssertMsg( BcFalse, "Code path to clear attachments needs retesting." ); 
		bindFrameBuffer( FrameBuffer, nullptr, nullptr, 0, nullptr );
#if 0
		if( EnableClearColour )
		{
			VkClearColorValue ClearColour;
			ClearColour.float32[ 0 ] = Colour.r();
			ClearColour.float32[ 1 ] = Colour.g();
			ClearColour.float32[ 2 ] = Colour.b();
			ClearColour.float32[ 3 ] = Colour.a();
			for( size_t Idx = 0; Idx < Desc.RenderTargets_.size(); ++Idx )
			{
				auto TextureVK = Desc.RenderTargets_[ Idx ]->getHandle< RsTextureVK* >();
				VkRect3D ClearRange;
				ClearRange.offset.x = 0;
				ClearRange.offset.y = 0;
				ClearRange.offset.z = 0;
				ClearRange.extent.width = Desc.RenderTargets_[ Idx ]->getDesc().Width_;
				ClearRange.extent.height = Desc.RenderTargets_[ Idx ]->getDesc().Height_;
				ClearRange.extent.depth = Desc.RenderTargets_[ Idx ]->getDesc().Depth_;
				vkCmdClearColorAttachment( getCommandBuffer(), Idx, TextureVK->getImageLayout(), &ClearColour, 1, &ClearRange );
			}
		}

		if( EnableClearDepth || EnableClearStencil )
		{
			VkImageAspectFlags AspectFlags = 
				EnableClearDepth ? VK_IMAGE_ASPECT_DEPTH_BIT : 0 |
				EnableClearStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0;
			VkClearDepthStencilValue ClearDepthStencil;
			ClearDepthStencil.depth = 1.0f;
			ClearDepthStencil.stencil = 0;
			if( Desc.DepthStencilTarget_ != nullptr )
			{
				auto TextureVK = Desc.DepthStencilTarget_->getHandle< RsTextureVK* >();
				VkRect3D ClearRange;
				ClearRange.offset.x = 0;
				ClearRange.offset.y = 0;
				ClearRange.offset.z = 0;
				ClearRange.extent.width = Desc.DepthStencilTarget_->getDesc().Width_;
				ClearRange.extent.height = Desc.DepthStencilTarget_->getDesc().Height_;
				ClearRange.extent.depth = Desc.DepthStencilTarget_->getDesc().Depth_;
				vkCmdClearDepthStencilAttachment( getCommandBuffer(), AspectFlags, TextureVK->getImageLayout(), &ClearDepthStencil, 1, &ClearRange );
			}
		}
#endif
	}
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

	// Null signifies backbuffer.
	if( FrameBuffer == nullptr )
	{
		FrameBuffer = getBackBuffer();
	}

	bindFrameBuffer( FrameBuffer, Viewport, ScissorRect, 0, nullptr );
	bindGraphicsPSO( TopologyType, GeometryBinding, ProgramBinding, RenderState, FrameBuffer );
	vkCmdDraw( getCommandBuffer(), NoofVertices, 1, VertexOffset, 0 );
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

	// Null signifies backbuffer.
	if( FrameBuffer == nullptr )
	{
		FrameBuffer = getBackBuffer();
	}

	bindFrameBuffer( FrameBuffer, Viewport, ScissorRect, 0, nullptr );
	bindGraphicsPSO( TopologyType, GeometryBinding, ProgramBinding, RenderState, FrameBuffer );
	vkCmdDrawIndexed( getCommandBuffer(), NoofIndices, 1, IndexOffset, VertexOffset, 0 );
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
// bindFrameBuffer
void RsContextVK::bindFrameBuffer( 
	const RsFrameBuffer* FrameBuffer, 
	const RsViewport* Viewport, const RsScissorRect* ScissorRect, 
	BcU32 NoofClearValues, const VkClearValue* ClearValues )
{
	if( FrameBuffer != BoundFrameBuffer_ || NoofClearValues > 0 )
	{
		// End pass from old frame buffer.
		if( BoundFrameBuffer_ != nullptr )
		{
			vkCmdEndRenderPass( getCommandBuffer() );
		}

		// Assign new bound framebuffer.
		BoundFrameBuffer_ = FrameBuffer;

		// If we have bound, begin new pass.
		if( BoundFrameBuffer_ )
		{

			// Set render pass for frame buffer.
			RsFrameBuffer* FrameBuffer = FrameBuffers_[ CurrentFrameBuffer_ ];
			RsFrameBufferVK* FrameBufferVK = FrameBuffer->getHandle< RsFrameBufferVK* >();
			BoundRenderPass_ = NoofClearValues > 0 ? FrameBufferVK->getClearRenderPass() : FrameBufferVK->getLoadRenderPass();

			VkRenderPassBeginInfo BeginInfo = {};
			BeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			BeginInfo.pNext = nullptr;
			BeginInfo.renderPass = BoundRenderPass_;
			BeginInfo.framebuffer = NoofClearValues > 0 ? FrameBufferVK->getClearFrameBuffer() : FrameBufferVK->getLoadFrameBuffer();
			BeginInfo.renderArea.offset.x = 0;
			BeginInfo.renderArea.offset.y = 0;
			BeginInfo.renderArea.extent.width = FrameBuffer->getDesc().RenderTargets_[ 0 ]->getDesc().Width_;
			BeginInfo.renderArea.extent.height = FrameBuffer->getDesc().RenderTargets_[ 0 ]->getDesc().Height_;
			BeginInfo.clearValueCount = NoofClearValues;
			BeginInfo.pClearValues = ClearValues;

			// Begin render pass.
			vkCmdBeginRenderPass( getCommandBuffer(), &BeginInfo, VK_SUBPASS_CONTENTS_INLINE );
		}
	}

	if( FrameBuffer != nullptr )
	{
		// Bind viewport + scissor rect.
		// Determine frame buffer width + height.
		auto RT = FrameBuffer->getDesc().RenderTargets_[ 0 ];
		BcAssert( RT );
		auto FBWidth = RT->getDesc().Width_;
		auto FBHeight = RT->getDesc().Height_;

		RsViewport FullViewport( 0, 0, FBWidth, FBHeight );
		RsScissorRect FullScissorRect( 0, 0, FBWidth, FBHeight );

		BcAssert( FBWidth > 0 );
		BcAssert( FBHeight > 0 );

		// Setup viewport if null.
		Viewport = Viewport != nullptr ? Viewport : &FullViewport;

		// Setup scissor rect if null.
		ScissorRect = ScissorRect != nullptr ? ScissorRect : &FullScissorRect;

		//if( BoundViewport_ != *Viewport )
		{
			VkViewport ViewportVK;
			ViewportVK.x = Viewport->x();
			ViewportVK.y = Viewport->y();
			ViewportVK.width = Viewport->width();
			ViewportVK.height = Viewport->height();
			ViewportVK.minDepth = 0.0f;
			ViewportVK.maxDepth = 1.0f;
			vkCmdSetViewport( getCommandBuffer(), 1, &ViewportVK );
		}

		//if( BoundScissorRect_ != *ScissorRect )
		{
			VkRect2D ScissorRectVK;
			ScissorRectVK.offset.x = ScissorRect->X_;
			ScissorRectVK.offset.y = ScissorRect->Y_;
			ScissorRectVK.extent.width = ScissorRect->Width_;
			ScissorRectVK.extent.height = ScissorRect->Height_;
			vkCmdSetScissor( getCommandBuffer(), 1, &ScissorRectVK );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// bindGraphicsPSO
void RsContextVK::bindGraphicsPSO( 
		RsTopologyType TopologyType,
		const RsGeometryBinding* GeometryBinding, 
		const RsProgramBinding* ProgramBinding,
		const RsRenderState* RenderState,
		const RsFrameBuffer* FrameBuffer )
{
	VkPipeline Pipeline;
	auto Program = ProgramBinding->getProgram();
	auto ProgramBindingVK = ProgramBinding->getHandle< RsProgramBindingVK* >();
	auto VertexDeclaration = GeometryBinding->getDesc().VertexDeclaration_;
	auto PSOBinding = PSOBindingTuple( 
		TopologyType,
		GeometryBinding,
		Program, 
		RenderState,
		FrameBuffer,
		BoundRenderPass_ );
	auto FoundPSO = PSOCache_.find( PSOBinding );
	if( FoundPSO == PSOCache_.end() )
	{
		VkResult RetVal = VK_SUCCESS;
		VkGraphicsPipelineCreateInfo PipelineCreateInfo;
		VkPipelineCacheCreateInfo PipelineCache;
		VkPipelineVertexInputStateCreateInfo VI;
		VkPipelineInputAssemblyStateCreateInfo IA;
		VkPipelineRasterizationStateCreateInfo RS;
		VkPipelineColorBlendStateCreateInfo CB;
		VkPipelineDepthStencilStateCreateInfo DS;
		VkPipelineViewportStateCreateInfo VP;
		VkPipelineMultisampleStateCreateInfo MS;
		VkDynamicState DynamicStateEnables[ 10 ];
		VkPipelineDynamicStateCreateInfo DynamicState;
		
		memset( DynamicStateEnables, 0, sizeof( DynamicStateEnables ) );
		memset( &DynamicState, 0, sizeof( DynamicState ) );
		DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicState.pDynamicStates = DynamicStateEnables;

		memset( &PipelineCreateInfo, 0, sizeof( PipelineCreateInfo ) );
		PipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		PipelineCreateInfo.layout = GraphicsPipelineLayout_;

		const auto& ProgramVertexAttributeList = Program->getVertexAttributeList();
		const auto& VertexDeclarationDesc = VertexDeclaration->getDesc();
		const auto& PrimitiveVertexElementList = VertexDeclarationDesc.Elements_;
		const auto& GeometryBindingDesc = GeometryBinding->getDesc();
		std::array< VkVertexInputBindingDescription, 16 > BindingDescription;
		std::array< VkVertexInputAttributeDescription, 16 > AttributeDescription;

		memset( &VI, 0, sizeof( VI ) );
		VI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VI.vertexAttributeDescriptionCount = VertexDeclaration->getDesc().Elements_.size();
		VI.pVertexBindingDescriptions = BindingDescription.data();
		VI.pVertexAttributeDescriptions = AttributeDescription.data();

		// Bind up all elements to attributes.
		BcU32 SlotIdx = 0;
		BcU32 MaxStreamIdx = 0;
		for( const auto& Attribute : ProgramVertexAttributeList )
		{
			auto FoundElement = std::find_if( PrimitiveVertexElementList.begin(), PrimitiveVertexElementList.end(),
				[ &Attribute ]( const RsVertexElement& Element )
				{
					return ( Element.Usage_ == Attribute.Usage_ &&
						Element.UsageIdx_ == Attribute.UsageIdx_ );
				} );

			// Force to an element with zero offset if we can't find a valid one.
			// TODO: Find a better approach.
			if( FoundElement == PrimitiveVertexElementList.end() )
			{
				FoundElement = std::find_if( PrimitiveVertexElementList.begin(), PrimitiveVertexElementList.end(),
					[]( const RsVertexElement& Element )
					{
						return Element.Offset_ == 0;
					} );
			}

			// Found an element we can bind to.
			if( FoundElement != PrimitiveVertexElementList.end() )
			{
				auto VertexBufferBinding = GeometryBindingDesc.VertexBuffers_[ FoundElement->StreamIdx_ ];
				auto VertexBuffer = VertexBufferBinding.Buffer_;

				// Bind up new vertex buffer if we need to.
				BcAssertMsg( FoundElement->StreamIdx_ < GeometryBindingDesc.VertexBuffers_.size(), "Stream index out of bounds for primitive." );
				BcAssertMsg( VertexBuffer != nullptr, "Vertex buffer not bound!" );

				BindingDescription[ FoundElement->StreamIdx_ ].binding = FoundElement->StreamIdx_;
				BindingDescription[ FoundElement->StreamIdx_ ].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
				BindingDescription[ FoundElement->StreamIdx_ ].stride = VertexBufferBinding.Stride_;

				AttributeDescription[ SlotIdx ].binding = FoundElement->StreamIdx_;
				AttributeDescription[ SlotIdx ].format = RsUtilsVK::GetVertexElementFormat( *FoundElement );
				AttributeDescription[ SlotIdx ].location = SlotIdx;
				AttributeDescription[ SlotIdx ].offset = FoundElement->Offset_;

				MaxStreamIdx = std::max( MaxStreamIdx, FoundElement->StreamIdx_ );
				++SlotIdx;
			}
		}
		BcAssert( ProgramVertexAttributeList.size() == SlotIdx );
		VI.vertexBindingDescriptionCount = MaxStreamIdx + 1;

		memset( &IA, 0, sizeof( IA ) );
		IA.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		IA.topology = RsUtilsVK::GetPrimitiveTopology( TopologyType );

		memset( &RS, 0, sizeof( RS ) );
		RS.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		RS.polygonMode = VK_POLYGON_MODE_FILL;
		RS.cullMode = VK_CULL_MODE_NONE;
		RS.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		RS.depthClampEnable = VK_TRUE;
		RS.rasterizerDiscardEnable = VK_FALSE;
		RS.depthBiasEnable = VK_FALSE;

		memset ( &CB, 0, sizeof( CB ) );
		CB.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		VkPipelineColorBlendAttachmentState AttachmentState[ 1 ];
		memset( AttachmentState, 0, sizeof( AttachmentState ) );
		AttachmentState[0].colorWriteMask = 0xf;
		AttachmentState[0].blendEnable = VK_FALSE;
		CB.attachmentCount = 1;
		CB.pAttachments = AttachmentState;

		memset( &VP, 0, sizeof( VP ) );
		VP.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		VP.viewportCount = 1;
		DynamicStateEnables[ DynamicState.dynamicStateCount++ ] = VK_DYNAMIC_STATE_VIEWPORT;
		VP.scissorCount = 1;
		DynamicStateEnables[ DynamicState.dynamicStateCount++ ] = VK_DYNAMIC_STATE_SCISSOR;

		memset( &DS, 0, sizeof( DS ) );
		DS.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		DS.depthTestEnable = VK_TRUE;
		DS.depthWriteEnable = VK_TRUE;
		DS.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		DS.depthBoundsTestEnable = VK_FALSE;
		DS.back.failOp = VK_STENCIL_OP_KEEP;
		DS.back.passOp = VK_STENCIL_OP_KEEP;
		DS.back.compareOp = VK_COMPARE_OP_ALWAYS;
		DS.stencilTestEnable = VK_FALSE;
		DS.front = DS.back;

		memset( &MS, 0, sizeof( MS ) );
		MS.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		MS.pSampleMask = nullptr;
		MS.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineShaderStageCreateInfo ShaderStages[ 5 ];
		memset( ShaderStages, 0, sizeof( ShaderStages ) );

		BcU32 ShaderIdx = 0;
		auto* ProgramVK = Program->getHandle< const RsProgramVK* >();
		PipelineCreateInfo.stageCount = Program->getShaders().size();
		for( const auto* Shader : Program->getShaders() )
		{
			VkPipelineShaderStageCreateInfo ShaderStage;
			memset( &ShaderStage, 0, sizeof( VkPipelineShaderStageCreateInfo ) );

			ShaderStages[ ShaderIdx ].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			ShaderStages[ ShaderIdx ].stage  = RsUtilsVK::GetShaderStage( Shader->getDesc().ShaderType_ );
			ShaderStages[ ShaderIdx ].module = ProgramVK->getShaderModules()[ ShaderIdx ];
			ShaderStages[ ShaderIdx ].pSpecializationInfo = nullptr;
			
			++ShaderIdx;
		}

		if( !PipelineCache_ )
		{
			memset( &PipelineCache, 0, sizeof( PipelineCache ) );
			PipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

			VK( vkCreatePipelineCache( Device_, &PipelineCache, AllocationCallbacks_, &PipelineCache_ ) );
		}

		auto FrameBufferVK = FrameBuffer->getHandle< RsFrameBufferVK* >();

		PipelineCreateInfo.pVertexInputState = &VI;
		PipelineCreateInfo.pInputAssemblyState = &IA;
		PipelineCreateInfo.pRasterizationState = &RS;
		PipelineCreateInfo.pColorBlendState = &CB;
		PipelineCreateInfo.pMultisampleState = &MS;
		PipelineCreateInfo.pViewportState = &VP;
		PipelineCreateInfo.pDepthStencilState = &DS;
		PipelineCreateInfo.pStages = ShaderStages;
		PipelineCreateInfo.renderPass = BoundRenderPass_;
		PipelineCreateInfo.pDynamicState = &DynamicState;

		VK( vkCreateGraphicsPipelines( Device_, PipelineCache_, 1, &PipelineCreateInfo, AllocationCallbacks_, &Pipeline ) );

		PSOCache_[ PSOBinding ] = Pipeline;
	}
	else
	{
		Pipeline = FoundPSO->second;
	}

	if( Program->isGraphics() )
	{
		vkCmdBindDescriptorSets( getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipelineLayout_, 0, 1, 
			ProgramBindingVK->getDescriptorSets(), 0, nullptr );
		vkCmdBindPipeline( getCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, Pipeline );
	}
	else
	{
		vkCmdBindDescriptorSets( getCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, ComputePipelineLayout_, 0, 1,
			ProgramBindingVK->getDescriptorSets(), 0, nullptr );
		vkCmdBindPipeline( getCommandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, Pipeline );
	}

	// Bind vertex buffers.
	const auto& GeometryBindingDesc = GeometryBinding->getDesc();	
	for( size_t Idx = 0; Idx < GeometryBindingDesc.VertexBuffers_.size(); ++Idx )
	{
		const auto& VertexBufferBinding = GeometryBindingDesc.VertexBuffers_[ Idx ];
		if( VertexBufferBinding.Buffer_ )
		{
			auto BufferVK = VertexBufferBinding.Buffer_->getHandle< RsBufferVK* >();
			VkDeviceSize Offset = VertexBufferBinding.Offset_;
			VkBuffer Buffer = BufferVK->getBuffer();
			vkCmdBindVertexBuffers( getCommandBuffer(), Idx, 1, &Buffer, &Offset );
		}
	}

	// Bind index buffer.
	if( GeometryBindingDesc.IndexBuffer_.Buffer_ != nullptr )
	{
		auto BufferVK = GeometryBindingDesc.IndexBuffer_.Buffer_->getHandle< RsBufferVK* >();
		VkIndexType Type;
		VkDeviceSize Offset = GeometryBindingDesc.IndexBuffer_.Offset_;
		switch( GeometryBindingDesc.IndexBuffer_.Stride_ )
		{
		case 2:
			Type = VK_INDEX_TYPE_UINT16;
			break;
		case 4:
			Type = VK_INDEX_TYPE_UINT32;
			break;
		default:
			BcAssertMsg( BcFalse, "RsGeometryBinding \"%s\" has invalid index stride (%u)",
				GeometryBinding->getDebugName(),
				GeometryBindingDesc.IndexBuffer_.Stride_ );
		}
		vkCmdBindIndexBuffer( getCommandBuffer(), BufferVK->getBuffer(), Offset, Type );
	}	
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

	VkSamplerCreateInfo SamplerCreateInfo = {};
	SamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	SamplerCreateInfo.pNext = nullptr;
	SamplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	SamplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	SamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_BASE;
	SamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	SamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	SamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	SamplerCreateInfo.mipLodBias = 0.0f;
	SamplerCreateInfo.maxAnisotropy = 1;
	SamplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	SamplerCreateInfo.minLod = 0.0f;
	SamplerCreateInfo.maxLod = 0.0f;
	SamplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	SamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	VkSampler SamplerVK;
	VK( vkCreateSampler( Device_, &SamplerCreateInfo, AllocationCallbacks_, &SamplerVK ) );
	SamplerState->setHandle( SamplerVK );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroySamplerState
bool RsContextVK::destroySamplerState(
	RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	VkSampler SamplerVK = SamplerState->getHandle< VkSampler >();
	vkDestroySampler( Device_, SamplerVK, AllocationCallbacks_ );
	SamplerState->setHandle( 0 );
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
	new RsBufferVK( Buffer, Device_, Allocator_.get() ); 
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyBuffer
bool RsContextVK::destroyBuffer( 
	class RsBuffer* Buffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	RsBufferVK* BufferVK = Buffer->getHandle< RsBufferVK* >();
	BcAssert( BufferVK );
	delete BufferVK;
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
#if 1
	void* Data = nullptr;
	auto BufferVK = Buffer->getHandle< RsBufferVK* >();
	auto RetVal = VK( vkMapMemory( Device_, BufferVK->getDeviceMemory(), Offset, Size, 0, &Data ) );
	if( !RetVal )
	{
		RsBufferLock Lock = { Data };
		UpdateFunc( Buffer, Lock );
		vkUnmapMemory( Device_, BufferVK->getDeviceMemory() );
	}
#else
	std::unique_ptr< BcU8[] > TempBuffer;
	TempBuffer.reset( new BcU8[ Buffer->getDesc().SizeBytes_ ] );
	RsBufferLock Lock = { TempBuffer.get() };
	UpdateFunc( Buffer, Lock );
#endif
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
	VkImageAspectFlags Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	VkImageLayout Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	if( ( Desc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		Layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	else if( ( Desc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		Aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		Layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}
	else if( ( Desc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		Aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}

	// Set image layout.
	TextureVK->setImageLayout( getCommandBuffer(), Aspect, Layout );

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
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	new RsProgramVK( Program, Device_ );
	return true;
}


//////////////////////////////////////////////////////////////////////////
// destroyProgram
bool RsContextVK::destroyProgram(
	class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	RsProgramVK* ProgramVK = Program->getHandle< RsProgramVK* >();
	BcAssert( ProgramVK );
	delete ProgramVK;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// createProgramBinding
bool RsContextVK::createProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	if( ProgramBinding->getProgram()->isGraphics() )
	{
		new RsProgramBindingVK( ProgramBinding, Device_, GraphicsDescriptorSetLayout_ );
	}
	else
	{
		new RsProgramBindingVK( ProgramBinding, Device_, ComputeDescriptorSetLayout_ );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyProgramBinding
bool RsContextVK::destroyProgramBinding( class RsProgramBinding* ProgramBinding )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	RsProgramBindingVK* ProgramBindingVK = ProgramBinding->getHandle< RsProgramBindingVK* >();
	BcAssert( ProgramBindingVK );
	delete ProgramBindingVK;
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
