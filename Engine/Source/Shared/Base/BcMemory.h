/**************************************************************************
*
* File:		BcMemory.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __BCMEMORY_H__
#define __BCMEMORY_H__

#include "Base/BcDebug.h"

//#define MEM_DEBUG

//////////////////////////////////////////////////////////////////////////
// new/delete overloading.
void* operator new( size_t Size );
void* operator new[]( size_t Size );
void operator delete( void* pMem ) throw();
void operator delete[]( void* pMem ) throw();

//////////////////////////////////////////////////////////////////////////
// BcMemAlign
extern void* BcMemAlign( BcSize Bytes, BcSize Alignment );

//////////////////////////////////////////////////////////////////////////
// BcMemFree
extern void BcMemFree( void* pMemory );

//////////////////////////////////////////////////////////////////////////
// BcMemSet
inline void BcMemSet( void* pMemory, BcU8 Value, BcSize Bytes )
{
	memset( pMemory, Value, Bytes );
}

//////////////////////////////////////////////////////////////////////////
// BcMemCopy
inline void BcMemCopy( void* pDst, const void* pSrc, BcSize Bytes )
{
	memcpy( pDst, pSrc, Bytes );
}

//////////////////////////////////////////////////////////////////////////
// BcMemCompare
inline BcBool BcMemCompare( const void* pA, const void* pB, BcSize Bytes )
{
	int Result = memcmp( pA, pB, Bytes );
	return Result == 0;
}

//////////////////////////////////////////////////////////////////////////
// BcMemZero
inline void BcMemZero( void* pMemory, BcSize Bytes )
{
	BcMemSet( pMemory, 0, Bytes );
}

#endif


