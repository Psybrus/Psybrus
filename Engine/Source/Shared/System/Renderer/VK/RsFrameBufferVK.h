#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsFrameBufferVK
class RsFrameBufferVK
{
public:
	RsFrameBufferVK( class RsFrameBuffer* Parent, VkDevice Device );
	~RsFrameBufferVK();

	VkFramebuffer getLoadFrameBuffer() const { return FrameBuffers_[0]; }
	VkRenderPass getLoadRenderPass() const { return RenderPasses_[0]; }

	VkFramebuffer getClearFrameBuffer() const { return FrameBuffers_[1]; }
	VkRenderPass getClearRenderPass() const { return RenderPasses_[1]; }

	/**
	 * Get format hash.
	 */
	BcU32 getFormatHash() const { return FormatHash_; }

private:
	void createFrameBuffer();
	void createRenderPass();

private:
	class RsFrameBuffer* Parent_ = nullptr;
	VkDevice Device_ = 0;
	std::array< VkRenderPass, 2 > RenderPasses_;
	std::array< VkFramebuffer, 2 > FrameBuffers_;
	BcU32 FormatHash_ = 0;
};

