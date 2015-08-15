#include "System/Renderer/D3D12/RsLinearHeapAllocatorD3D12.h"
#include "Base/BcMath.h"
#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////
// RsLinearHeapAllocatorD3D12
RsLinearHeapAllocatorD3D12::RsLinearHeapAllocatorD3D12( 
		ID3D12Device* Device, D3D12_HEAP_TYPE HeapType, BcU64 MinResourceBlockSize ):
	Device_( Device ),
	HeapType_( HeapType ),
	MinResourceBlockSize_( MinResourceBlockSize ),
	OwningThread_( BcCurrentThreadId() ),
	Blocks_(),
	BlocksCreated_( 0 )
{
	BcAssert( MinResourceBlockSize_ >= MIN_RESOURCE_BLOCK_SIZE );
	BcAssert( ( MinResourceBlockSize_ & ( MAX_ALIGNMENT - 1 ) ) == 0 );
	BcAssert( HeapType_ == D3D12_HEAP_TYPE_UPLOAD || HeapType_ == D3D12_HEAP_TYPE_READBACK );
}

//////////////////////////////////////////////////////////////////////
// Dtor
RsLinearHeapAllocatorD3D12::~RsLinearHeapAllocatorD3D12()
{

}

//////////////////////////////////////////////////////////////////////
// Dtor
RsResourceAllocationD3D12 RsLinearHeapAllocatorD3D12::allocate( BcU64 Size, BcU64 Alignment )
{
	BcAssert( OwningThread_ == BcCurrentThreadId() );
	BcAssert( Alignment <= MAX_ALIGNMENT );

	RsResourceAllocationD3D12 Allocation = 
	{
		nullptr,
		0,
		nullptr,
		0
	};

	ResourceBlock* FoundBlock = nullptr;
	
	// Check if we have a valid block size.
	for( auto & Block : Blocks_ )
	{
		BcU64 AlignedOffset = BcPotRoundUp( Block.CurrentOffset_, Alignment );
		BcS64 Remaining = static_cast< BcS64 >( Block.Size_ - AlignedOffset );
		if( Remaining >= static_cast< BcS64 >( Size ) )
		{
			FoundBlock = &Block;
		}
	}
	
	// Allocate new block if need be.
	if( FoundBlock == nullptr )
	{
		auto NewBlock = createResourceBlock( Size );
		Blocks_.push_back( NewBlock );
		FoundBlock = &Blocks_.back();
	}

	// Grab the correct offset and assert remaining siz.e
	BcU64 AlignedOffset = BcPotRoundUp( FoundBlock->CurrentOffset_, Alignment );
	BcS64 Remaining = static_cast< BcS64 >( FoundBlock->Size_ - AlignedOffset );
	BcAssert( Remaining >= static_cast< BcS64 >( Size ) );

	// Setup allocation
	if( Remaining >= static_cast< BcS64 >( Size ) )
	{
		Allocation.BaseResource_ = FoundBlock->Resource_;
		Allocation.OffsetInBaseResource_ = AlignedOffset;
		Allocation.Address_ = static_cast< BcU8* >( FoundBlock->BaseAddress_ ) + AlignedOffset;
		Allocation.Size_ = Size;

		// Advance current offset.
		FoundBlock->CurrentOffset_ = AlignedOffset + Size;
		BcAssert( ( Allocation.OffsetInBaseResource_ & ( Alignment - 1 ) ) == 0 );
	}


	return std::move( Allocation );
}

//////////////////////////////////////////////////////////////////////
// reset
void RsLinearHeapAllocatorD3D12::reset()
{
	BcAssert( OwningThread_ == BcCurrentThreadId() );
	PSY_LOGSCOPEDCATEGORY( "RsLinearHeapAllocatorD3D12" );

	BcU64 TotalUsage = 0;
	BcU64 TotalSize = 0;
	
	// Set block offsets back to 0.
	for( auto& Block : Blocks_ )
	{
		TotalUsage += Block.CurrentOffset_;
		TotalSize += Block.Size_;
		Block.CurrentOffset_ = 0;
	}

	// If we have many blocks, merge into 1 and create as large as current total size.
	if( Blocks_.size() > 1 )
	{
		PSY_LOG( "More than 1 block allocated. Compacting." );
		PSY_LOGSCOPEDINDENT;
		PSY_LOG( "Total size: %u kB", TotalSize / 1024 );
		PSY_LOG( "Total usage this frame: %u kB", TotalUsage / 1024 );
		PSY_LOG( "Blocks: %u", Blocks_.size() );
		PSY_LOG( "Blocks created: %u", BlocksCreated_ );

		Blocks_.clear();
		auto Block = createResourceBlock( TotalSize );
		Blocks_.push_back( Block );
	}	
	
	// Reset stats.
	BlocksCreated_ = 0;
}

//////////////////////////////////////////////////////////////////////
// createResourceBlock
RsLinearHeapAllocatorD3D12::ResourceBlock RsLinearHeapAllocatorD3D12::createResourceBlock( BcU64 Size )
{
	// Minimum sized block, round up to max alignment.
	ResourceBlock Block = 
	{
		nullptr,
		nullptr,
		BcPotRoundUp( std::max( Size, MinResourceBlockSize_ ), MAX_ALIGNMENT ),
		0
	};

	// Setup heap properties.
	CD3DX12_HEAP_PROPERTIES HeapProperties( HeapType_ );
	CD3DX12_RESOURCE_DESC ResourceDesc( CD3DX12_RESOURCE_DESC::Buffer( Block.Size_, D3D12_RESOURCE_FLAG_NONE, MAX_ALIGNMENT ) );

	// Setup appropriate resource usage.
	D3D12_RESOURCE_STATES ResourceUsage = 
		( HeapType_ == D3D12_HEAP_TYPE_UPLOAD ) ? 
			D3D12_RESOURCE_STATE_GENERIC_READ : D3D12_RESOURCE_STATE_COPY_DEST;

	// Committed resource.
	HRESULT RetVal = Device_->CreateCommittedResource( 
		&HeapProperties, 
		D3D12_HEAP_FLAG_NONE, 
		&ResourceDesc,
		ResourceUsage,
		nullptr, IID_PPV_ARGS( Block.Resource_.GetAddressOf() ) );
	BcAssert( SUCCEEDED( RetVal ) );

	// Persistently map.
	Block.Resource_->Map( 0, nullptr, &Block.BaseAddress_ );

	// Blocks created.
	++BlocksCreated_;

	return std::move( Block );
}
