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
// beginFrame
void RsContextVK::beginFrame( BcU32 Width, BcU32 Height )
{
	Width_ = Width;
	Height_ = Height;
}

//////////////////////////////////////////////////////////////////////////
// endFrame
void RsContextVK::endFrame()
{
}

//////////////////////////////////////////////////////////////////////////
// takeScreenshot
void RsContextVK::takeScreenshot( RsScreenshotFunc ScreenshotFunc )
{
}

//////////////////////////////////////////////////////////////////////////
// setViewport
void RsContextVK::setViewport( const class RsViewport& )
{
}

//////////////////////////////////////////////////////////////////////////
// create
void RsContextVK::setScissorRect( BcS32 X, BcS32 Y, BcS32 Width, BcS32 Height )
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

	// Grab layers.
	uint32_t InstanceLayerCount = 0;
    if( ( RetVal = vkGetGlobalLayerProperties( &InstanceLayerCount, nullptr ) ) == VK_SUCCESS )
	{
		InstanceLayers_.resize( InstanceLayerCount );
		RetVal = vkGetGlobalLayerProperties( &InstanceLayerCount, InstanceLayers_.data() );
		BcAssert( !RetVal );
	}
	else
	{
		// TODO: Error message.
		BcBreakpoint;
		return;
	}

	// Grab extensions.
	uint32_t InstanceExtensionCount = 0;
	if( ( RetVal = vkGetGlobalExtensionProperties( nullptr, &InstanceExtensionCount, nullptr ) ) == VK_SUCCESS )
	{
		InstanceExtensions_.resize( InstanceExtensionCount );
		RetVal = vkGetGlobalExtensionProperties( nullptr, &InstanceExtensionCount, InstanceExtensions_.data() );
		BcAssert( !RetVal );
	}
	else
	{
		// TODO: Error message.
		BcBreakpoint;
		return;
	}

	// Setup application.
	AppInfo_.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	AppInfo_.pNext = nullptr;
	AppInfo_.pAppName = "Psybrus";
	AppInfo_.appVersion = 0;
	AppInfo_.pEngineName = "Psybrus";
	AppInfo_.engineVersion = 0;
	AppInfo_.apiVersion = VK_API_VERSION;

	InstanceCreateInfo_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	InstanceCreateInfo_.pNext = nullptr;
	InstanceCreateInfo_.pAppInfo = &AppInfo_;
	InstanceCreateInfo_.pAllocCb = nullptr;
	InstanceCreateInfo_.layerCount = 0;
	InstanceCreateInfo_.ppEnabledLayerNames = nullptr;
	InstanceCreateInfo_.extensionCount = 0;
	InstanceCreateInfo_.ppEnabledExtensionNames = nullptr;

	// Create instance.
	if( ( RetVal = vkCreateInstance( &InstanceCreateInfo_, &Instance_ ) ) == VK_SUCCESS )
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

		// Create first device.
		DeviceQueueCreateInfo_.queueFamilyIndex = 0;
		DeviceQueueCreateInfo_.queueCount = 1;
		DeviceCreateInfo_.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		DeviceCreateInfo_.pNext = nullptr;
		DeviceCreateInfo_.queueRecordCount = 1;
		DeviceCreateInfo_.pRequestedQueues = &DeviceQueueCreateInfo_;
		DeviceCreateInfo_.layerCount = 0;
		DeviceCreateInfo_.ppEnabledLayerNames = nullptr;
		DeviceCreateInfo_.extensionCount = 0;
		DeviceCreateInfo_.ppEnabledExtensionNames = nullptr;
		DeviceCreateInfo_.flags = 0;

		if( ( RetVal = vkCreateDevice( PhysicalDevices_[ 0 ], &DeviceCreateInfo_, &Device_ ) ) == VK_SUCCESS )
		{
			// Get fps.
			GET_INSTANCE_PROC_ADDR( Instance_, GetPhysicalDeviceSurfaceSupportWSI );
			GET_DEVICE_PROC_ADDR( Device_, GetSurfaceInfoWSI );
			GET_DEVICE_PROC_ADDR( Device_, CreateSwapChainWSI );
			GET_DEVICE_PROC_ADDR( Device_, CreateSwapChainWSI );
			GET_DEVICE_PROC_ADDR( Device_, DestroySwapChainWSI );
			GET_DEVICE_PROC_ADDR( Device_, GetSwapChainInfoWSI );
			GET_DEVICE_PROC_ADDR( Device_, AcquireNextImageWSI );
			GET_DEVICE_PROC_ADDR( Device_, QueuePresentWSI );

			//
			RetVal = vkGetPhysicalDeviceProperties( PhysicalDevices_[ 0 ], &DeviceProps_ );
			BcAssert( !RetVal );
			uint32_t DeviceQueueCount = 0;
			RetVal = vkGetPhysicalDeviceQueueCount( PhysicalDevices_[ 0 ], &DeviceQueueCount );
			BcAssert( !RetVal );
			DeviceQueueProps_.resize( DeviceQueueCount );
			RetVal = vkGetPhysicalDeviceQueueProperties( PhysicalDevices_[ 0 ], DeviceQueueCount, DeviceQueueProps_.data() );
			BcAssert( !RetVal );
		}
		else
		{
			// TODO: Error message.
			BcBreakpoint;
			return;
		}

		// Setup windowing.
		WindowSurfaceDesc_.sType = VK_STRUCTURE_TYPE_SURFACE_DESCRIPTION_WINDOW_WSI;
		WindowSurfaceDesc_.pNext = NULL;
