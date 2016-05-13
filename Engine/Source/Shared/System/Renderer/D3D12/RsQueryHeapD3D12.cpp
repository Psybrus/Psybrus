#include "System/Renderer/D3D12/RsQueryHeapD3D12.h"
#include "System/Renderer/D3D12/RsContextD3D12.h"
#include "System/Renderer/D3D12/RsUtilsD3D12.h"
#include "System/Renderer/RsQueryHeap.h"

#include "Base/BcMath.h"

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

	// Allocate a readback buffer.
	// TODO: Need to use a buffer provided by the user.
	// Setup heap properties.
	size_t HeapSize = BcPotRoundUp( sizeof( BcU64 ) * QueryHeapDesc.NoofQueries_, 256 );
	CD3DX12_HEAP_PROPERTIES HeapProperties( D3D12_HEAP_TYPE_READBACK );
	CD3DX12_RESOURCE_DESC ResourceDesc( CD3DX12_RESOURCE_DESC::Buffer( HeapSize, D3D12_RESOURCE_FLAG_NONE, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT ) );

	// Setup appropriate resource usage.
	D3D12_RESOURCE_STATES ResourceUsage = D3D12_RESOURCE_STATE_COPY_DEST;

	// Committed resource.
	RetVal = Device->CreateCommittedResource( 
		&HeapProperties, 
		D3D12_HEAP_FLAG_NONE, 
		&ResourceDesc,
		ResourceUsage,
		nullptr, IID_PPV_ARGS( ReadbackBuffer_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Clear query end frames to the max frame.
	QueryEndFrames_.resize( QueryHeapDesc.NoofQueries_ );
	std::fill( QueryEndFrames_.begin(), QueryEndFrames_.end(), static_cast< BcU64 >( -1 ) );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsQueryHeapD3D12::~RsQueryHeapD3D12()
{

}
