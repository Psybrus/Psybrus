#include "System/Renderer/VK/RsAllocatorVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"
#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsAllocatorVK::RsAllocatorVK( VkPhysicalDevice PhysicalDevice, VkDevice Device ):
	PhysicalDevice_( PhysicalDevice ),
	Device_( Device )
{
	vkGetPhysicalDeviceMemoryProperties( PhysicalDevice_, &MemoryProperties_ );
}


//////////////////////////////////////////////////////////////////////////
// allocate
VkDeviceMemory RsAllocatorVK::allocate( size_t Size, size_t Alignment, uint32_t TypeFlags, VkMemoryPropertyFlagBits PropertyFlags )
{
	VkDeviceMemory Memory = 0;
	VkMemoryAllocateInfo MemoryAllocInfo = {};
	MemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	MemoryAllocInfo.pNext = nullptr;
	MemoryAllocInfo.allocationSize = BcPotRoundUp( Size, Alignment );
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
		VK( vkAllocateMemory( Device_, &MemoryAllocInfo, nullptr/*allocation*/, &Memory ) );
	}
	else
	{
		PSY_LOG( "Error: Unable to allocate %u bytes of device memory using type flags %u and property flags %u", 
			Size,
			TypeFlags,
			PropertyFlags );
	}
	
	PSY_LOG( "Allocated: %p, %x", Memory, Size );
	return Memory;
}

//////////////////////////////////////////////////////////////////////////
// allocate
VkDeviceMemory RsAllocatorVK::allocate( size_t Size, size_t Alignment, uint32_t TypeFlags, std::initializer_list< VkMemoryPropertyFlagBits > PropertyFlagsList )
{
	VkDeviceMemory Memory = nullptr;
	for( auto PropertyFlags : PropertyFlagsList )
	{
		if( Memory = allocate( Size, Alignment, TypeFlags, PropertyFlags ) )
		{
			break;
		}
	}
	return Memory;
}

//////////////////////////////////////////////////////////////////////////
// free
void RsAllocatorVK::free( VkDeviceMemory Memory )
{
	vkFreeMemory( Device_, Memory, nullptr/*allocation*/ );
}
