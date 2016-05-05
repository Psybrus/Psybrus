#include "System/Renderer/RsProgramBinding.h"
#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsTexture.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsProgramBindingDesc::RsProgramBindingDesc()
{
	SamplerStates_.fill( nullptr );
}

//////////////////////////////////////////////////////////////////////////
// setUniformBuffer
bool RsProgramBindingDesc::setUniformBuffer( BcU32 Slot, class RsBuffer* Buffer, BcU32 Offset, BcU32 Size )
{
	BcAssert( Size > 0 );
	BcAssert( ( Offset + Size ) <= Buffer->getDesc().SizeBytes_ );

	bool RetVal = false;
	if( Slot < UniformBuffers_.size() )
	{
		if( Buffer )
		{
			BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::UNIFORM_BUFFER ) != RsResourceBindFlags::NONE );
		}
		RetVal |= UniformBuffers_[ Slot ].Buffer_ != Buffer || UniformBuffers_[ Slot ].Offset_ != Offset || UniformBuffers_[ Slot ].Size_ != Size;
		UniformBuffers_[ Slot ].Buffer_ = Buffer;
		UniformBuffers_[ Slot ].Offset_ = Offset;
		UniformBuffers_[ Slot ].Size_ = Size;
	}
	else
	{
		PSY_LOG( "ERROR: setUniformBuffer being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setSamplerState
bool RsProgramBindingDesc::setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState )
{
	bool RetVal = false;
	if( Slot < SamplerStates_.size() )
	{
		RetVal |= SamplerStates_[ Slot ] != SamplerState;
		SamplerStates_[ Slot ] = SamplerState;
	}
	else
	{
		PSY_LOG( "ERROR: setSamplerState being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
bool RsProgramBindingDesc::setShaderResourceView( BcU32 Slot, class RsBuffer* Buffer )
{
	bool RetVal = false;
	if( Slot < ShaderResourceSlots_.size() )
	{
		if( Buffer )
		{
			BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
			RetVal |= ShaderResourceSlots_[ Slot ].Buffer_ != Buffer;
			ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::BUFFER;
			ShaderResourceSlots_[ Slot ].Buffer_ = Buffer;
		}
		else
		{
			RetVal |= ShaderResourceSlots_[ Slot ].Resource_ != nullptr;
			ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::INVALID;
			ShaderResourceSlots_[ Slot ].Resource_ = nullptr;
		}
	}
	else
	{
		PSY_LOG( "ERROR: setShaderResourceView being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
bool RsProgramBindingDesc::setShaderResourceView( BcU32 Slot, class RsTexture* Texture )
{
	bool RetVal = false;
	if( Slot < ShaderResourceSlots_.size() )
	{
		if( Texture )
		{
			BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::SHADER_RESOURCE ) != RsResourceBindFlags::NONE );
			RetVal |= ShaderResourceSlots_[ Slot ].Texture_ != Texture;
			ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::TEXTURE;
			ShaderResourceSlots_[ Slot ].Texture_ = Texture;
		}
		else
		{
			RetVal |= ShaderResourceSlots_[ Slot ].Resource_ != nullptr;
			ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::INVALID;
			ShaderResourceSlots_[ Slot ].Resource_ = nullptr;
		}
	}
	else
	{
		PSY_LOG( "ERROR: setShaderResourceView being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setUnorderedAccessView
bool RsProgramBindingDesc::setUnorderedAccessView( BcU32 Slot, class RsBuffer* Buffer )
{
	bool RetVal = false;
	if( Slot < UnorderedAccessSlots_.size() )
	{
		if( Buffer )
		{
			BcAssert( ( Buffer->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
			RetVal |= ShaderResourceSlots_[ Slot ].Buffer_ != Buffer;
			UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::BUFFER;
			UnorderedAccessSlots_[ Slot ].Buffer_ = Buffer;
		}
		else
		{
			RetVal |= UnorderedAccessSlots_[ Slot ].Resource_ != nullptr;
			UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::INVALID;
			UnorderedAccessSlots_[ Slot ].Resource_ = nullptr;
		}
	}
	else
	{
		PSY_LOG( "ERROR: setUnorderedAccessView being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setUnorderedAccessView
bool RsProgramBindingDesc::setUnorderedAccessView( BcU32 Slot, class RsTexture* Texture )
{
	bool RetVal = false;
	if( Slot < UnorderedAccessSlots_.size() )
	{
		if( Texture )
		{
			BcAssert( ( Texture->getDesc().BindFlags_ & RsResourceBindFlags::UNORDERED_ACCESS ) != RsResourceBindFlags::NONE );
			RetVal |= UnorderedAccessSlots_[ Slot ].Texture_ != Texture;
			UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::TEXTURE;
			UnorderedAccessSlots_[ Slot ].Texture_ = Texture;
		}
		else
		{
			RetVal |= UnorderedAccessSlots_[ Slot ].Resource_ != nullptr;
			UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::INVALID;
			UnorderedAccessSlots_[ Slot ].Resource_ = nullptr;
		}
	}
	else
	{
		PSY_LOG( "ERROR: setUnorderedAccessView being called with invalid slot." );
	}
	return RetVal;
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
