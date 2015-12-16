#include "System/Renderer/VK/RsTextureVK.h"
#include "System/Renderer/VK/RsAllocatorVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"

#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureVK::RsTextureVK( class RsTexture* Parent, VkDevice Device, class RsAllocatorVK* Allocator ):
	Parent_( Parent ),
	Device_( Device ),
	Allocator_( Allocator ),
	OwnImage_( true )
{
	Parent->setHandle( this );
	createImage();
	createViews();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsTextureVK::RsTextureVK( class RsTexture* Parent, VkDevice Device, class RsAllocatorVK* Allocator, VkImage Image ):
	Parent_( Parent ),
	Device_( Device ),
	Allocator_( Allocator ),
	OwnImage_( false ),
	Image_( Image )
{
	Parent->setHandle( this );
	createViews();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsTextureVK::~RsTextureVK()
{
	if( Image_ && OwnImage_ )
	{
		vkDestroyImage( Device_, Image_ );
		Image_ = 0;
	}

	if( DeviceMemory_ )
	{
		Allocator_->free( DeviceMemory_ );
		DeviceMemory_ = 0;
	}

	if( ImageView_ )
	{
		vkDestroyImageView( Device_, ImageView_ );
		ImageView_ = 0;
	}

	Parent_->setHandle( 0 );
}

//////////////////////////////////////////////////////////////////////////
// setImageLayout
void RsTextureVK::setImageLayout( VkCmdBuffer CommandBuffer, VkImageAspectFlags Aspect, VkImageLayout ImageLayout )
{
	VkImageMemoryBarrier ImageMemoryBarrier = {};
	ImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ImageMemoryBarrier.pNext = NULL;
	ImageMemoryBarrier.outputMask = 0;
	ImageMemoryBarrier.inputMask = 0;
	ImageMemoryBarrier.oldLayout = ImageLayout_;
	ImageMemoryBarrier.newLayout = ImageLayout;
	ImageMemoryBarrier.image = Image_;
	ImageMemoryBarrier.subresourceRange = { Aspect, 0, 1, 0, 0 };

	if( ImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DESTINATION_OPTIMAL )
	{
		// Make sure anything that was copying from this image has completed.
		ImageMemoryBarrier.inputMask = VK_MEMORY_INPUT_TRANSFER_BIT;
	}

	if( ImageLayout_ == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
	{
		// Make sure any Copy or CPU writes to image are flushed.
		ImageMemoryBarrier.outputMask = VK_MEMORY_OUTPUT_HOST_WRITE_BIT | VK_MEMORY_OUTPUT_TRANSFER_BIT;
	}

	VkImageMemoryBarrier* MemoryBarriers = &ImageMemoryBarrier;

	VkPipelineStageFlags SrcStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags DestStages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	vkCmdPipelineBarrier( CommandBuffer, SrcStages, DestStages, false, 1, (const void * const*)&MemoryBarriers );
}

//////////////////////////////////////////////////////////////////////////
// createViews
void RsTextureVK::createImage()
{
	const auto & Desc = Parent_->getDesc();

	VkImageCreateInfo ImageCreateInfo = {};
	ImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ImageCreateInfo.pNext = nullptr;
	switch( Desc.Type_ )
	{
		case RsTextureType::TEX1D:
			ImageCreateInfo.imageType = VK_IMAGE_TYPE_1D;
			ImageCreateInfo.arraySize = 1;
			ImageCreateInfo.flags = 0;
			break;
		case RsTextureType::TEX2D:
			ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			ImageCreateInfo.arraySize = 1;
			ImageCreateInfo.flags = 0;
			break;
		case RsTextureType::TEX3D:
			ImageCreateInfo.imageType = VK_IMAGE_TYPE_3D;
			ImageCreateInfo.arraySize = 1;
			ImageCreateInfo.flags = 0;
			break;
		case RsTextureType::TEXCUBE:
			ImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			ImageCreateInfo.arraySize = 6;
			ImageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			break;
	}
	ImageCreateInfo.format = RsUtilsVK::GetTextureFormat( Desc.Format_ );
	ImageCreateInfo.extent = 
	{
		static_cast< int32_t >( Desc.Width_ ),
		static_cast< int32_t >( Desc.Height_ ),
		static_cast< int32_t >( Desc.Depth_ )
	};
	ImageCreateInfo.mipLevels = Desc.Levels_;
	ImageCreateInfo.samples = 1;

	
	// Setup property flags, usage, and tiling.
	VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_ONLY;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	ImageCreateInfo.usage = 0;
	if( ( Desc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT;
	}	

	if( ( Desc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}	

	if( ( Desc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	}	

	if( ( Desc.BindFlags_ & RsResourceBindFlags::TRANSIENT ) != RsResourceBindFlags::NONE )
	{
		// TODO: Should not need to allocate memory for transient images.
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	}

	// If no usage is specified, assume it's for source transfers.
	if( ImageCreateInfo.usage == 0 )
	{
		PropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
		ImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT;
	}

	// Create image.
	VK( vkCreateImage( Device_, &ImageCreateInfo, &Image_ ) );

	// Allocate memory.
	VkMemoryRequirements MemoryRequirements = {};
	VK( vkGetImageMemoryRequirements( Device_, Image_, &MemoryRequirements ) );

	DeviceMemory_ = Allocator_->allocate( 
		MemoryRequirements.size, 
		MemoryRequirements.alignment,
		MemoryRequirements.memoryTypeBits,
		PropertyFlags );

	// Bind image memory.
    VK( vkBindImageMemory( Device_, Image_, DeviceMemory_ , 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// createViews
void RsTextureVK::createViews()
{
	const auto & Desc = Parent_->getDesc();

	// Create image view if needed.
	bool IsRT = ( Desc.BindFlags_ & ( RsResourceBindFlags::RENDER_TARGET ) ) != RsResourceBindFlags::NONE;
	bool IsDS = ( Desc.BindFlags_ & ( RsResourceBindFlags::DEPTH_STENCIL ) ) != RsResourceBindFlags::NONE;
	bool NeedsImageView = IsRT || IsDS ||
		( Desc.BindFlags_ & ( RsResourceBindFlags::SHADER_RESOURCE ) ) != RsResourceBindFlags::NONE;
	if( NeedsImageView )
	{
		VkImageViewCreateInfo ViewCreateInfo = {};
		ViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ViewCreateInfo.pNext = nullptr;
		ViewCreateInfo.image = Image_;
		ViewCreateInfo.format = RsUtilsVK::GetTextureFormat( Desc.Format_ );
		switch( Desc.Type_ )
		{
			case RsTextureType::TEX1D:
				ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
				break;
			case RsTextureType::TEX2D:
				ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
				break;
			case RsTextureType::TEX3D:
				ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
				break;
			case RsTextureType::TEXCUBE:
				ViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				break;
		}
		ViewCreateInfo.channels = {
			VK_CHANNEL_SWIZZLE_R,
			VK_CHANNEL_SWIZZLE_G,
			VK_CHANNEL_SWIZZLE_B,
			VK_CHANNEL_SWIZZLE_A
		};

		if( IsDS )
		{
			ViewCreateInfo.subresourceRange =  {
				VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 
				0, 1, 0, 1
			};
		}
		else
		{
			ViewCreateInfo.subresourceRange =  {
				VK_IMAGE_ASPECT_COLOR_BIT, 
				0, 1, 0, 1
			};
		}
		VK( vkCreateImageView( Device_, &ViewCreateInfo, &ImageView_ ) );
		if( !ImageView_ )
		{
			PSY_LOG( "WARNING: Unable to create view for RsTexture %s.", Parent_->getDebugName() );
		}
	}
}