#ifdef PLATFORM_WINDOWS
		WindowSurfaceDesc_.platform = VK_PLATFORM_WIN32_WSI;
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
			if( ( RetVal = fpGetPhysicalDeviceSurfaceSupportWSI_( PhysicalDevices_[ 0 ], Idx, (VkSurfaceDescriptionWSI*)&WindowSurfaceDesc_, &SupportsPresent ) ) == VK_SUCCESS )
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
		size_t FormatSize = 0;
		if( ( RetVal = fpGetSurfaceInfoWSI_( Device_, (VkSurfaceDescriptionWSI*)&WindowSurfaceDesc_, VK_SURFACE_INFO_TYPE_FORMATS_WSI, &FormatSize, nullptr ) ) == VK_SUCCESS )
		{
			SurfaceFormats_.resize( FormatSize / sizeof( VkSurfaceFormatPropertiesWSI ) );
			RetVal = fpGetSurfaceInfoWSI_( Device_, (VkSurfaceDescriptionWSI*)&WindowSurfaceDesc_, VK_SURFACE_INFO_TYPE_FORMATS_WSI, &FormatSize, SurfaceFormats_.data() );
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

		// Swap chain.
		// TODO: Get present mode info and try to use mailbox, then try immediate, then try FIFO.
		// TODO: Get present mode info to determine correct number of iamges. Use 3 for now.
		// TODO: Get present mode info to get pre transform.
	    VkPresentModeWSI SwapChainPresentMode = VK_PRESENT_MODE_FIFO_WSI;
		uint32_t DesiredNumberOfSwapChainImages = 3;
		VkSurfaceTransformWSI PreTransform = VK_SURFACE_TRANSFORM_NONE_WSI;


		SwapChainCreateInfo_.sType = VK_STRUCTURE_TYPE_SWAP_CHAIN_CREATE_INFO_WSI;
        SwapChainCreateInfo_.pNext = nullptr;
        SwapChainCreateInfo_.pSurfaceDescription = (const VkSurfaceDescriptionWSI *)&SurfaceDescription_;
        SwapChainCreateInfo_.minImageCount = DesiredNumberOfSwapChainImages;
        SwapChainCreateInfo_.imageFormat = SurfaceFormats_[ 0 ].format;
		SwapChainCreateInfo_.imageExtent.width = pClient_->getWidth();
		SwapChainCreateInfo_.imageExtent.height = pClient_->getHeight();
        SwapChainCreateInfo_.preTransform = PreTransform;
        SwapChainCreateInfo_.imageArraySize = 1;
        SwapChainCreateInfo_.presentMode = SwapChainPresentMode;
        SwapChainCreateInfo_.oldSwapChain.handle = 0;
        SwapChainCreateInfo_.clipped = true;

		RetVal = fpCreateSwapChainWSI_( Device_, &SwapChainCreateInfo_, &SwapChain_ );
		BcAssert( !RetVal );

		size_t SwapChainImagesSize = 0;
		RetVal = fpGetSwapChainInfoWSI_( Device_, SwapChain_, 
			VK_SWAP_CHAIN_INFO_TYPE_IMAGES_WSI, &SwapChainImagesSize, nullptr );
		BcAssert( !RetVal );

		size_t SwapChainImageCount = SwapChainImagesSize / sizeof( VkSwapChainImagePropertiesWSI );
		SwapChainImages_.resize( SwapChainImageCount );
		RetVal = fpGetSwapChainInfoWSI_( Device_, SwapChain_, 
			VK_SWAP_CHAIN_INFO_TYPE_IMAGES_WSI, &SwapChainImagesSize, (VkSwapChainImagePropertiesWSI*)SwapChainImages_.data() );
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
			RsTextureVK* TextureVK = new RsTextureVK( SwapChainTexture, Device_, Allocator_.get(), SwapChainImages_[ Idx ].image );
			SwapChainTexture->setHandle( TextureVK );
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
			RsTextureVK* TextureVK = new RsTextureVK( DepthStencilTexture_, Device_, Allocator_.get() );
			DepthStencilTexture_->setHandle( TextureVK );
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
			RsFrameBufferVK* FrameBufferVK = new RsFrameBufferVK( FrameBuffer, Device_ );
			FrameBuffer->setHandle( FrameBufferVK );
		}		
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
// setDefaultState
void RsContextVK::setDefaultState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateRenderState
void RsContextVK::invalidateRenderState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// invalidateTextureState
void RsContextVK::invalidateTextureState()
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setRenderState
void RsContextVK::setRenderState( RsRenderState* RenderState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextVK::setSamplerState( BcU32 Handle, class RsSamplerState* SamplerState )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsContextVK::setSamplerState( BcU32 Handle, const RsTextureParams& Params, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setTexture
void RsContextVK::setTexture( BcU32 Handle, RsTexture* pTexture, BcBool Force )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setProgram
void RsContextVK::setProgram( class RsProgram* Program )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setIndexBuffer
void RsContextVK::setIndexBuffer( class RsBuffer* IndexBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// setVertexBuffer
void RsContextVK::setVertexBuffer( 
	BcU32 StreamIdx, 
	class RsBuffer* VertexBuffer,
	BcU32 Stride )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( StreamIdx );
	BcUnusedVar( VertexBuffer );
	BcUnusedVar( Stride );
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsContextVK::setUniformBuffer( 
	BcU32 Handle, 
	class RsBuffer* UniformBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Handle );
	BcUnusedVar( UniformBuffer );
}

//////////////////////////////////////////////////////////////////////////
// setVertexDeclaration
void RsContextVK::setVertexDeclaration( class RsVertexDeclaration* VertexDeclaration )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( VertexDeclaration );
}

//////////////////////////////////////////////////////////////////////////
// setFrameBuffer
void RsContextVK::setFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
}

