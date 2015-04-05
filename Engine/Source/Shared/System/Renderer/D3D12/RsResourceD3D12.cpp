#include "System/Renderer/D3D12/RsResourceD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"

#include "System/Renderer/RsBuffer.h"
#include "System/Renderer/RsTexture.h"

#include "Base/BcMath.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsResourceD3D12::RsResourceD3D12( 
		ID3D12Resource* Resource, 
		D3D12_RESOURCE_USAGE Usage, 
		D3D12_RESOURCE_USAGE InitialUsage ):
	Resource_( Resource ),
	Usage_( Usage ),
	CurrentUsage_( InitialUsage )
{
	BcAssert( ( Usage_ & CurrentUsage_ ) != 0 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsResourceD3D12::~RsResourceD3D12()
{
}

//////////////////////////////////////////////////////////////////////////
// getInternalResource
ComPtr< ID3D12Resource >& RsResourceD3D12::getInternalResource()
{
	return Resource_;
}
//////////////////////////////////////////////////////////////////////////
// getGPUVirtualAddress
D3D12_GPU_VIRTUAL_ADDRESS RsResourceD3D12::getGPUVirtualAddress()
{
	return Resource_->GetGPUVirtualAddress();
}

//////////////////////////////////////////////////////////////////////////
// resourceBarrierTransition
D3D12_RESOURCE_USAGE RsResourceD3D12::resourceBarrierTransition( ID3D12GraphicsCommandList* CommandList, D3D12_RESOURCE_USAGE Usage )
{
	BcAssert( ( Usage_ & Usage ) != 0 );
	auto OldUsage = CurrentUsage_;
	if( CurrentUsage_ != Usage )
	{
		D3D12_RESOURCE_BARRIER_DESC descBarrier = {};
		descBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		descBarrier.Transition.pResource = Resource_.Get();
		descBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		descBarrier.Transition.StateBefore = CurrentUsage_;
		descBarrier.Transition.StateAfter = Usage;

		CommandList->ResourceBarrier( 1, &descBarrier );
		CurrentUsage_ = Usage;
	}
	return OldUsage;
}

//////////////////////////////////////////////////////////////////////////
// resourceBarrierTransition
D3D12_RESOURCE_USAGE RsResourceD3D12::resourceUsage() const
{
	return CurrentUsage_;
}
