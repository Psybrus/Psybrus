#include "System/Renderer/VK/RsProgramVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsShader.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramVK::RsProgramVK( class RsProgram* Parent, VkDevice Device ):
	Parent_( Parent ),
	Device_( Device )
{
	Parent->setHandle( this );

#if 0
	// Sampler + SRV bindings.
	BcU32 SamplerIdx = 0;
	for( const auto& Parameter : Parent->getParameterList() )
	{
		if( Parameter.Type_ == RsProgramUniformType::SAMPLER_1D ||
			Parameter.Type_ == RsProgramUniformType::SAMPLER_2D ||
			Parameter.Type_ == RsProgramUniformType::SAMPLER_3D ||
			Parameter.Type_ == RsProgramUniformType::SAMPLER_CUBE ||
			Parameter.Type_ == RsProgramUniformType::SAMPLER_1D_SHADOW ||
			Parameter.Type_ == RsProgramUniformType::SAMPLER_2D_SHADOW )
		{
			// TEMPORARY HACK.
			std::string SamplerName = Uniform.Name_;
			if( SamplerName[0] == 's' )
			{
				SamplerName[0] = 'a';
			}
			Parent_->addShaderResource( 
				SamplerName,
				RsShaderResourceType::TEXTURE,
				SamplerIdx );
			Parent_->addSamplerSlot( 
				SamplerName,
				SamplerIdx );
			SamplerIdx++;
		}
	}

	// UB bindings.
	BcU32 UBIdx = 0;
	for( const auto& UniformBlock : Parent->getUniformBlockList() )
	{
		auto Class = ReManager::GetClass( UniformBlock.Name_ );
		BcAssert( Class->getSize() == (size_t)UniformBlock.Size_ );
		Parent_->addUniformBufferSlot( UniformBlock.Name_, UBIdx, Class );
	}
#endif

	// Create shader + shader module.
	VkShaderModuleCreateInfo ModuleCreateInfo;
	VkShaderCreateInfo ShaderCreateInfo;
	VkShaderModule ShaderModule;
	VkShader Shader;
	VkResult RetVal = VK_SUCCESS;

	ModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ModuleCreateInfo.pNext = NULL;

	ShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO;
	ShaderCreateInfo.pNext = NULL;
	ShaderCreateInfo.pName = "main";

	ShaderModules_.reserve( Parent_->getShaders().size() );
	Shaders_.reserve( Parent_->getShaders().size() );
	for( const auto& InShader : Parent_->getShaders() )
	{
		ModuleCreateInfo.codeSize = InShader->getDataSize();
		ModuleCreateInfo.pCode = InShader->getData();
		ModuleCreateInfo.flags = 0;
		RetVal = vkCreateShaderModule( Device_, &ModuleCreateInfo, &ShaderModule );
		BcAssert( !RetVal );

		ShaderCreateInfo.flags = 0;
		ShaderCreateInfo.module = ShaderModule;
		ShaderCreateInfo.pName = "main";
		ShaderCreateInfo.stage = RsUtilsVK::GetShaderStage( InShader->getDesc().ShaderType_ );
		RetVal = vkCreateShader( Device_, &ShaderCreateInfo, &Shader );
		BcAssert( !RetVal );

		ShaderModules_.emplace_back( ShaderModule );
		Shaders_.emplace_back( Shader );
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsProgramVK::~RsProgramVK()
{
	for( auto& ShaderModule : ShaderModules_ )
	{
		vkDestroyShaderModule( Device_, ShaderModule );
	}

	for( auto& Shader : Shaders_ )
	{
		vkDestroyShader( Device_, Shader );
	}

	Parent_->setHandle( 0 );
}
