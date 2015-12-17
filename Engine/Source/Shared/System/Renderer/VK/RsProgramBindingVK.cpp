#include "System/Renderer/VK/RsProgramBindingVK.h"
#include "System/Renderer/VK/RsBufferVK.h"
#include "System/Renderer/VK/RsProgramVK.h"
#include "System/Renderer/VK/RsTextureVK.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsProgram.h"
#include "System/Renderer/RsProgramBinding.h"
#include "System/Renderer/RsSamplerState.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramBindingVK::RsProgramBindingVK( class RsProgramBinding* Parent, VkDevice Device, VkDescriptorSetLayout DescriptorSetLayout ):
	Parent_( Parent ),
	Device_( Device ),
	DescriptorSetLayout_( DescriptorSetLayout )
{
	Parent_->setHandle( this );
	auto Desc = Parent->getDesc();
	auto Program = Parent->getProgram();
	auto ProgramVK = Program->getHandle< RsProgramVK* >();

	// Descriptor types.
	size_t CountIdx = 0;
	size_t TotalInfos = 0;
	VkDescriptorPoolSize TypeCounts[4];

	const size_t UniformBufferBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	TypeCounts[ CountIdx ].descriptorCount = 16;
	CountIdx++;
	TotalInfos += 16;

	const size_t ImageSamplerBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	TypeCounts[ CountIdx ].descriptorCount = 16;
	CountIdx++;
	TotalInfos += 16;

	const size_t StorageImageBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	TypeCounts[ CountIdx ].descriptorCount = 16;
	CountIdx++;
	TotalInfos += 16;

	const size_t StorageBufferBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	TypeCounts[ CountIdx ].descriptorCount = 16;
	CountIdx++;
	TotalInfos += 16;

	// Setup descriptor infos.
	BcU32 MaxImageSampler = 0;
	BcU32 MaxUniformBuffer = 0;
	BcU32 MaxStorageImage = 0;
	BcU32 MaxStorageBuffer = 0;
	std::vector< VkDescriptorBufferInfo > UniformBufferInfos;
	//std::vector< VkDescriptorInfo > DescInfos;
	UniformBufferInfos.resize( 16 );
	memset( UniformBufferInfos.data(), 0, UniformBufferInfos.size() * sizeof( VkDescriptorBufferInfo ) );

	// Setup uniform buffers.
	for( size_t Idx = 0; Idx < Desc.UniformBuffers_.size(); ++Idx )
	{
		auto UniformBuffer = Desc.UniformBuffers_[ Idx ];
		if( UniformBuffer )
		{
			auto BindInfo = ProgramVK->getUniformBufferBindInfo( Idx );
			auto& DescInfo = UniformBufferInfos[ BindInfo.Binding_ ];
			auto BufferVK = UniformBuffer->getHandle< const RsBufferVK* >();
			DescInfo.buffer = BufferVK->getBuffer();
			DescInfo.offset = 0;
			DescInfo.range = UniformBuffer->getDesc().SizeBytes_;

			MaxUniformBuffer = std::max( MaxUniformBuffer, BindInfo.Binding_ + 1 );
		}
	}
#if 0
	// Setup samplers.
	for( size_t Idx = 0; Idx < Desc.SamplerStates_.size(); ++Idx )
	{
		auto SamplerState = Desc.SamplerStates_[ Idx ];
		if( SamplerState )
		{
			auto BindInfo = ProgramVK->getSamplerBindInfo( Idx );
			auto& DescInfo = DescInfos[ ImageSamplerBase + BindInfo.Binding_ ];
			DescInfo.sampler = SamplerState->getHandle< VkSampler >();

			MaxImageSampler = std::max( MaxImageSampler, BindInfo.Binding_ + 1 );
		}
	}

	// Setup SRVs.
	for( size_t Idx = 0; Idx < Desc.ShaderResourceSlots_.size(); ++Idx )
	{
		auto ShaderResource = Desc.ShaderResourceSlots_[ Idx ];
		if( ShaderResource.Resource_ )
		{
			auto BindInfo = ProgramVK->getSRVBindInfo( Idx );
			switch( BindInfo.BindType_ )
			{
			case RsProgramBindTypeVK::SAMPLER:
				{
					auto& DescInfo = DescInfos[ ImageSamplerBase + BindInfo.Binding_ ];
					auto TextureVK = ShaderResource.Texture_->getHandle< const RsTextureVK* >();
					DescInfo.imageView = TextureVK->getImageView();
					DescInfo.imageLayout = TextureVK->getImageLayout();

					MaxImageSampler = std::max( MaxImageSampler, BindInfo.Binding_ + 1 );
				}
				break;
			case RsProgramBindTypeVK::IMAGE:
				{
					auto& DescInfo = DescInfos[ StorageImageBase + BindInfo.Binding_ ];
					auto TextureVK = ShaderResource.Texture_->getHandle< const RsTextureVK* >();
					DescInfo.imageView = TextureVK->getImageView();
					DescInfo.imageLayout = TextureVK->getImageLayout();

					MaxStorageImage = std::max( MaxStorageImage, BindInfo.Binding_ + 1 );
				}
				break;
			case RsProgramBindTypeVK::BUFFER:
				{
					auto& DescInfo = DescInfos[ StorageBufferBase + BindInfo.Binding_ ];
					auto BufferVK = ShaderResource.Buffer_->getHandle< const RsBufferVK* >();
					DescInfo.bufferView = BufferVK->getBufferView();
					DescInfo.bufferInfo.buffer = BufferVK->getBuffer();
					DescInfo.bufferInfo.offset = 0;
					DescInfo.bufferInfo.range = ShaderResource.Buffer_->getDesc().SizeBytes_;

					MaxStorageBuffer = std::max( MaxStorageBuffer, BindInfo.Binding_ + 1 );
				}
				break;
			default:
				BcAssertMsg( BcFalse, "Failure binding SRV %u in RsProgramBinding \"%s\"",
					Idx,
					Parent_->getDebugName() );
			}
		}
	}
	
	// Setup UAVs.
	for( size_t Idx = 0; Idx < Desc.UnorderedAccessSlots_.size(); ++Idx )
	{
		auto ShaderResource = Desc.UnorderedAccessSlots_[ Idx ];
		if( ShaderResource.Resource_ )
		{
			auto BindInfo = ProgramVK->getUAVBindInfo( Idx );
			switch( BindInfo.BindType_ )
			{
			case RsProgramBindTypeVK::SAMPLER:
				{
					auto& DescInfo = DescInfos[ ImageSamplerBase + BindInfo.Binding_ ];
					auto TextureVK = ShaderResource.Texture_->getHandle< const RsTextureVK* >();
					DescInfo.imageView = TextureVK->getImageView();
					DescInfo.imageLayout = TextureVK->getImageLayout();

					MaxImageSampler = std::max( MaxImageSampler, BindInfo.Binding_ + 1 );
				}
				break;
			case RsProgramBindTypeVK::IMAGE:
				{
					auto& DescInfo = DescInfos[ StorageImageBase + BindInfo.Binding_ ];
					auto TextureVK = ShaderResource.Texture_->getHandle< const RsTextureVK* >();
					DescInfo.imageView = TextureVK->getImageView();
					DescInfo.imageLayout = TextureVK->getImageLayout();

					MaxStorageImage = std::max( MaxStorageImage, BindInfo.Binding_ + 1 );
				}
				break;
			case RsProgramBindTypeVK::BUFFER:
				{
					auto& DescInfo = DescInfos[ StorageBufferBase + BindInfo.Binding_ ];
					auto BufferVK = ShaderResource.Buffer_->getHandle< const RsBufferVK* >();
					DescInfo.bufferView = BufferVK->getBufferView();
					DescInfo.bufferInfo.buffer = BufferVK->getBuffer();
					DescInfo.bufferInfo.offset = 0;
					DescInfo.bufferInfo.range = ShaderResource.Buffer_->getDesc().SizeBytes_;

					MaxStorageBuffer = std::max( MaxStorageBuffer, BindInfo.Binding_ + 1 );
				}
				break;
			default:
				BcAssertMsg( BcFalse, "Failure binding SRV %u in RsProgramBinding \"%s\"",
					Idx,
					Parent_->getDebugName() );
			}
		}
	}
#endif
	// Update descriptor pool allocation.
	//CountIdx = 0;
	//TypeCounts[ CountIdx++ ].count = MaxUniformBuffer;
	//TypeCounts[ CountIdx++ ].count = MaxImageSampler;
	//TypeCounts[ CountIdx++ ].count = MaxStorageImage;
	//TypeCounts[ CountIdx++ ].count = MaxStorageBuffer;

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {};
	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.pNext = nullptr;
	DescriptorPoolCreateInfo.maxSets = 1;
	DescriptorPoolCreateInfo.poolSizeCount = CountIdx;
	DescriptorPoolCreateInfo.pPoolSizes = TypeCounts;
	VkResult RetVal;
	RetVal = VK( vkCreateDescriptorPool( Device_, &DescriptorPoolCreateInfo, nullptr/*allocation*/, &DescriptorPool_ ) );
	BcAssertMsg( RetVal == VK_SUCCESS, "Error creating descriptor pool for RsProgramBinding \"%s\"", 
		Parent_->getDebugName() );

	VkDescriptorSetAllocateInfo DescriptorSetAllocInfo = {};
	DescriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	DescriptorSetAllocInfo.pNext = nullptr;
	DescriptorSetAllocInfo.descriptorPool = DescriptorPool_;
	DescriptorSetAllocInfo.setLayoutCount = 1;
	DescriptorSetAllocInfo.pSetLayouts = &DescriptorSetLayout_;
	RetVal = vkAllocateDescriptorSets( Device_, &DescriptorSetAllocInfo, &DescriptorSet_ );
	BcAssertMsg( RetVal == VK_SUCCESS, "Error allocating descriptor set for RsProgramBinding \"%s\"", 
		Parent_->getDebugName() );

	std::array< VkWriteDescriptorSet, 1 > WriteSet;
	memset( WriteSet.data(), 0, sizeof( WriteSet ) );
	WriteSet[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteSet[ 0 ].dstSet = DescriptorSet_;
	WriteSet[ 0 ].dstBinding = UniformBufferBase;
	WriteSet[ 0 ].descriptorCount = MaxUniformBuffer;
	WriteSet[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	WriteSet[ 0 ].pBufferInfo = UniformBufferInfos.data();
#if 0
	WriteSet[ 1 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteSet[ 1 ].destSet = DescriptorSet_;
	WriteSet[ 1 ].destBinding = ImageSamplerBase;
	WriteSet[ 1 ].count = MaxImageSampler;
	WriteSet[ 1 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	WriteSet[ 1 ].pDescriptors = DescInfos.data() + ImageSamplerBase;

	WriteSet[ 2 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteSet[ 2 ].destSet = DescriptorSet_;
	WriteSet[ 2 ].destBinding = StorageImageBase;
	WriteSet[ 2 ].count = MaxStorageImage;
	WriteSet[ 2 ].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	WriteSet[ 2 ].pDescriptors = DescInfos.data() + StorageImageBase;

	WriteSet[ 3 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteSet[ 3 ].destSet = DescriptorSet_;
	WriteSet[ 3 ].destBinding = StorageBufferBase;
	WriteSet[ 3 ].count = MaxStorageBuffer;
	WriteSet[ 3 ].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	WriteSet[ 3 ].pDescriptors = DescInfos.data() + StorageBufferBase;
#endif
	// Do descriptor update.
	vkUpdateDescriptorSets( Device_, WriteSet.size(), WriteSet.data(), 0, nullptr );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsProgramBindingVK::~RsProgramBindingVK()
{
	VK( vkFreeDescriptorSets( Device_, DescriptorPool_, 1, &DescriptorSet_ ) );
	vkDestroyDescriptorPool( Device_, DescriptorPool_, nullptr/*allocation*/ );
	Parent_->setHandle( nullptr );
}
