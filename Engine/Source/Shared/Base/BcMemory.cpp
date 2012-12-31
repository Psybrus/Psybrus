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
#include "Base/BcAtomic.h"
#include "Base/BcMutex.h"
#include "Base/BcScopedLock.h"

#if PLATFORM_WINDOWS
#include <windows.h>
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
#else
	return memalign( Alignment, Bytes );
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
	return memalign( Alignment, Bytes );
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
