#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramBindingVK
class RsProgramBindingVK
{
public:
	RsProgramBindingVK( class RsProgramBinding* Parent, VkDevice Device, VkDescriptorSetLayout DescriptorSetLayout );
	virtual ~RsProgramBindingVK();

	const VkDescriptorSet* getDescriptorSets() const { return &DescriptorSet_; }

private:
	class RsProgramBinding* Parent_ = nullptr;
	VkDevice Device_;
	VkDescriptorSetLayout DescriptorSetLayout_;
	VkDescriptorPool DescriptorPool_;
	VkDescriptorSet DescriptorSet_;
};

