#pragma once

#include "System/Renderer/VK/RsVK.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramVK
class RsProgramVK
{
public:
	RsProgramVK( class RsProgram* Parent, VkDevice Device );
	~RsProgramVK();

	const std::vector< VkShaderModule >& getShaderModules() const { return ShaderModules_; }
	const std::vector< VkShader >& getShaders() const { return Shaders_; }

private:
	class RsProgram* Parent_;
	VkDevice Device_;

	std::vector< VkShaderModule > ShaderModules_;
	std::vector< VkShader > Shaders_;
};
