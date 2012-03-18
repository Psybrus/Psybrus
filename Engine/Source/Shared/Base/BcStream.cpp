/**************************************************************************
*
* File:		BcStream.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Data stream.
*		
*
*
* 
**************************************************************************/

#include "Base/BcStream.h"
#include "Base/BcDebug.h"
#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
BcStream::BcStream( BcBool bSwapEndian, BcSize AllocSize, BcSize InitialSize ):
	AllocSize_( 0 ),
	BufferSize_( 0 ),
	pDataBuffer_( NULL ),
	CurrentPosition_( 0 )
{
	create( bSwapEndian, AllocSize, InitialSize );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
BcStream::~BcStream()
{
	free();
}

//////////////////////////////////////////////////////////////////////////
// create
void BcStream::create( BcBool bSwapEndian, BcSize AllocSize, BcSize InitialSize )
{
	// Will automatically swap endianess on some datatypes.
	bSwapEndian_ = bSwapEndian;

	// TODO :P
	BcAssert( bSwapEndian_ == BcFalse );

	// Alloc size needs to be a power of 2.
	AllocSize_ = AllocSize;
	BufferSize_ = 0;
	CurrentPosition_ = 0;

	if( InitialSize != 0 )
	{
		realloc( InitialSize );
	}
}

//////////////////////////////////////////////////////////////////////////
// free
void BcStream::free()
{
	if( pDataBuffer_ != NULL )
	{
		delete [] pDataBuffer_;

		BufferSize_ = 0;
		pDataBuffer_ = NULL;
		CurrentPosition_ = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
// release
BcU8* BcStream::release( BcSize& Size )
{
	BcU8* pRetVal = pDataBuffer_;
	Size = CurrentPosition_;

	CurrentPosition_ = 0;
	BufferSize_ = 0;
	AllocSize_ = 0;
	pDataBuffer_ = NULL;

	return pRetVal;	
}

//////////////////////////////////////////////////////////////////////////
// realloc
void BcStream::realloc( BcSize NewSize )
{
	BcAssert( NewSize > BufferSize_ );

	// Round up size
	NewSize = ( ( (BcSize)( NewSize ) + AllocSize_ - 1 ) & ~( AllocSize_ - 1 ) );

	//
	if( pDataBuffer_ != NULL )
	{
		BcU8* pNewBuffer = new BcU8[ NewSize ];

		// Copy data to new buffer
		memcpy( pNewBuffer, pDataBuffer_, BufferSize_ );

		// Free old buffer
		delete [] pDataBuffer_;

		// Assign new buffer.
		pDataBuffer_ = pNewBuffer;
		BufferSize_ = NewSize;
	}
	else
	{
		// New buffer from scratch.
		pDataBuffer_ = new BcU8[ NewSize ];
		BufferSize_ = NewSize;
	}
}

//////////////////////////////////////////////////////////////////////////
// realloc
BcSize BcStream::push( const void* pData, BcSize nBytes )
{
	//
	BcSize CurrentPosition = CurrentPosition_; 

	//
	const BcSize DataEnd = CurrentPosition_ + nBytes;

	//
	if( DataEnd > BufferSize_ )
	{
		realloc( DataEnd );
	}

	BcU8* pCurrentPos = &pDataBuffer_[ CurrentPosition_ ];
	memcpy( pCurrentPos, pData, nBytes );
	CurrentPosition_ += nBytes;

	return CurrentPosition;
}
