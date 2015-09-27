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

	if( Memory_ )
	{
		Allocator_->free( Memory_ );
		Memory_ = 0;
	}

	if( ImageView_ )
	{
		vkDestroyImageView( Device_, ImageView_ );
		ImageView_ = 0;
	}

	if( AttachmentView_ )
	{
		vkDestroyAttachmentView( Device_, AttachmentView_ );
		AttachmentView_ = 0;
	}
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
	VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	ImageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
	ImageCreateInfo.usage = VK_IMAGE_USAGE_GENERAL;
	if( ( Desc.BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE )
	{
		PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_ONLY;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}	

	if( ( Desc.BindFlags_ & RsResourceBindFlags::RENDER_TARGET ) != RsResourceBindFlags::NONE )
	{
		PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_ONLY;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	}	

	if( ( Desc.BindFlags_ & RsResourceBindFlags::DEPTH_STENCIL ) != RsResourceBindFlags::NONE )
	{
		PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_ONLY;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_DEPTH_STENCIL_BIT;
	}	

	if( ( Desc.BindFlags_ & RsResourceBindFlags::TRANSIENT ) != RsResourceBindFlags::NONE )
	{
		// TODO: Should not need to allocate memory for transient images.
		PropertyFlags = VK_MEMORY_PROPERTY_DEVICE_ONLY;
		ImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		ImageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
	}

	// Create image.
	auto RetVal = vkCreateImage( Device_, &ImageCreateInfo, &Image_ );
	BcAssert( !RetVal );

	// Allocate memory.
	VkMemoryRequirements MemoryRequirements = {};
	RetVal = vkGetImageMemoryRequirements( Device_, Image_, &MemoryRequirements );
	BcAssert( !RetVal );

	Memory_ = Allocator_->allocate( 
		MemoryRequirements.size, 
		MemoryRequirements.alignment,
		MemoryRequirements.memoryTypeBits,
		PropertyFlags );
}

//////////////////////////////////////////////////////////////////////////
// createViews
void RsTextureVK::createViews()
{
	const auto & Desc = Parent_->getDesc();

	// Create image view if needed.
	bool NeedsImageView = ( Desc.BindFlags_ & ( RsResourceBindFlags::SHADER_RESOURCE ) ) != RsResourceBindFlags::NONE;
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
		ViewCreateInfo.subresourceRange =  {
			VK_IMAGE_ASPECT_COLOR, 
			0, 1, 0, 1
		};

		auto RetVal = vkCreateImageView( Device_, &ViewCreateInfo, &ImageView_ );
		BcAssert( !RetVal );
	}

	// Create attachment view if needed.
	bool NeedsAttachmentView = ( Desc.BindFlags_ & ( RsResourceBindFlags::RENDER_TARGET | RsResourceBindFlags::DEPTH_STENCIL ) ) != RsResourceBindFlags::NONE;
	if( NeedsAttachmentView )
	{
		VkAttachmentViewCreateInfo ViewCreateInfo = {};
		ViewCreateInfo.sType = VK_STRUCTURE_TYPE_ATTACHMENT_VIEW_CREATE_INFO;
		ViewCreateInfo.pNext = nullptr;
		ViewCreateInfo.image = Image_;
		ViewCreateInfo.format = RsUtilsVK::GetTextureFormat( Desc.Format_ );
		ViewCreateInfo.mipLevel = 0;
		ViewCreateInfo.baseArraySlice = 0;
		ViewCreateInfo.arraySize = 1;
		auto RetVal = vkCreateAttachmentView( Device_, &ViewCreateInfo, &AttachmentView_ );
		BcAssert( !RetVal );
	}
}
