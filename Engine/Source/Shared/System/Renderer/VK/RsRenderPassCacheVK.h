#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsRenderPassCacheVK
class RsRenderPassCacheVK
{
public:
	RsRenderPassCacheVK();
	~RsRenderPassCacheVK();

	/**
	 * Get render pass for frame buffer.
	 */
	VkRenderPass getRenderPass( class RsFrameBufferVK* FrameBuffer );

private:
	

};

