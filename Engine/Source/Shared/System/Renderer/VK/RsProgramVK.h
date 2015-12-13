#pragma once

#include "System/Renderer/VK/RsVK.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramBindTypeVK
enum class RsProgramBindTypeVK
{
	NONE,
	IMAGE,
	BUFFER,
	UNIFORM,
	SAMPLER,
};

//////////////////////////////////////////////////////////////////////////
// RsProgramBindInfoVK
struct RsProgramBindInfoVK
{
	RsProgramBindInfoVK()
	{}
	
	RsProgramBindInfoVK( RsProgramBindTypeVK BindType, BcU32 Binding ):
		BindType_( BindType ),
		Binding_( Binding )
	{}

	RsProgramBindInfoVK( RsProgramBindTypeVK BindType, RsTextureType TextureType, BcU32 Binding ):
		BindType_( BindType ),
		TextureType_( TextureType ),
		Binding_( Binding )
	{}

	RsProgramBindTypeVK BindType_ = RsProgramBindTypeVK::NONE;
	RsTextureType TextureType_ = RsTextureType::UNKNOWN;
	BcU32 Binding_ = BcErrorCode;
};

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

	std::vector< RsProgramBindInfoVK > SRVBindInfo_;
	std::vector< RsProgramBindInfoVK > UAVBindInfo_;
	std::vector< RsProgramBindInfoVK > SamplerBindInfo_;
	std::vector< RsProgramBindInfoVK > UniformBufferBindInfo_;
};
