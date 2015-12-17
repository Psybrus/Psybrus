/**************************************************************************
*
* File:		RsVK.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Vulkan includes.
*		
*
*
* 
**************************************************************************/

#ifndef __RSVK_H__
#define __RSVK_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"

#include "System/Renderer/RsTypes.h"

#define VK_USE_PLATFORM_WIN32_KHR

#define VK_PROTOTYPES
#include <vulkan/vulkan.h>
#include <vulkan/vk_lunarg_debug_report.h>

#if PLATFORM_WINDOWS
#  include <Windows.h>
#  include <vulkan/vk_platform.h>
#  include <vulkan/vk_sdk_platform.h>
#endif

inline VkResult HandleVulkanResult( VkResult Result, const char* File, int Line, const char* CallString )
{
	if( Result != VK_SUCCESS )
	{
		PSY_LOG( "RsVK: %s:%u", File, Line );
		PSY_LOG( " - Call: %s\n", CallString );
	}
	switch( Result )
	{
	case VK_NOT_READY:
		PSY_LOG( " - Error: VK_NOT_READY" );
		break;
	case VK_TIMEOUT:
		PSY_LOG( " - Error: VK_TIMEOUT" );
		break;
	case VK_EVENT_SET:
		PSY_LOG( " - Error: VK_EVENT_SET" );
		break;
	case VK_EVENT_RESET:
		PSY_LOG( " - Error: VK_EVENT_RESET" );
		break;
	case VK_INCOMPLETE:
		PSY_LOG( " - Error: VK_INCOMPLETE" );
		break;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		PSY_LOG( " - Error: VK_ERROR_OUT_OF_HOST_MEMORY" );
		break;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		PSY_LOG( " - Error: VK_ERROR_OUT_OF_DEVICE_MEMORY" );
		break;
	case VK_ERROR_INITIALIZATION_FAILED:
		PSY_LOG( " - Error: VK_ERROR_INITIALIZATION_FAILED" );
		break;
	case VK_ERROR_DEVICE_LOST:
		PSY_LOG( " - Error: VK_ERROR_DEVICE_LOST" );
		break;
	case VK_ERROR_MEMORY_MAP_FAILED:
		PSY_LOG( " - Error: VK_ERROR_MEMORY_MAP_FAILED" );
		break;
	case VK_ERROR_LAYER_NOT_PRESENT:
		PSY_LOG( " - Error: VK_ERROR_LAYER_NOT_PRESENT" );
		break;
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		PSY_LOG( " - Error: VK_ERROR_EXTENSION_NOT_PRESENT" );
		break;
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		PSY_LOG( " - Error: VK_ERROR_INCOMPATIBLE_DRIVER" );
		break;
	}

	if( Result != VK_SUCCESS )
	{
		BcBreakpoint;
	}

	return Result;
}


#if !PSY_PRODUCTION
#define VK( _call ) HandleVulkanResult( _call, __FILE__, __LINE__, #_call )
#else
#define VK( _call ) _call
#endif

#endif

