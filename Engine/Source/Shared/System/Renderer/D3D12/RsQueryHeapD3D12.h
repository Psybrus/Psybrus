#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"

//////////////////////////////////////////////////////////////////////////
// RsQueryHeapGL
class RsQueryHeapD3D12
{
public:
	RsQueryHeapD3D12( class RsQueryHeap* Parent, ID3D12Device* Device );
	~RsQueryHeapD3D12();

	ID3D12QueryHeap* getQueryHeap() const { return QueryHeap_.Get(); }
	void setQueryEndFrame( size_t Idx, BcU64 Frame ) { QueryEndFrames_[ Idx ] = Frame; }
	BcU64 getQueryEndFrame( size_t Idx ) const { return QueryEndFrames_[ Idx ]; }

private:
	class RsQueryHeap* Parent_ = nullptr;
	ComPtr< ID3D12QueryHeap > QueryHeap_;
	std::vector< BcU64 > QueryEndFrames_;
};
