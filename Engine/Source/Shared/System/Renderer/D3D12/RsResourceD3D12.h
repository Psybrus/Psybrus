#pragma once

#include "System/Renderer/D3D12/RsD3D12.h"
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// RsResourceD3D12
class RsResourceD3D12
{
public:
	RsResourceD3D12( ID3D12Resource* Resource, RsResourceBindFlags BindFlags, RsResourceBindFlags InitialBindType );
	~RsResourceD3D12();

	/**
	 * Get internal resource.
	 */
	ComPtr< ID3D12Resource >& getInternalResource();

	/**
	 * Transition resource from current usage to next usage.
	 * @pre @a BindType is valid (as constructed with).
	 * @pre @a BindType has only 1 bit set.
	 * @post Resource is valid for next usage type.
	 */
	void resourceBarrierTransition( ID3D12GraphicsCommandList* CommandList, RsResourceBindFlags BindType );

	/**
	 * TODO: Move to utils.
	 */
	static D3D12_RESOURCE_USAGE GetResourceUsage( RsResourceBindFlags BindFlags );

private:
	/// Internal D3D12 resource.
	ComPtr< ID3D12Resource > Resource_;

	/// Bind flags.
	RsResourceBindFlags BindFlags_;

	/// Current bind type on GPU.
	RsResourceBindFlags CurrentBindType_;
};

