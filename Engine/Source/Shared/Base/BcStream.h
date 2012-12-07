/**************************************************************************
*
* File:		BcStream.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Data stream.
*		
*
*
* 
**************************************************************************/

#ifndef __BCSTREAM_H__
#define __BCSTREAM_H__

#include "Base/BcTypes.h"

//////////////////////////////////////////////////////////////////////////
// BcStream
class BcStream
{
public:
	BcStream( BcBool bSwapEndian = BcFalse, BcSize AllocSize = 512, BcSize InitialSize = 0 );
	~BcStream();

	/**
	*	Create initial buffer.
	*/
	void create( BcBool bSwapEndian = BcFalse, BcSize AllocSize = 512, BcSize InitialSize = 0 );

	/**
	*	Free buffer.
	*/
	void free();

	/**
	*	Release buffer.
	*/
	BcU8* release( BcSize& Size );

	/**
	*	Reallocate.
	*/
	void realloc( BcSize NewSize );

	/**
	*	Buffer's full size.
	*/
	BcSize bufferSize();

	/**
	*	Buffer's data size.
	*/
	BcSize dataSize();
	
	/**
	*	Raw data access.
	*/
	BcU8* pData();

	/**
	*	Clear.
	*/
	void clear();

	/**
	*	Push data into buffer.
	*/
	BcSize push( const void* pData, BcSize nBytes );

	template< class _Ty >
	BcStream& operator << ( const _Ty& Data );

	BcStream& operator << ( BcU32 Data );
	BcStream& operator << ( BcU16 Data );
	BcStream& operator << ( BcU8 Data );
	BcStream& operator << ( BcStream* pStream );

private:

	BcBool bSwapEndian_;
	BcSize AllocSize_;
	BcSize BufferSize_;
	BcU8* pDataBuffer_;

	BcSize CurrentPosition_;
};

//////////////////////////////////////////////////////////////////////////
// Streaming operators
template< class _Ty >
inline BcStream& BcStream::operator << ( const _Ty& Data )
{
	push( &Data, sizeof( _Ty ) );
	return (*this);
}

inline BcStream& BcStream::operator << ( BcU32 Data )
{
	push( &Data, sizeof( BcU32 ) );
	return (*this);
}

inline BcStream& BcStream::operator << ( BcU16 Data )
{
	push( &Data, sizeof( BcU16 ) );
	return (*this);
}

inline BcStream& BcStream::operator << ( BcU8 Data )
{
	push( &Data, sizeof( BcU8 ) );
	return (*this);
}

inline BcStream& BcStream::operator << ( BcStream* pStream )
{
	push( pStream->pData(), pStream->dataSize() );
	return (*this);
}

inline BcSize BcStream::bufferSize()
{
	return BufferSize_;
}

inline BcSize BcStream::dataSize()
{
	return CurrentPosition_;
}

inline BcU8* BcStream::pData()
{
	return pDataBuffer_;
}


#endif