//////////////////////////////////////////////////////////////////////////
// clear
void RsContextVK::clear( 
	const RsColour& Colour,
	BcBool EnableClearColour,
	BcBool EnableClearDepth,
	BcBool EnableClearStencil )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Colour );
	BcUnusedVar( EnableClearColour );
	BcUnusedVar( EnableClearDepth );
	BcUnusedVar( EnableClearStencil );
}

//////////////////////////////////////////////////////////////////////////
// drawPrimitives
void RsContextVK::drawPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
}

//////////////////////////////////////////////////////////////////////////
// drawIndexedPrimitives
void RsContextVK::drawIndexedPrimitives( RsTopologyType PrimitiveType, BcU32 IndexOffset, BcU32 NoofIndices, BcU32 VertexOffset )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( PrimitiveType );
	BcUnusedVar( IndexOffset );
	BcUnusedVar( NoofIndices );
	BcUnusedVar( VertexOffset );
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
	BcUnusedVar( FrameBuffer );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyFrameBuffer
bool RsContextVK::destroyFrameBuffer( class RsFrameBuffer* FrameBuffer )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( FrameBuffer );
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
	BcUnusedVar( Texture );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// destroyTexture
bool RsContextVK::destroyTexture( 
	class RsTexture* Texture )
{
	BcAssertMsg( BcCurrentThreadId() == OwningThread_, "Calling context calls from invalid thread." );
	BcUnusedVar( Texture );
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
	std::unique_ptr< BcU8[] > TempBuffer;
		BcU32 Width = BcMax( 1, TextureDesc.Width_ >> Slice.Level_ );
		BcU32 Height = BcMax( 1, TextureDesc.Height_ >> Slice.Level_ );
		BcU32 Depth = BcMax( 1, TextureDesc.Depth_ >> Slice.Level_ );
		BcU32 DataSize = RsTextureFormatSize( 
			TextureDesc.Format_,
			Width,
			Height,
			Depth,
			1 );
	TempBuffer.reset( new BcU8[ DataSize ] );
	RsTextureLock Lock = 
	{
		TempBuffer.get(),
		TextureDesc.Width_,
		TextureDesc.Width_ * TextureDesc.Height_
	};	
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
