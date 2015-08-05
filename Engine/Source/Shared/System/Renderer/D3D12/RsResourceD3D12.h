#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsResourceD3D12
class RsResourceD3D12
{
public:
	RsResourceD3D12( ID3D12Resource* Resource, D3D12_RESOURCE_STATES Usage, D3D12_RESOURCE_STATES InitialUsage );
	~RsResourceD3D12();

	/**
	 * Get internal resource.
	 */
	ComPtr< ID3D12Resource >& getInternalResource();

	/**
	 * Get GPU virtual address.
	 */
	D3D12_GPU_VIRTUAL_ADDRESS getGPUVirtualAddress();

	/**
	 * Transition resource from current usage to next usage.
	 * @pre @a Usage is valid (as constructed with).
	 * @post Resource is valid for next usage type.
	 * @return Old usage.
	 */
	D3D12_RESOURCE_STATES resourceBarrierTransition( ID3D12GraphicsCommandList* CommandList, D3D12_RESOURCE_STATES Usage );
	
	/**
	 * @return Current usage.
	 */
	D3D12_RESOURCE_STATES resourceUsage() const;

	/**
	 * TODO: Move to utils.
	 */
	static D3D12_RESOURCE_STATES GetResourceUsage( RsResourceBindFlags BindFlags );

private:
	/// Internal D3D12 resource.
	ComPtr< ID3D12Resource > Resource_;

	/// Usage.
	D3D12_RESOURCE_STATES Usage_;

	/// Current usage.
	D3D12_RESOURCE_STATES CurrentUsage_;
};

