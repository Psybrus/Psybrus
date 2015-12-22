#pragma once

#include "System/Renderer/VK/RsVK.h"
#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsVertexDeclaration.h"

//////////////////////////////////////////////////////////////////////////
// RsUtilsVK
class RsUtilsVK
{
public:
	static VkFormat GetTextureFormat( RsTextureFormat TextureFormat );
	static RsTextureFormat GetTextureFormat( VkFormat Format );
	static VkPrimitiveTopology GetPrimitiveTopology( RsTopologyType TopologyType );
	static VkShaderStageFlagBits GetShaderStage( RsShaderType ShaderType );
	static VkFormat GetVertexElementFormat( RsVertexElement Element );
	static VkBufferUsageFlagBits GetBufferUsageFlags( RsResourceBindFlags BindFlags );
	static VkBlendOp GetBlendOp( RsBlendOp Op );
	static VkBlendFactor GetBlendFactor( RsBlendType Type );
	static VkPolygonMode GetPolygonMode( RsFillMode Mode );
};


