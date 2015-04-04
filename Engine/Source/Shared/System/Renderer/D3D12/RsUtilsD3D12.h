#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsTextureFormatD3D12
struct RsTextureFormatD3D12
{
	RsTextureFormat TextureFormat_;
	DXGI_FORMAT RTVFormat_;
	DXGI_FORMAT DSVFormat_;
	DXGI_FORMAT SRVFormat_;
};

//////////////////////////////////////////////////////////////////////////
// RsUtilsD3D12
class RsUtilsD3D12
{
public:
	static const RsTextureFormatD3D12& GetTextureFormat( RsTextureFormat TextureFormat );

};
