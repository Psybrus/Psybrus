/**************************************************************************
*
* File:		RmParameters.cpp
* Author:	Neil Richardson 
* Ver/Date:	9/04/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Remote/RmParameters.h"
#include "Base/BcMemory.h"
#include "Base/BcHash.h"

////////////////////////////////////////////////////////////////////////////////
// Ctor
RmParameters::RmParameters( RmParameters::eType Type, void* pData, BcU32 Size )
{
	Type_ = Type;
	pBuffer_ = NULL;
	BufferSize_ = 0;
	Cursor_ = 0;

	switch( Type )
	{
		case TYPE_SEND:
			{
				pBuffer_ = new BcU8[ 1024 * 1024 ];
			}
			break;

		case TYPE_RECV:
			{
				BcAssert( pData != NULL && Size != 0 );
				pBuffer_ = (BcU8*)pData;
				BufferSize_ = Size;
			}
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Dtor
RmParameters::~RmParameters()
{
	if( Type_ == TYPE_SEND )
	{
		delete [] pBuffer_;
	}
}

////////////////////////////////////////////////////////////////////////////////
// push
void RmParameters::push( const void* pData, BcU32 Bytes )
{
	const BcSize DataEnd = Cursor_ + Bytes;
	if( DataEnd > BufferSize_ )
	{
		resize( DataEnd );
	}
	
	BcU8* pCurrentPos = &pBuffer_[ Cursor_ ];
	BcMemCopy( pCurrentPos, pData, Bytes );
	Cursor_ += Bytes;
}

////////////////////////////////////////////////////////////////////////////////
// pop
void RmParameters::pop( void* pData, BcU32 Bytes )
{
	BcAssert( ( Cursor_ + Bytes ) <= BufferSize_ );
	BcMemCopy( pData, &pBuffer_[ Cursor_ ], Bytes );
	Cursor_ += Bytes;
}

////////////////////////////////////////////////////////////////////////////////
// <<
RmParameters& RmParameters::operator << ( const std::string& Value )
{
	BcU32 Length = Value.length() + 1; // Include null terminator.
	push( &Length, sizeof( Length ) );
	push( Value.c_str(), Length );
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
// >>
RmParameters& RmParameters::operator >> ( std::string& Value )
{
	BcU32 Length;
	pop( &Length, sizeof( Length ) );
	BcChar* pBuffer = new BcChar[ Length ];
	pop( pBuffer, Length );
	Value = pBuffer;
	delete [] pBuffer;
	return (*this);
}

////////////////////////////////////////////////////////////////////////////////
// resize
void RmParameters::resize( BcU32 NewSize )
{
	BcAssert( NewSize > BufferSize_ );
	
	//
	if( pBuffer_ != NULL )
	{
		BcU8* pNewBuffer = new BcU8[ NewSize ];
		
		// Copy data to new buffer
		memcpy( pNewBuffer, pBuffer_, BufferSize_ );
		
		// Free old buffer
		delete [] pBuffer_;
		
		// Assign new buffer.
		pBuffer_ = pNewBuffer;
		BufferSize_ = NewSize;
	}
	else
	{
		// New buffer from scratch.
		pBuffer_ = new BcU8[ NewSize ];
		BufferSize_ = NewSize;
	}
}

////////////////////////////////////////////////////////////////////////////////
// getData
void* RmParameters::getData()
{
	return pBuffer_;
}

////////////////////////////////////////////////////////////////////////////////
// getDataSize
BcU32 RmParameters::getDataSize() const
{
	return BufferSize_;
}

////////////////////////////////////////////////////////////////////////////////
// getDataFromCursor
void* RmParameters::getDataFromCursor()
{
	return &pBuffer_[ Cursor_ ];
}

////////////////////////////////////////////////////////////////////////////////
// getDataSizeFromCursor
BcU32 RmParameters::getDataSizeFromCursor() const
{
	return BufferSize_ - Cursor_;
}


