#include "System/Renderer/VK/RsProgramVK.h"
#include "System/Renderer/VK/RsUtilsVK.h"
#include "System/Renderer/GL/RsGL.h"					// TEMP: For GL types.
#include "System/Renderer/GL/RsProgramFileDataGL.h"		// TEMP: For GL types.
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsShader.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramVK::RsProgramVK( class RsProgram* Parent, VkDevice Device ):
	Parent_( Parent ),
	Device_( Device )
{
	Parent->setHandle( this );

	for( const auto& Parameter : Parent->getParameterList() )
	{
		RsProgramParameterTypeValueGL InternalType;
		InternalType.Value_ = Parameter.InternalType_;
		switch( InternalType.Storage_ )
		{
		case RsProgramParameterStorageGL::UNIFORM_BLOCK:
			{
				auto Class = ReManager::GetClass( Parameter.Name_ );
				BcAssertMsg( Class->getSize() == (size_t)Parameter.Size_,
					"Size mismatch in RsProgram \"%s\". Uniform block \"%s\" is of size %u, expecting %u",
					Parent_->getDebugName(),
					Parameter.Name_,
					Parameter.Size_, 
					Class->getSize() );

				BcU32 UBSlot = UniformBufferBindInfo_.size();
				Parent_->addUniformBufferSlot( Parameter.Name_, UBSlot, Class );
				UniformBufferBindInfo_.emplace_back( RsProgramBindInfoVK( RsProgramBindTypeVK::UNIFORM, InternalType.Binding_ ) );
			}
			break;
		case RsProgramParameterStorageGL::SAMPLER:
			{
				RsTextureType TextureType = RsTextureType::UNKNOWN;
				switch( InternalType.Type_ )
				{
				case GL_SAMPLER_1D:
					TextureType = RsTextureType::TEX1D;
					break;
				case GL_SAMPLER_2D:
					TextureType = RsTextureType::TEX2D;
					break;
				case GL_SAMPLER_3D:
					TextureType = RsTextureType::TEX3D;
					break;
				case GL_SAMPLER_CUBE:
					TextureType = RsTextureType::TEXCUBE;
					break;
				case GL_SAMPLER_1D_SHADOW:
					TextureType = RsTextureType::TEX1D;
					break;
				case GL_SAMPLER_2D_SHADOW:
					TextureType = RsTextureType::TEX2D;
					break;
				default:
					BcAssertMsg( BcFalse, "Unsupported sampler type in program \"%s\"", 
						Parent->getDebugName() );
				}

				BcU32 SamplerSlot = SamplerBindInfo_.size();
				Parent_->addSamplerSlot( Parameter.Name_, SamplerSlot );
				SamplerBindInfo_.emplace_back( RsProgramBindInfoVK( RsProgramBindTypeVK::SAMPLER, TextureType, InternalType.Binding_ ) );

				BcU32 SRVSlot = SRVBindInfo_.size();
				Parent_->addShaderResource( Parameter.Name_, RsShaderResourceType::TEXTURE, SRVSlot );
				SRVBindInfo_.emplace_back( RsProgramBindInfoVK( RsProgramBindTypeVK::SAMPLER, TextureType, InternalType.Binding_ ) );
			}
			break;
		case RsProgramParameterStorageGL::SHADER_STORAGE_BUFFER:
			{
				if( InternalType.ReadOnly_ )
				{
					BcU32 SRVSlot = SRVBindInfo_.size();
					Parent_->addShaderResource( Parameter.Name_, RsShaderResourceType::BUFFER, SRVSlot );
					SRVBindInfo_.emplace_back( RsProgramBindInfoVK( RsProgramBindTypeVK::BUFFER, InternalType.Binding_ ) );
				}
				else
				{
					BcU32 UAVSlot = UAVBindInfo_.size();
					Parent_->addUnorderedAccess( Parameter.Name_, RsUnorderedAccessType::BUFFER, UAVSlot );
					UAVBindInfo_.emplace_back( RsProgramBindInfoVK( RsProgramBindTypeVK::BUFFER, InternalType.Binding_ ) );
				}
			}
			break;
		case RsProgramParameterStorageGL::IMAGE:
			{
				RsTextureType TextureType = RsTextureType::UNKNOWN;
				switch( InternalType.Type_ )
				{
				case GL_IMAGE_1D:
					TextureType = RsTextureType::TEX1D;
					break;
				case GL_IMAGE_2D:
					TextureType = RsTextureType::TEX2D;
					break;
				case GL_IMAGE_3D:
					TextureType = RsTextureType::TEX3D;
					break;
				case GL_IMAGE_2D_RECT:
					TextureType = RsTextureType::TEX2D;
					break;
				case GL_IMAGE_CUBE:
					TextureType = RsTextureType::TEXCUBE;
					break;
				case GL_IMAGE_1D_ARRAY:
					TextureType = RsTextureType::TEX1D;
					break;
				case GL_IMAGE_2D_ARRAY:
					TextureType = RsTextureType::TEX2D;
					break;
				case GL_IMAGE_CUBE_MAP_ARRAY:
					TextureType = RsTextureType::TEXCUBE;
					break;
				case GL_IMAGE_2D_MULTISAMPLE:
					TextureType = RsTextureType::TEX2D;
					break;
				case GL_IMAGE_2D_MULTISAMPLE_ARRAY:
					TextureType = RsTextureType::TEX2D;
					break;
				default:
					BcAssertMsg( BcFalse, "Unsupported image type in program \"%s\"", 
						Parent->getDebugName() );
				}

				if( InternalType.ReadOnly_ )
				{
					BcU32 SRVSlot = SRVBindInfo_.size();
					Parent->addShaderResource( Parameter.Name_, RsShaderResourceType::TEXTURE, SRVSlot );
					SRVBindInfo_.emplace_back( RsProgramBindInfoVK( RsProgramBindTypeVK::IMAGE, TextureType, InternalType.Binding_ ) );
				}
				else
				{
					BcU32 UAVSlot = UAVBindInfo_.size();
					Parent->addUnorderedAccess( Parameter.Name_, RsUnorderedAccessType::TEXTURE, UAVSlot );
					UAVBindInfo_.emplace_back( RsProgramBindInfoVK( RsProgramBindTypeVK::IMAGE, TextureType, InternalType.Binding_ ) );
				}
			}
			break;
		}
	}

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
		RetVal = VK( vkCreateShaderModule( Device_, &ModuleCreateInfo, &ShaderModule ) );
		BcAssert( !RetVal && ShaderModule );

		ShaderCreateInfo.flags = 0;
		ShaderCreateInfo.module = ShaderModule;
		ShaderCreateInfo.pName = "main";
		ShaderCreateInfo.stage = RsUtilsVK::GetShaderStage( InShader->getDesc().ShaderType_ );
		RetVal = VK( vkCreateShader( Device_, &ShaderCreateInfo, &Shader ) );
		BcAssert( !RetVal && Shader );

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
