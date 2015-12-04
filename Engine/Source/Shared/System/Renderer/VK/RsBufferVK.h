#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsBufferVK
class RsBufferVK
{
public:
	RsBufferVK( class RsBuffer* Parent, VkDevice Device );
	~RsBufferVK();

private:
	class RsBuffer* Parent_ = nullptr;
	VkDevice Device_ = 0;
	VkBuffer Buffer_ = 0;
	VkBufferView View_ = 0;



};
