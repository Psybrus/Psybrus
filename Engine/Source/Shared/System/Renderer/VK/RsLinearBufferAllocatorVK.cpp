#include "System/Renderer/VK/RsLinearBufferAllocatorVK.h"
#include "System/Renderer/VK/RsAllocatorVK.h"
#include "Base/BcMath.h"
#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////
// RsLinearBufferAllocatorVK
RsLinearBufferAllocatorVK::RsLinearBufferAllocatorVK( 
		VkDevice Device, class RsAllocatorVK* Allocator, VkMemoryPropertyFlagBits PropertyFlags, BcU64 MinResourceBlockSize ):
	Device_( Device ),
	Allocator_( Allocator ),
	PropertyFlags_( PropertyFlags ),
	MinResourceBlockSize_( MinResourceBlockSize ),
	OwningThread_( BcCurrentThreadId() ),
	Blocks_(),
	BlocksCreated_( 0 )
{
	BcAssert( MinResourceBlockSize_ >= MIN_RESOURCE_BLOCK_SIZE );
	BcAssert( ( MinResourceBlockSize_ & ( MAX_ALIGNMENT - 1 ) ) == 0 );
	BcAssert( ( PropertyFlags_ & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) != 0 );
}

//////////////////////////////////////////////////////////////////////
// Dtor
RsLinearBufferAllocatorVK::~RsLinearBufferAllocatorVK()
{
	for( auto& Block : Blocks_ )
	{
		vkDestroyBuffer( Device_, Block.Buffer_, nullptr/*allocation*/ );
		Allocator_->free( Block.DeviceMemory_ );
	}
}

//////////////////////////////////////////////////////////////////////
// Dtor
RsBufferAllocationVK RsLinearBufferAllocatorVK::allocate( BcU64 Size, BcU64 Alignment )
{
	BcAssert( OwningThread_ == BcCurrentThreadId() );
	BcAssert( Alignment <= MAX_ALIGNMENT );

	RsBufferAllocationVK Allocation = 
	{
		nullptr,
		nullptr,
		0,
		nullptr,
		0
	};

	ResourceBlock* FoundBlock = nullptr;
	
	// Check if we have a valid block size.
	for( auto & Block : Blocks_ )
	{
		BcU64 AlignedOffset = BcPotRoundUp( Block.CurrentOffset_, Alignment );
		BcS64 Remaining = static_cast< BcS64 >( Block.Size_ - AlignedOffset );
		if( Remaining >= static_cast< BcS64 >( Size ) )
		{
			FoundBlock = &Block;
		}
	}
	
	// Allocate new block if need be.
	if( FoundBlock == nullptr )
	{
		auto NewBlock = createResourceBlock( Size );
		Blocks_.push_back( NewBlock );
		FoundBlock = &Blocks_.back();
	}

	// Grab the correct offset and assert remaining siz.e
	BcU64 AlignedOffset = BcPotRoundUp( FoundBlock->CurrentOffset_, Alignment );
	BcS64 Remaining = static_cast< BcS64 >( FoundBlock->Size_ - AlignedOffset );
	BcAssert( Remaining >= static_cast< BcS64 >( Size ) );

	// Setup allocation
	if( Remaining >= static_cast< BcS64 >( Size ) )
	{
		Allocation.Buffer_ = FoundBlock->Buffer_;
		Allocation.DeviceMemory_ = FoundBlock->DeviceMemory_;
		Allocation.Offset_ = AlignedOffset;
		Allocation.Address_ = static_cast< BcU8* >( FoundBlock->BaseAddress_ ) + AlignedOffset;
		Allocation.Size_ = Size;

		// Advance current offset.
		FoundBlock->CurrentOffset_ = AlignedOffset + Size;
		BcAssert( ( Allocation.Offset_ & ( Alignment - 1 ) ) == 0 );
	}


	return std::move( Allocation );
}

//////////////////////////////////////////////////////////////////////
// reset
void RsLinearBufferAllocatorVK::reset()
{
	BcAssert( OwningThread_ == BcCurrentThreadId() );
	PSY_LOGSCOPEDCATEGORY( RsLinearBufferAllocatorVK );

	BcU64 TotalUsage = 0;
	BcU64 TotalSize = 0;
	
	// Set block offsets back to 0.
	for( auto& Block : Blocks_ )
	{
		TotalUsage += Block.CurrentOffset_;
		TotalSize += Block.Size_;
		Block.CurrentOffset_ = 0;
	}

	// If we have many blocks, merge into 1 and create as large as current total size.
	if( Blocks_.size() > 1 )
	{
		PSY_LOG( "More than 1 block allocated. Compacting." );
		PSY_LOGSCOPEDINDENT;
		PSY_LOG( "Total size: %u kB", TotalSize / 1024 );
		PSY_LOG( "Total usage this frame: %u kB", TotalUsage / 1024 );
		PSY_LOG( "Blocks: %u", Blocks_.size() );
		PSY_LOG( "Blocks created: %u", BlocksCreated_ );

		for( auto& Block : Blocks_ )
		{
			vkDestroyBuffer( Device_, Block.Buffer_, nullptr/*allocation*/ );
			Allocator_->free( Block.DeviceMemory_ );
		}

		Blocks_.clear();
		auto Block = createResourceBlock( TotalSize );
		Blocks_.push_back( Block );
	}	
	
	// Reset stats.
	BlocksCreated_ = 0;
}

//////////////////////////////////////////////////////////////////////
// createResourceBlock
RsLinearBufferAllocatorVK::ResourceBlock RsLinearBufferAllocatorVK::createResourceBlock( BcU64 Size )
{
	// Minimum sized block, round up to max alignment.
	ResourceBlock Block = 
	{
		nullptr,
		nullptr,
		nullptr,
		BcPotRoundUp( std::max( Size, MinResourceBlockSize_ ), MAX_ALIGNMENT ),
		0
	};

	// Setup heap properties.
	VkBufferCreateInfo BufferCreateInfo;
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.pNext = nullptr;
	BufferCreateInfo.size = Block.Size_;
	BufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	BufferCreateInfo.flags = 0;
	BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	BufferCreateInfo.queueFamilyIndexCount = 0;
	BufferCreateInfo.pQueueFamilyIndices = nullptr;
	VK( vkCreateBuffer( Device_, &BufferCreateInfo, nullptr/*allocation*/, &Block.Buffer_ ) );

	VkMemoryAllocateInfo MemAlloc;
	MemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemAlloc.pNext = nullptr;
	MemAlloc.allocationSize = 0;
	MemAlloc.memoryTypeIndex = 0;

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements( Device_, Block.Buffer_, &MemoryRequirements );

	// TODO: Use proper visibility flag.
	VkMemoryPropertyFlagBits PropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	Block.DeviceMemory_ = Allocator_->allocate( 
		MemoryRequirements.size, 
		MemoryRequirements.alignment,
		MemoryRequirements.memoryTypeBits,
		PropertyFlags );

	// Bind image memory.
	VK( vkBindBufferMemory( Device_, Block.Buffer_, Block.DeviceMemory_ , 0 ) );

	// Persistently map.
	auto RetVal = VK( vkMapMemory( Device_, Block.DeviceMemory_, 0, Block.Size_, 0, &Block.BaseAddress_ ) );
	BcAssert( !RetVal );

	// Blocks created.
	++BlocksCreated_;

	return std::move( Block );
}
