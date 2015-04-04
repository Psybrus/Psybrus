#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsDescriptorHeapCacheD3D12
class RsDescriptorHeapCacheD3D12
{
public:
	RsDescriptorHeapCacheD3D12( ID3D12Device* Device );
	~RsDescriptorHeapCacheD3D12();

private:
	ComPtr< ID3D12Device > Device_;

};
