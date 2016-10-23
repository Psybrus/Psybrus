#pragma once

#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsUniquePointers.h"

//////////////////////////////////////////////////////////////////////////
// RsBufferAllocator
class RsBufferAllocator
{
public:
	RsBufferAllocator( RsBindFlags BindFlags, BcU32 Size, const char* DebugName );
	RsBufferAllocator( const RsBufferAllocator& ) = delete;
	~RsBufferAllocator();

	/**
	 *	Allocate.
	 *	This will allocate space in a buffer that will be valid for the frame.
	 *	@param Size Number of bytes to allocate.
	 */
	RsBufferAlloc alloc( BcU32 Size );

	/**
	 *	Advance to next frame.
	 */
	void nextFrame();


	RsBindFlags getBindFlags() const { return BindFlags_; }
	BcU32 getSize() const { return Size_; }

private:
	RsBindFlags BindFlags_;
	BcU32 Size_;
	std::mutex AllocMutex_;
	std::atomic< BcU32 > CurrentOffset_;
	std::array< RsBufferUPtr, MAX_FRAMES_AHEAD > Buffer_;
	size_t CurrentFrame_;
};
