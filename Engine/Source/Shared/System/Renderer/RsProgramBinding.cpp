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
bool RsProgramBindingDesc::setUniformBuffer( BcU32 Slot, class RsBuffer* Buffer, size_t Offset, size_t Size )
{
	BcAssert( Size > 0 );
	BcAssert( ( Offset + Size ) <= Buffer->getDesc().SizeBytes_ );

	bool RetVal = false;
	if( Slot < UniformBuffers_.size() )
	{
		if( Buffer )
		{
			BcAssert( ( Buffer->getDesc().BindFlags_ & RsBindFlags::UNIFORM_BUFFER ) != RsBindFlags::NONE );
		}
		RetVal |= UniformBuffers_[ Slot ].Buffer_ != Buffer || UniformBuffers_[ Slot ].Offset_ != Offset || UniformBuffers_[ Slot ].Size_ != Size;
		UniformBuffers_[ Slot ].Buffer_ = Buffer;
		UniformBuffers_[ Slot ].Offset_ = Offset;
		UniformBuffers_[ Slot ].Size_ = Size;
	}
	else
	{
		PSY_LOGSCOPEDCATEGORY( RsProgramBindingDesc );
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
		PSY_LOGSCOPEDCATEGORY( RsProgramBindingDesc );
		PSY_LOG( "ERROR: setSamplerState being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
bool RsProgramBindingDesc::setShaderResourceView( BcU32 Slot, class RsBuffer* Buffer,
	BcU32 FirstElement, BcU32 NumElements )
{
	bool RetVal = false;
	if( Slot < ShaderResourceSlots_.size() )
	{
		if( Buffer )
		{
			BcAssert( ( Buffer->getDesc().BindFlags_ & RsBindFlags::SHADER_RESOURCE ) != RsBindFlags::NONE );
			RetVal |= ShaderResourceSlots_[ Slot ].Buffer_ != Buffer;
			ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::BUFFER;
			ShaderResourceSlots_[ Slot ].Buffer_ = Buffer;
			ShaderResourceSlots_[ Slot ].MostDetailedMip_FirstElement_ = FirstElement;
			ShaderResourceSlots_[ Slot ].MipLevels_NumElements_ = NumElements > 0 ? 
				NumElements : BcU32( Buffer->getDesc().SizeBytes_ ) / BcU32( Buffer->getDesc().StructureStride_ );
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
		PSY_LOGSCOPEDCATEGORY( RsProgramBindingDesc );
		PSY_LOG( "ERROR: setShaderResourceView being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setShaderResourceView
bool RsProgramBindingDesc::setShaderResourceView( BcU32 Slot, class RsTexture* Texture, 
	BcU32 MostDetailedMip, BcU32 MipLevels, BcU32 FirstArraySlice, BcU32 ArraySize )
{
	bool RetVal = false;
	if( Slot < ShaderResourceSlots_.size() )
	{
		if( Texture )
		{
			BcAssert( MostDetailedMip < Texture->getDesc().Levels_ );
			BcAssert( ( MostDetailedMip + MipLevels ) <= Texture->getDesc().Levels_ );
			BcAssert( ( Texture->getDesc().BindFlags_ & RsBindFlags::SHADER_RESOURCE ) != RsBindFlags::NONE );
			RetVal |= ShaderResourceSlots_[ Slot ].Texture_ != Texture;
			ShaderResourceSlots_[ Slot ].Type_ = RsShaderResourceType::TEXTURE;
			ShaderResourceSlots_[ Slot ].Texture_ = Texture;
			ShaderResourceSlots_[ Slot ].MostDetailedMip_FirstElement_ = MostDetailedMip;
			ShaderResourceSlots_[ Slot ].MipLevels_NumElements_ = MipLevels > 0 ? MipLevels : Texture->getDesc().Levels_;
			ShaderResourceSlots_[ Slot ].FirstArraySlice_ = FirstArraySlice;
			ShaderResourceSlots_[ Slot ].ArraySize_ = ArraySize;
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
		PSY_LOGSCOPEDCATEGORY( RsProgramBindingDesc );
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
			BcAssert( ( Buffer->getDesc().BindFlags_ & RsBindFlags::UNORDERED_ACCESS ) != RsBindFlags::NONE );
			RetVal |= ShaderResourceSlots_[ Slot ].Buffer_ != Buffer;
			UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::BUFFER;
			UnorderedAccessSlots_[ Slot ].Buffer_ = Buffer;

			// TODO!
			UnorderedAccessSlots_[ Slot ].MipSlice_FirstElement_ = 0;
			UnorderedAccessSlots_[ Slot ].FirstArraySlice_NumElements_ = BcU32( Buffer->getDesc().SizeBytes_ );
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
		PSY_LOGSCOPEDCATEGORY( RsProgramBindingDesc );
		PSY_LOG( "ERROR: setUnorderedAccessView being called with invalid slot." );
	}
	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// setUnorderedAccessView
bool RsProgramBindingDesc::setUnorderedAccessView( BcU32 Slot, class RsTexture* Texture, BcU32 MipSlice, BcU32 FirstArraySlice, BcU32 ArraySize )
{
	bool RetVal = false;
	if( Slot < UnorderedAccessSlots_.size() )
	{
		if( Texture )
		{
			BcAssert( ( Texture->getDesc().BindFlags_ & RsBindFlags::UNORDERED_ACCESS ) != RsBindFlags::NONE );
			RetVal |= UnorderedAccessSlots_[ Slot ].Texture_ != Texture;
			UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::TEXTURE;
			UnorderedAccessSlots_[ Slot ].Texture_ = Texture;
			UnorderedAccessSlots_[ Slot ].MipSlice_FirstElement_ = MipSlice;
			UnorderedAccessSlots_[ Slot ].FirstArraySlice_NumElements_ = FirstArraySlice;
			UnorderedAccessSlots_[ Slot ].ArraySize_ = ArraySize;
		}
		else
		{
			RetVal |= UnorderedAccessSlots_[ Slot ].Resource_ != nullptr;
			UnorderedAccessSlots_[ Slot ].Type_ = RsUnorderedAccessType::INVALID;
			UnorderedAccessSlots_[ Slot ].Buffer_ = nullptr;
		}
	}
	else
	{
		PSY_LOGSCOPEDCATEGORY( RsProgramBindingDesc );
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
