#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureVK
class RsTextureVK
{
public:
	/**
	 * Create texture.
	 */
	RsTextureVK( class RsTexture* Parent, VkDevice Device, class RsAllocatorVK* Alloctor );

	/**
	 * Create texture with already existing image.
	 */
	RsTextureVK( class RsTexture* Parent, VkDevice Device, class RsAllocatorVK* Alloctor, VkImage Image );

	/**
	 * Destroys all internal texture related.
	 */
	~RsTextureVK();

	/**
	 * Set image layout.
	 */
	void setImageLayout( VkCommandBuffer CommandBuffer, VkImageAspectFlags Aspect, VkImageLayout ImageLayout );

	VkImage getImage() const { return Image_; }
	VkImageView getImageView() const { return ImageView_; }
	VkImageLayout getImageLayout() const { return ImageLayout_; }
	VkDeviceMemory getDeviceMemory() const { return DeviceMemory_; }

private:
	void createImage();
	void createViews();


private:
	RsTexture* Parent_ = nullptr;
	VkDevice Device_ = 0;
	class RsAllocatorVK* Allocator_ = nullptr;
	bool OwnImage_ = false;
	VkImage Image_ = 0;
	VkImageLayout ImageLayout_ = VK_IMAGE_LAYOUT_GENERAL;
	VkMemoryRequirements MemoryRequirements_ = {};
	VkDeviceMemory DeviceMemory_ = 0;
	VkImageView ImageView_ = 0;
	
};
