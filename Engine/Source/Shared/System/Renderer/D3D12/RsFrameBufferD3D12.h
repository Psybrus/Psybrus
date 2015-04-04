#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsFrameBufferD3D12
class RsFrameBufferD3D12
{
public:
	RsFrameBufferD3D12( class RsFrameBuffer* Parent, ID3D12Device* Device );
	~RsFrameBufferD3D12();

private:
	class RsFrameBuffer* Parent_;
	ComPtr< ID3D12Device > Device_;
	ComPtr< ID3D12DescriptorHeap > RTV_;
	ComPtr< ID3D12DescriptorHeap > DSV_;
};
