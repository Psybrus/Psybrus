#include "System/Renderer/RsComputeBindings.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsComputeBindings::RsComputeBindings()
{
	UniformBuffers_.fill( nullptr );
	SamplerStates_.fill( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsComputeBindings::setUniformBuffer( BcU32 Slot, class RsBuffer* Buffer )
{
	BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE );
	UniformBuffers_[ Slot ] = Buffer;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsComputeBindings::setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState )
{
	SamplerStates_[ Slot ] = SamplerState;
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
void RsComputeBindings::setShaderResourceView( BcU32 Slot, class RsBuffer* Buffer )
{
	BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
	ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::BUFFER;
	ShaderResourceSlots_[ Slot ].Buffer_ = Buffer;
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
void RsComputeBindings::setShaderResourceView( BcU32 Slot, class RsTexture* Texture )
{
	BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
	ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::TEXTURE;
	ShaderResourceSlots_[ Slot ].Texture_ = Texture;
}

//////////////////////////////////////////////////////////////////////////
// setUnorderedAccessView
void RsComputeBindings::setUnorderedAccessView( BcU32 Slot, class RsBuffer* Buffer )
{
	BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
	UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::BUFFER;
	UnorderedAccessSlots_[ Slot ].Buffer_ = Buffer;
}

//////////////////////////////////////////////////////////////////////////
// setUnorderedAccessView
void RsComputeBindings::setUnorderedAccessView( BcU32 Slot, class RsTexture* Texture )
{
	BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
	UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::TEXTURE;
	UnorderedAccessSlots_[ Slot ].Texture_ = Texture;
}
