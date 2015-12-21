#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramBindingVK
class RsProgramBindingVK
{
public:
	RsProgramBindingVK( class RsProgramBinding* Parent, VkDevice Device, const VkDescriptorSetLayout* DescriptorSetLayouts, size_t DescriptorSetCount );
	virtual ~RsProgramBindingVK();

	const VkDescriptorSet* getDescriptorSets() const { return DescriptorSets_.data(); }

private:
	class RsProgramBinding* Parent_ = nullptr;
	VkDevice Device_;
	std::array< VkDescriptorSetLayout, 2 > DescriptorSetLayouts_ = {};
	VkDescriptorPool DescriptorPool_;
	std::array< VkDescriptorSet, 2 > DescriptorSets_ = {};
};

