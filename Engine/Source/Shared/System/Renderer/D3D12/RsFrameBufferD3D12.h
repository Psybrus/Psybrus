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

	/**
	 * Get format hash.
	 */
	BcU32 getFormatHash() const { return FormatHash_; }
	
private:
	class RsFrameBuffer* Parent_ = nullptr;
	ComPtr< ID3D12Device > Device_;
	BcU32 NumRTVs_ = 0;
	ComPtr< ID3D12DescriptorHeap > RTV_;
	ComPtr< ID3D12DescriptorHeap > DSV_;
	BcU32 FormatHash_ = 0;
};
