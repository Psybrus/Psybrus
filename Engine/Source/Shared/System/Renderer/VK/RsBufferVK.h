#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsBufferVK
class RsBufferVK
{
public:
	RsBufferVK( class RsBuffer* Parent, VkDevice Device, class RsAllocatorVK* Allocator );
	~RsBufferVK();

	VkBuffer getBuffer() const { return Buffer_; }
	VkBufferView getBufferView() const { return View_; }
	VkDeviceMemory getDeviceMemory() const { return DeviceMemory_; }

private:
	class RsBuffer* Parent_ = nullptr;
	VkDevice Device_ = 0;
	class RsAllocatorVK* Allocator_;
	VkBuffer Buffer_ = 0;
	VkDeviceMemory DeviceMemory_ = 0;
	VkBufferView View_ = 0;

};
