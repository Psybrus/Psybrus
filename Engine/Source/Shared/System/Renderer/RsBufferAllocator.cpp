#include "System/Renderer/RsBufferAllocator.h"

#include "System/Renderer/RsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsBufferAllocator::RsBufferAllocator( RsBindFlags BindFlags, BcU32 Size, const char* DebugName ):
	BindFlags_( BindFlags ),
	Size_( Size )
{
	RsBufferDesc BufferDesc;
	BufferDesc.BindFlags_ = BindFlags;
	BufferDesc.Flags_ = RsResourceCreationFlags::STREAM;
	BufferDesc.SizeBytes_ = Size;
	BufferDesc.StructureStride_ = 0;
	for( auto& Buffer : Buffer_ )
	{
		Buffer = RsCore::pImpl()->createBuffer( BufferDesc, DebugName );
	}
}

//////////////////////////////////////////////////////////////////////////
// Dtor
RsBufferAllocator::~RsBufferAllocator()
{
	std::lock_guard< std::mutex > Lock( AllocMutex_ );

	for( auto& Buffer : Buffer_ )
	{
		Buffer.reset();
	}
}

//////////////////////////////////////////////////////////////////////////
// alloc
RsBufferAlloc RsBufferAllocator::alloc( BcU32 Size )
{
	RsBufferAlloc RetVal;
	std::lock_guard< std::mutex > Lock( AllocMutex_ );

	if( ( CurrentOffset_ + Size ) < Size_ )
	{
		RetVal.Buffer_ = Buffer_[ CurrentFrame_ ].get();
		RetVal.Offset_ = CurrentOffset_;
		RetVal.Size_ = Size;
		CurrentOffset_ += Size;
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// nextFrame
void RsBufferAllocator::nextFrame()
{
	std::lock_guard< std::mutex > Lock( AllocMutex_ );

	CurrentFrame_ = ( CurrentFrame_ + 1 ) % MAX_FRAMES_AHEAD;
	CurrentOffset_ = 0;
}
