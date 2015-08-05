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

	void createRTVDescriptorHeap();
	void createDSVDescriptorHeap();
	void setupRTVs();
	void setupDSV();

	void clear( 
		ID3D12GraphicsCommandList* CommandList, 
		const RsColour& Colour,
		BcBool EnableClearColour,
		BcBool EnableClearDepth,
		BcBool EnableClearStencil );

	/**
	 * Set render targets and transition them to the correct usage.
	 */
	void setRenderTargets( ID3D12GraphicsCommandList* CommandList );

	/**
	 * Transition render targets to read usage.
	 */
	void transitionToRead( ID3D12GraphicsCommandList* CommandList );
	
private:
	class RsFrameBuffer* Parent_;
	ComPtr< ID3D12Device > Device_;
	BcU32 NumRTVs_;
	ComPtr< ID3D12DescriptorHeap > RTV_;
	ComPtr< ID3D12DescriptorHeap > DSV_;
};
