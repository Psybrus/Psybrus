#pragma once

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsSRVSlot
struct RsSRVSlot
{
	RsShaderResourceType Type_ = RsShaderResourceType::INVALID;
	union
	{
		class RsResource* Resource_ = nullptr;
		class RsBuffer* Buffer_;
		class RsTexture* Texture_;
	};

	BcU32 MostDetailedMip_FirstElement_ = 0;
	BcU32 MipLevels_NumElements_ = 0;
	BcU32 FirstArraySlice_ = 0;
	BcU32 ArraySize_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// RsUAVSlot
struct RsUAVSlot
{
	RsUnorderedAccessType Type_ = RsUnorderedAccessType::INVALID;
	union
	{
		class RsResource* Resource_ = nullptr;
		class RsBuffer* Buffer_;
		class RsTexture* Texture_;
	};

	BcU32 MipSlice_FirstElement_ = 0;
	BcU32 FirstArraySlice_NumElements_ = 0;
	BcU32 ArraySize_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// RsUBSlot
struct RsUBSlot
{
	class RsBuffer* Buffer_ = nullptr;
	BcU32 Offset_ = 0;
	BcU32 Size_ = 0;
};

//////////////////////////////////////////////////////////////////////////
// RsProgramBindingDesc
struct RsProgramBindingDesc
{
public:
	RsProgramBindingDesc();

	/**
	 * Set uniform buffer.
	 * @return true if slot has changed, false if not.
	 */
	bool setUniformBuffer( BcU32 Slot, class RsBuffer* Buffer, BcU32 Offset, BcU32 Size );

	/**
	 * Set sampler state.
	 * @return true if slot has changed, false if not.
	 */
	bool setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState );

	/**
	 * Set shader resource view to buffer.
	 * @param Slot Slot to set.
	 * @param Buffer Buffer to bind.
	 * @param FirstElement First element in buffer to bind.
	 * @param NumElements Number of elements to bind. If 0 will use @a Buffer Size/Stride.
	 * @return true if slot has changed, false if not.
 	 */
	bool setShaderResourceView( BcU32 Slot, class RsBuffer* Buffer,
			BcU32 FirstElement = 0, BcU32 NumElements = 0 );

	/**
	 * Set shader resource view to texture.
	 * @param Slot Slot to set.
	 * @param Texture Texture to bind.
	 * @param MostDetailedMip Most detailed mip to bind.
	 * @param MipLevels Number of mips to bind. If 0, will use max from @a Texture
	 * @param FirstArraySlice First element in texture array to bind.
	 * @param ArraySize Number of array elements to bind.
	 * @return true if slot has changed, false if not.
 	 */
	bool setShaderResourceView( BcU32 Slot, class RsTexture* Texture, BcU32 MostDetailedMip = 0, 
		BcU32 MipLevels = 0, BcU32 FirstArraySlice = 0, BcU32 ArraySize = 0 );

	/**
	 * Set unordered access view to buffer.
	 * @return true if slot has changed, false if not.
 	 */
	bool setUnorderedAccessView( BcU32 Slot, class RsBuffer* Buffer );

	/**
	 * Set unordered access view to texture.
	 * @param Slot Slot to set.
	 * @param Texture Texture to bind.
	 * @param MipSlice Which mip slice to bind.
	 * @param FirstArraySlice First element in texture array to bind.
	 * @param ArraySize Number of array elements to bind.
	 * @return true if slot has changed, false if not.
 	 */
	bool setUnorderedAccessView( BcU32 Slot, class RsTexture* Texture, BcU32 MipSlice = 0, 
		BcU32 FirstArraySlice = 0, BcU32 ArraySize = 0 );

	/// Slots.
	std::array< RsSRVSlot, 32 > ShaderResourceSlots_;
	std::array< RsUAVSlot, 32 > UnorderedAccessSlots_;
	std::array< RsUBSlot, 32 > UniformBuffers_;
	std::array< class RsSamplerState*, 32 > SamplerStates_;
};

////////////////////////////////////////////////////////////////////////////////
// RsProgramBinding
class RsProgramBinding:
	public RsResource
{
public:
	RsProgramBinding( class RsContext* pContext, class RsProgram* Program, const RsProgramBindingDesc & Desc );
	virtual ~RsProgramBinding();

	class RsProgram* getProgram() const;
	const RsProgramBindingDesc& getDesc() const;

private:
	class RsProgram* Program_;
	RsProgramBindingDesc Desc_;
};

