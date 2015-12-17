#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsCommandBufferVK
class RsCommandBufferVK
{
public:
	RsCommandBufferVK( VkDevice Device );
	~RsCommandBufferVK();

private:
	VkDevice Device_ = 0;
	VkCommandPool CommandPool_ = 0;
	VkCommandBuffer CommandBuffer_ = 0;

};
