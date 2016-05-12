#include "System/Renderer/D3D12/RsQueryHeapD3D12.h"
#include "System/Renderer/D3D12/RsContextD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"
#include "System/Renderer/RsQueryHeap.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsQueryHeapD3D12::RsQueryHeapD3D12( class RsQueryHeap* Parent, ID3D12Device* Device ):
	Parent_( Parent )
{
	Parent_->setHandle( this );
	const auto& QueryHeapDesc = Parent_->getDesc();

	D3D12_QUERY_HEAP_DESC Desc;
	Desc.Count = static_cast< UINT >( QueryHeapDesc.NoofQueries_ );
	Desc.Type = RsUtilsD3D12::GetQueryHeapType( QueryHeapDesc.QueryType_ );
	Desc.NodeMask = 1;

	HRESULT RetVal = Device->CreateQueryHeap( &Desc, IID_PPV_ARGS( QueryHeap_.GetAddressOf() ) );
	BcAssertMsg( SUCCEEDED( RetVal ), "Failed to create RsQueryHeap \"%s\", error code %x", 
		Parent->getDebugName(),
		RetVal );

	// Clear query end frames to the max frame.
	QueryEndFrames_.resize( QueryHeapDesc.NoofQueries_ );
	std::fill( QueryEndFrames_.begin(), QueryEndFrames_.end(), static_cast< BcU64 >( -1 ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsQueryHeapD3D12::~RsQueryHeapD3D12()
{

}
