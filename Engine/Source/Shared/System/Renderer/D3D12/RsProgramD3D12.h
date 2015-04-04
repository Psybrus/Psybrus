#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsProgramD3D12
class RsProgramD3D12
{
public:
	RsProgramD3D12( class RsProgram* Parent, ID3D12Device* Device );
	~RsProgramD3D12();

private:
	class RsProgram* Parent_;
	ComPtr< ID3D12Device > Device_;
	ComPtr< ID3D12DescriptorHeap > SRV_;
	ComPtr< ID3D12DescriptorHeap > Sampler_;

};
