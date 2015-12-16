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
	return;
	// Descriptor types.
	size_t CountIdx = 0;
	size_t TotalInfos = 0;
	VkDescriptorTypeCount TypeCounts[4];

	const size_t UniformBufferBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	TypeCounts[ CountIdx ].count = 16;
	CountIdx++;
	TotalInfos += 16;

	const size_t ImageSamplerBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	TypeCounts[ CountIdx ].count = 16;
	CountIdx++;
	TotalInfos += 16;

	const size_t StorageImageBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	TypeCounts[ CountIdx ].count = 16;
	CountIdx++;
	TotalInfos += 16;

	const size_t StorageBufferBase = TotalInfos;
	TypeCounts[ CountIdx ].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	TypeCounts[ CountIdx ].count = 16;
	CountIdx++;
	TotalInfos += 16;

	// Setup descriptor infos.
	BcU32 MaxImageSampler = 0;
	BcU32 MaxUniformBuffer = 0;
	BcU32 MaxStorageImage = 0;
	BcU32 MaxStorageBuffer = 0;
	std::vector< VkDescriptorInfo > DescInfos;
	DescInfos.resize( TotalInfos );
	memset( DescInfos.data(), 0, DescInfos.size() * sizeof( VkDescriptorInfo ) );

	// Setup uniform buffers.
	for( size_t Idx = 0; Idx < Desc.UniformBuffers_.size(); ++Idx )
	{
		auto UniformBuffer = Desc.UniformBuffers_[ Idx ];
		if( UniformBuffer )
		{
			auto BindInfo = ProgramVK->getUniformBufferBindInfo( Idx );
			auto& DescInfo = DescInfos[ UniformBufferBase + BindInfo.Binding_ ];
			auto BufferVK = UniformBuffer->getHandle< const RsBufferVK* >();
			DescInfo.bufferInfo.buffer = BufferVK->getBuffer();
			DescInfo.bufferInfo.offset = 0;
			DescInfo.bufferInfo.range = UniformBuffer->getDesc().SizeBytes_;

			MaxUniformBuffer = std::max( MaxUniformBuffer, BindInfo.Binding_ + 1 );
		}
	}

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

	// Update descriptor pool allocation.
	//CountIdx = 0;
	//TypeCounts[ CountIdx++ ].count = MaxUniformBuffer;
	//TypeCounts[ CountIdx++ ].count = MaxImageSampler;
	//TypeCounts[ CountIdx++ ].count = MaxStorageImage;
	//TypeCounts[ CountIdx++ ].count = MaxStorageBuffer;

	VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo = {};
	DescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	DescriptorPoolCreateInfo.pNext = nullptr;
	DescriptorPoolCreateInfo.poolUsage = VK_DESCRIPTOR_POOL_USAGE_ONE_SHOT;
	DescriptorPoolCreateInfo.maxSets = 1;
	DescriptorPoolCreateInfo.count = CountIdx;
	DescriptorPoolCreateInfo.pTypeCount = TypeCounts;
	VkResult RetVal;
	RetVal = VK( vkCreateDescriptorPool( Device_, &DescriptorPoolCreateInfo, &DescriptorPool_ ) );
	BcAssertMsg( RetVal == VK_SUCCESS, "Error creating descriptor pool for RsProgramBinding \"%s\"", 
		Parent_->getDebugName() );

	RetVal = vkAllocDescriptorSets( Device_, DescriptorPool_,
		VK_DESCRIPTOR_SET_USAGE_STATIC,
		1, &DescriptorSetLayout_,
		&DescriptorSet_ );
	BcAssertMsg( RetVal == VK_SUCCESS, "Error allocating descriptor set for RsProgramBinding \"%s\"", 
		Parent_->getDebugName() );

	std::array< VkWriteDescriptorSet, 4 > WriteSet;
	memset( WriteSet.data(), 0, sizeof( WriteSet ) );
	WriteSet[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	WriteSet[ 0 ].destSet = DescriptorSet_;
	WriteSet[ 0 ].destBinding = UniformBufferBase;
	WriteSet[ 0 ].count = MaxUniformBuffer;
	WriteSet[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	WriteSet[ 0 ].pDescriptors = DescInfos.data() + UniformBufferBase;

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

	// Do descriptor update.
	//VK( vkUpdateDescriptorSets( Device_, WriteSet.size(), WriteSet.data(), 0, nullptr ) );

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsProgramBindingVK::~RsProgramBindingVK()
{
	VK( vkFreeDescriptorSets( Device_, DescriptorPool_, 1, &DescriptorSet_ ) );
	vkDestroyDescriptorPool( Device_, DescriptorPool_ );
	Parent_->setHandle( nullptr );
}
