#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsAllocatorVK
class RsAllocatorVK
{
public:
	RsAllocatorVK( VkPhysicalDevice PhysicalDevice, VkDevice Device );

	/**
	 * Allocate memory.
	 * @param Size Size in bytes.
	 * @param Size Alignment in bytes.
	 * @param TypeFlags Type of memory that is valid to allocate.
	 * @param PropertyFlags Flags to determine which heap to allocate from.
	 */
	VkDeviceMemory allocate( size_t Size, size_t Alignment, uint32_t TypeFlags, VkMemoryPropertyFlagBits PropertyFlags );

	/**
	 * Allocate memory.
	 * @param Size Size in bytes.
	 * @param Size Alignment in bytes.
	 * @param TypeFlags Type of memory that is valid to allocate.
	 * @param PropertyFlagsList Flags to determine which heap to allocate from. Will attempt them in order.
	 */
	VkDeviceMemory allocate( size_t Size, size_t Alignment, uint32_t TypeFlags, std::initializer_list< VkMemoryPropertyFlagBits > PropertyFlagsList );

	/**
	 * Free memory.
	 * @param Memory Memory to free.
	 */
	void free( VkDeviceMemory Memory );

private:


private:
	VkPhysicalDevice PhysicalDevice_ = 0;
	VkDevice Device_ = 0;
	VkPhysicalDeviceMemoryProperties MemoryProperties_ = {};
};
