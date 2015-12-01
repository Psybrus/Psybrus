#include "System/Renderer/RsProgramBinding.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramBindingDesc::RsProgramBindingDesc()
{
	UniformBuffers_.fill( nullptr );
	SamplerStates_.fill( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
void RsProgramBindingDesc::setUniformBuffer( BcU32 Slot, class RsBuffer* Buffer )
{
	if( Buffer )
	{
		BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE );
	}
	UniformBuffers_[ Slot ] = Buffer;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
void RsProgramBindingDesc::setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState )
{
	SamplerStates_[ Slot ] = SamplerState;
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
void RsProgramBindingDesc::setShaderResourceView( BcU32 Slot, class RsBuffer* Buffer )
{
	if( Buffer )
	{
		BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
		ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::BUFFER;
		ShaderResourceSlots_[ Slot ].Buffer_ = Buffer;
	}
	else
	{
		ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::INVALID;
		ShaderResourceSlots_[ Slot ].Resource_ = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
void RsProgramBindingDesc::setShaderResourceView( BcU32 Slot, class RsTexture* Texture )
{
	if( Texture )
	{
		BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
		ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::TEXTURE;
		ShaderResourceSlots_[ Slot ].Texture_ = Texture;
	}
	else
	{
		ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::INVALID;
		ShaderResourceSlots_[ Slot ].Resource_ = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
// setUnorderedAccessView
void RsProgramBindingDesc::setUnorderedAccessView( BcU32 Slot, class RsBuffer* Buffer )
{
	if( Buffer )
	{
		BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
		UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::BUFFER;
		UnorderedAccessSlots_[ Slot ].Buffer_ = Buffer;
	}
	else
	{
		UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::INVALID;
		UnorderedAccessSlots_[ Slot ].Resource_ = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
// setUnorderedAccessView
void RsProgramBindingDesc::setUnorderedAccessView( BcU32 Slot, class RsTexture* Texture )
{
	if( Texture )
	{
		BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
		UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::TEXTURE;
		UnorderedAccessSlots_[ Slot ].Texture_ = Texture;
	}
	else
	{
		UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::INVALID;
		UnorderedAccessSlots_[ Slot ].Resource_ = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramBinding::RsProgramBinding( RsContext* pContext, RsProgram* Program, const RsProgramBindingDesc & Desc ):
	RsResource( RsResourceType::PROGRAM_BINDING, pContext ),
	Program_( Program ),
	Desc_( Desc )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsProgramBinding::~RsProgramBinding()
{
}

//////////////////////////////////////////////////////////////////////////
// getProgram
RsProgram* RsProgramBinding::getProgram() const
{
	return Program_;
}

//////////////////////////////////////////////////////////////////////////
// getDesc
const RsProgramBindingDesc& RsProgramBinding::getDesc() const
{
	return Desc_;
}
