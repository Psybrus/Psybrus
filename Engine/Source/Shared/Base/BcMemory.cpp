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

#include "BcMemory.h"
#include "BcAtomic.h"

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
	void* pMem = malloc( Size );
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
	void* pMem = malloc( Size );
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
	free( pMem );
}

void operator delete[]( void* pMem ) throw()
{
#ifdef MEM_DEBUG
	BcPrintf( "PsyDelete: %p\n", pMem );
#endif
	free( pMem );
}
