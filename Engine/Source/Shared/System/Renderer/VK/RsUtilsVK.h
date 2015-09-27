#pragma once

#include "System/Renderer/VK/RsVK.h"

//////////////////////////////////////////////////////////////////////////
// RsUtilsVK
class RsUtilsVK
{
public:
	static VkFormat GetTextureFormat( RsTextureFormat TextureFormat );
	static RsTextureFormat GetTextureFormat( VkFormat Format );
};


