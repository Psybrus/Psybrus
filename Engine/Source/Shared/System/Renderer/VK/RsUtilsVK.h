#pragma once

#include "System/Renderer/VK/RsVK.h"
#include "System/Renderer/RsVertexDeclaration.h"

//////////////////////////////////////////////////////////////////////////
// RsUtilsVK
class RsUtilsVK
{
public:
	static VkFormat GetTextureFormat( RsTextureFormat TextureFormat );
	static RsTextureFormat GetTextureFormat( VkFormat Format );
	static VkPrimitiveTopology GetPrimitiveTopology( RsTopologyType TopologyType );
	static VkShaderStage GetShaderStage( RsShaderType ShaderType );
	static VkFormat GetVertexElementFormat( RsVertexElement Element );
	static VkBufferUsageFlagBits GetBufferUsageFlags( RsResourceBindFlags BindFlags );
};


