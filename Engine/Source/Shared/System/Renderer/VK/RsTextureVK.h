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

	VkImage getImage() const { return Image_; }
	VkImageView getImageView() const { return ImageView_; }
	VkDeviceMemory getDeviceMemory() const { return DeviceMemory_; }
	VkAttachmentView getAttachmentView() const { return AttachmentView_; }


private:
	void createImage();
	void createViews();


private:
	RsTexture* Parent_ = nullptr;
	VkDevice Device_ = 0;
	class RsAllocatorVK* Allocator_ = nullptr;
	bool OwnImage_ = false;
	VkImage Image_ = 0;
	VkImageLayout ImageLayout_ = VK_IMAGE_LAYOUT_UNDEFINED;
	VkDeviceMemory DeviceMemory_ = 0;
	VkImageView ImageView_ = 0;
	VkAttachmentView AttachmentView_ = 0;

};
