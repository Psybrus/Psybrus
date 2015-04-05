#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"

//////////////////////////////////////////////////////////////////////
// RsResourceAllocationD3D12
struct RsResourceAllocationD3D12
{
	/// Base resource we're pointing to.
	ComPtr< ID3D12Resource > BaseResource_;

	/// Offset in base resource.
	BcU64 OffsetInBaseResource_;

	/// Address of allocation.
	void* Address_;

	/// Size of allocation.
	BcU64 Size_;
};

//////////////////////////////////////////////////////////////////////
// RsLinearHeapAllocatorD3D12
class RsLinearHeapAllocatorD3D12
{
public:
	static const BcU64 DEFAULT_ALIGNMENT = 256;
	static const BcU64 MAX_ALIGNMENT = 64 * 1024;
	static const BcU64 MIN_RESOURCE_BLOCK_SIZE = MAX_ALIGNMENT;

public:
	RsLinearHeapAllocatorD3D12( ID3D12Device* Device, D3D12_HEAP_TYPE HeapType, BcU64 MinResourceBlockSize );
	~RsLinearHeapAllocatorD3D12();

	/**
	 * Allocate.
	 */
	RsResourceAllocationD3D12 allocate( BcU64 Size, BcU64 Alignment = DEFAULT_ALIGNMENT );

	/**
	 * Reset.
	 */
	void reset();


private:
	/// Block containing resource.
	struct ResourceBlock
	{
		ComPtr< ID3D12Resource > Resource_;
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
	ComPtr< ID3D12Device > Device_;

	/// Heap type we are allocating for.
	D3D12_HEAP_TYPE HeapType_;

	/// Minimum resource block size.
	BcU64 MinResourceBlockSize_;

	/// Owning thread. We only want to be used from one thread to save locking.
	BcThreadId OwningThread_;

	/// Blocks in pool.
	std::vector< ResourceBlock > Blocks_;
};
