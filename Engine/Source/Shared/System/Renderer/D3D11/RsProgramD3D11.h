#pragma once

#include "System/Renderer/D3D11/RsD3D11.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramD3D11
class RsProgramD3D11
{
public:
	RsProgramD3D11( class RsProgram* Parent, ID3D11Device* Device );
	~RsProgramD3D11();

	BcU32 getSamplerSlot( RsShaderType Type, BcU32 Idx );
	BcU32 getSRVSlot( RsShaderType Type, BcU32 Idx );
	BcU32 getUAVSlot( RsShaderType Type, BcU32 Idx );
	BcU32 getCBSlot( RsShaderType Type, BcU32 Idx );

private:
	class RsProgram* Parent_;
	ComPtr< ID3D11Device > Device_;

	struct SlotMapping
	{
		BcU32 ShaderSlot[ (BcU32)RsShaderType::MAX ] = 
		{
			BcErrorCode,
			BcErrorCode,
			BcErrorCode,
			BcErrorCode,
			BcErrorCode,
			BcErrorCode 
		};
	};

	std::array< SlotMapping, 32 > SamplerSlots_;
	std::array< SlotMapping, 32 > SRVSlots_;
	std::array< SlotMapping, 32 > UAVSlots_;
	std::array< SlotMapping, 32 > CBSlots_;
};
