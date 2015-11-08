#pragma once

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsResource.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramSRVDesc
struct RsProgramSRVDesc
{
};

//////////////////////////////////////////////////////////////////////////
// RsProgramUAVDesc
struct RsProgramUAVDesc
{
};

//////////////////////////////////////////////////////////////////////////
// RsProgramBindingDesc
class RsProgramBindingDesc
{
public:
	RsProgramBindingDesc();

	/**
	 * Set uniform buffer.
	 */
	void setUniformBuffer( BcU32 Slot, class RsBuffer* Buffer );

	/**
	 * Set sampler state.
	 */
	void setSamplerState( BcU32 Slot, class RsSamplerState* SamplerState );

	/**
	 * Set shader resource view to buffer.
 	 */
	void setShaderResourceView( BcU32 Slot, class RsBuffer* Buffer );

	/**
	 * Set shader resource view to texture.
 	 */
	void setShaderResourceView( BcU32 Slot, class RsTexture* Texture );

	/**
	 * Set unordered access view to buffer.
 	 */
	void setUnorderedAccessView( BcU32 Slot, class RsBuffer* Buffer );

	/**
	 * Set unordered access view to texture.
 	 */
	void setUnorderedAccessView( BcU32 Slot, class RsTexture* Texture );

	struct SRVSlot
	{
		RsShaderResourceType Type_ = RsShaderResourceType::INVALID;
		union
		{
			class RsResource* Resource_ = nullptr;
			class RsBuffer* Buffer_;
			class RsTexture* Texture_;
		};
	};

	struct UAVSlot
	{
		RsUnorderedAccessType Type_ = RsUnorderedAccessType::INVALID;
		union
		{
			class RsResource* Resource_ = nullptr;
			class RsBuffer* Buffer_;
			class RsTexture* Texture_;
		};
	};

	/// Slots.
	std::array< SRVSlot, 32 > ShaderResourceSlots_;
	std::array< UAVSlot, 32 > UnorderedAccessSlots_;
	std::array< class RsBuffer*, 32 > UniformBuffers_;
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

