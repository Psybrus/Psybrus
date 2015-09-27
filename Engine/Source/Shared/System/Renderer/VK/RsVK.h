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

#define VK_PROTOTYPES
#include <vulkan.h>

#if PLATFORM_WINDOWS
#  include <Windows.h>
#  include <vk_wsi_swapchain.h>
#  include <vk_wsi_device_swapchain.h>
#endif

#endif

