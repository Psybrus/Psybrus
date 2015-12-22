#include "System/Renderer/VK/RsBufferVK.h"
#include "System/Renderer/VK/RsAllocatorVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"
#include "System/Renderer/RsBuffer.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsBufferVK::RsBufferVK( class RsBuffer* Parent, VkDevice Device, RsAllocatorVK* Allocator ):
	Parent_( Parent ),
	Device_( Device ),
	Allocator_( Allocator )
{
	Parent_->setHandle( this );

	const auto& BufferDesc = Parent_->getDesc();
	VkBufferCreateInfo BufferCreateInfo;
	BufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	BufferCreateInfo.pNext = nullptr;
	BufferCreateInfo.size = BufferDesc.SizeBytes_;
	BufferCreateInfo.usage = RsUtilsVK::GetBufferUsageFlags( BufferDesc.BindFlags_ ) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	BufferCreateInfo.flags = 0;
	BufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	BufferCreateInfo.queueFamilyIndexCount = 0;
	BufferCreateInfo.pQueueFamilyIndices = nullptr;
	VK( vkCreateBuffer( Device, &BufferCreateInfo, nullptr/*allocation*/, &Buffer_ ) );

	VkMemoryAllocateInfo MemAlloc;
	MemAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemAlloc.pNext = nullptr;
	MemAlloc.allocationSize = 0;
	MemAlloc.memoryTypeIndex = 0;

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements( Device_, Buffer_, &MemoryRequirements );

	DeviceMemory_ = Allocator_->allocate( 
		MemoryRequirements.size, 
		MemoryRequirements.alignment,
		MemoryRequirements.memoryTypeBits,
		{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT } );

	// Bind image memory.
	VK( vkBindBufferMemory( Device_, Buffer_, DeviceMemory_ , 0 ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsBufferVK::~RsBufferVK()
{
	Parent_->setHandle( nullptr );
	vkDestroyBuffer( Device_, Buffer_, nullptr/*allocation*/ );
	Allocator_->free( DeviceMemory_ );
}
