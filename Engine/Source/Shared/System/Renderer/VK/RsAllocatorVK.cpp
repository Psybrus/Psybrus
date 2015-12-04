#include "System/Renderer/VK/RsAllocatorVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsAllocatorVK::RsAllocatorVK( VkPhysicalDevice PhysicalDevice, VkDevice Device ):
	PhysicalDevice_( PhysicalDevice ),
	Device_( Device )
{
	auto RetVal = vkGetPhysicalDeviceMemoryProperties( PhysicalDevice_, &MemoryProperties_ );
	BcAssert( !RetVal );
}


//////////////////////////////////////////////////////////////////////////
// allocate
VkDeviceMemory RsAllocatorVK::allocate( size_t Size, size_t Alignment, uint32_t TypeFlags, VkMemoryPropertyFlagBits PropertyFlags )
{
	VkDeviceMemory Memory = 0;
	VkMemoryAllocInfo MemoryAllocInfo = {};
	MemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO;
	MemoryAllocInfo.pNext = nullptr;
	MemoryAllocInfo.allocationSize = Size;
	MemoryAllocInfo.memoryTypeIndex = std::numeric_limits< uint32_t >::max();
	for( uint32_t Idx = 0; Idx < MemoryProperties_.memoryTypeCount; ++Idx )
	{
		// Check type.
		if( ( TypeFlags & 1 ) == 1 )
		{
			if( ( MemoryProperties_.memoryTypes[ Idx ].propertyFlags & PropertyFlags ) == PropertyFlags )
			{
				MemoryAllocInfo.memoryTypeIndex = Idx;
				break;
			}
		}
		TypeFlags >>= 1;		
	}

	if( MemoryAllocInfo.memoryTypeIndex < VK_MAX_MEMORY_TYPES )
	{
		auto RetVal = vkAllocMemory( Device_, &MemoryAllocInfo, &Memory );
		BcAssert( !RetVal );
	}
	else
	{
		PSY_LOG( "Error: Unable to allocate %u bytes of device memory using type flags %u and property flags %u", 
			Size,
			TypeFlags,
			PropertyFlags );
	}
	
	return Memory;
}

//////////////////////////////////////////////////////////////////////////
// free
void RsAllocatorVK::free( VkDeviceMemory Memory )
{
	vkFreeMemory( Device_, Memory );
}
