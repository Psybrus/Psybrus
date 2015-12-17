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
	
	size_t getSRVBindCount() { return SRVBindInfo_.size(); };
	size_t getUAVBindCount() { return UAVBindInfo_.size(); };
	size_t getSamplerBindCount() { return SamplerBindInfo_.size(); };
	size_t getUniformBufferBindCount() { return UniformBufferBindInfo_.size(); };

	RsProgramBindInfoVK getSRVBindInfo( BcU32 Idx ) const { return SRVBindInfo_[ Idx ]; };
	RsProgramBindInfoVK getUAVBindInfo( BcU32 Idx ) const { return UAVBindInfo_[ Idx ]; };
	RsProgramBindInfoVK getSamplerBindInfo( BcU32 Idx ) const { return SamplerBindInfo_[ Idx ]; };
	RsProgramBindInfoVK getUniformBufferBindInfo( BcU32 Idx ) const { return UniformBufferBindInfo_[ Idx ]; };

private:
	class RsProgram* Parent_;
	VkDevice Device_;

	std::vector< VkShaderModule > ShaderModules_;

	std::vector< RsProgramBindInfoVK > SRVBindInfo_;
	std::vector< RsProgramBindInfoVK > UAVBindInfo_;
	std::vector< RsProgramBindInfoVK > SamplerBindInfo_;
	std::vector< RsProgramBindInfoVK > UniformBufferBindInfo_;
};
