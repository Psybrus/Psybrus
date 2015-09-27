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
	VkCmdPool CommandPool_ = 0;
	VkCmdBuffer CommandBuffer_ = 0;

};
