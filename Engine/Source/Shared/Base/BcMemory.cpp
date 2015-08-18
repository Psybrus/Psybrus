/**************************************************************************
*
* File:		BcMemory.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "Base/BcMemory.h"
#include <atomic>
#include <mutex>


#if PLATFORM_WINDOWS
#include <windows.h>
#endif

#if PLATFORM_LINUX
#include <malloc.h>
#endif

//////////////////////////////////////////////////////////////////////////
// Ctor
BcMemoryAllocator::BcMemoryAllocator()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcMemoryAllocator::~BcMemoryAllocator()
{

}

//////////////////////////////////////////////////////////////////////////
//virtual
void* BcMemoryAllocator::malloc( BcSize Bytes, BcSize Alignment )
{
	return BcSysMemAlign( Bytes, Alignment );
}

//////////////////////////////////////////////////////////////////////////
// realloc
//virtual
void* BcMemoryAllocator::realloc( void* pMemory, BcSize Bytes, BcSize Alignment )
{
	BcSysMemRealloc( pMemory, Bytes, Alignment );
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// free
//virtual
void BcMemoryAllocator::free( void* pMemory )
{
	BcSysMemFree( pMemory );
}

//////////////////////////////////////////////////////////////////////////
// BcSysMemAlign
void* BcSysMemAlign( BcSize Bytes, BcSize Alignment )
{
#if PLATFORM_WINDOWS
	return _aligned_malloc( Bytes, Alignment );
#elif PLATFORM_OSX
	void* Mem = nullptr;
	BcVerifyMsg( posix_memalign( &Mem, Alignment, Bytes ) >= 0, "Failed to allocate with posix_memalign" );
	return Mem;
#else
	auto Mem = memalign( Alignment, Bytes );
	BcAssert( Mem );
	return Mem;
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcSysMemRealloc
void* BcSysMemRealloc( void* pMemory, BcSize Bytes, BcSize Alignment )
{
#if PLATFORM_WINDOWS
	return _aligned_realloc( pMemory, Bytes, Alignment );
#else
	BcBreakpoint;
	return nullptr;
#endif
}

//////////////////////////////////////////////////////////////////////////
// free
void BcSysMemFree( void* pMemory )
{
#if PLATFORM_WINDOWS
	_aligned_free( pMemory );
#else
	free( pMemory );
#endif
}



#if PLATFORM_OSX
#include <execinfo.h>

void printBacktrace()
{
	void* callstack[16];
	int i, frames = backtrace(callstack, 16);
	char** strs = backtrace_symbols(callstack, frames);
	for (i = 0; i < frames; ++i) {
		printf("%s\n", strs[i]);
	}
}
#endif

#if COMPILER_MSVC && 0
void* operator _concrt_new( size_t Size )
{
	void* pMem = BcMemAlign( Size, 16 );
	return pMem;
}

#endif

#if 0

void* operator new( size_t Size )
{
	void* pMem = BcMemAlign( Size, 16 );
	return pMem;
}

void* operator new[]( size_t Size )
{
	void* pMem = BcMemAlign( Size, 16 );
	return pMem;
}

void operator delete( void* pMem ) throw()
{
	BcMemFree( pMem );
}

void operator delete[]( void* pMem ) throw()
{
	BcMemFree( pMem );
}

#endif

//////////////////////////////////////////////////////////////////////////
// BcMemAlign
void* BcMemAlign( BcSize Bytes, BcSize Alignment )
{
	if( BcMemoryAllocator::pImpl() )
	{
		return BcMemoryAllocator::pImpl()->malloc( Bytes, Alignment );
	}
	else
	{
		// Go to system allocator.
		return BcSysMemAlign( Bytes, Alignment );
	}
}

//////////////////////////////////////////////////////////////////////////
// BcMemFree
void BcMemFree( void* pMemory )
{
	if( BcMemoryAllocator::pImpl() )
	{
		return BcMemoryAllocator::pImpl()->free( pMemory );
	}
	else
	{
		// Go to system allocator.
		BcSysMemFree( pMemory );
	}
}
