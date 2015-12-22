#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////
// RsBufferAllocationVK
struct RsBufferAllocationVK
{
	/// Buffer we're pointing to.
	VkBuffer Buffer_;

	/// Device memory.
	VkDeviceMemory DeviceMemory_;

	/// Offset.
	BcU64 Offset_;

	/// Address of allocation.
	void* Address_;

	/// Size of allocation.
	BcU64 Size_;
};

//////////////////////////////////////////////////////////////////////
// RsLinearBufferAllocatorVK
class RsLinearBufferAllocatorVK
{
public:
	static const BcU64 DEFAULT_ALIGNMENT = 256;
	static const BcU64 MAX_ALIGNMENT = 64 * 1024;
	static const BcU64 MIN_RESOURCE_BLOCK_SIZE = MAX_ALIGNMENT;

public:
	RsLinearBufferAllocatorVK( VkDevice Device, class RsAllocatorVK* Allocator, VkMemoryPropertyFlagBits PropertyFlags, BcU64 MinResourceBlockSize );
	~RsLinearBufferAllocatorVK();

	/**
	 * Allocate.
	 */
	RsBufferAllocationVK allocate( BcU64 Size, BcU64 Alignment = DEFAULT_ALIGNMENT );

	/**
	 * Reset.
	 */
	void reset();


private:
	/// Block containing resource.
	struct ResourceBlock
	{
		VkBuffer Buffer_;
		VkDeviceMemory DeviceMemory_;
		void* BaseAddress_;
		BcU64 Size_;
		BcU64 CurrentOffset_;
		BcU64 AllocCounter_;
	};

	/**
	 * Create resource block.
	 */
	ResourceBlock createResourceBlock( BcU64 Size );

private:
	/// Device to use.
	VkDevice Device_;

	/// Main allocator to take from.
	class RsAllocatorVK* Allocator_ = nullptr;

	/// Property flags.
	VkMemoryPropertyFlagBits PropertyFlags_;

	/// Minimum resource block size.
	BcU64 MinResourceBlockSize_;

	/// Owning thread. We only want to be used from one thread to save locking.
	BcThreadId OwningThread_;

	/// Blocks in pool.
	std::vector< ResourceBlock > Blocks_;

	/// Blocks created.
	size_t BlocksCreated_;
};
