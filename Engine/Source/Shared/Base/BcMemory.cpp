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

#if PLATFORM_WINDOWS
#include <windows.h>
#endif

BcAtomicU32 gAllocID = 0;

void initHeap()
{
	static BcBool Initialised = BcFalse;
	if( Initialised == BcFalse )
	{
		Initialised = BcTrue;
		gAllocID = 0;
		
		
	}
}

void* operator new( size_t Size )
{
	initHeap();
	void* pMem = BcMemAlign( Size, 16 );
#ifdef PSY_DEBUG // neilogd: stamp 0x69 over uninitialised memory.
	BcMemSet( pMem, 0x69, Size );
#endif
#ifdef MEM_DEBUG
	
	BcU32 BreakID = -1;
	if( gAllocID == BreakID )
	{
		BcBreakpoint;
	}
	
	BcPrintf( "PsyNew: %p - %u", pMem, gAllocID++ );
	//printBacktrace();
#endif
	return pMem;
}

void* operator new[]( size_t Size)
{
	initHeap();
	void* pMem = BcMemAlign( Size, 16 );
#ifdef PSY_DEBUG // neilogd: stamp 0x69 over uninitialised memory.
	BcMemSet( pMem, 0x69, Size );
#endif
#ifdef MEM_DEBUG
	BcU32 BreakID = -1;
	if( gAllocID == BreakID )
	{
		BcBreakpoint;
	}

	BcPrintf( "PsyNew: %p - %u\n", pMem, gAllocID++ );
	printBacktrace();
#endif
	return pMem;
}

void operator delete( void* pMem ) throw()
{
#ifdef MEM_DEBUG
	BcPrintf( "PsyDelete: %p\n", pMem );
#endif
	BcMemFree( pMem );
}

void operator delete[]( void* pMem ) throw()
{
#ifdef MEM_DEBUG
	BcPrintf( "PsyDelete: %p\n", pMem );
#endif
	BcMemFree( pMem );
}

//////////////////////////////////////////////////////////////////////////
// BcMemAlign
void* BcMemAlign( BcSize Bytes, BcSize Alignment )
{
#if PLATFORM_WINDOWS
	return _aligned_malloc( Bytes, Alignment );
#else
	return memalign( Alignment, Bytes );
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcMemRealloc
void* BcMemRealloc( void* pOriginal, BcSize Bytes, BcSize Alignment )
{
#if PLATFORM_WINDOWS
	return _aligned_realloc( pOriginal, Bytes, Alignment );
#else
	BcBreakpoint; // Need to implement.
#endif
}

//////////////////////////////////////////////////////////////////////////
// BcMemFree
void BcMemFree( void* pMemory )
{
#if PLATFORM_WINDOWS
	_aligned_free( pMemory );
#else
	free( pMemory );
#endif
}
