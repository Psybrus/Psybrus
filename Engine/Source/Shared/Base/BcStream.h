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
#include "Base/BcDebug.h"

//////////////////////////////////////////////////////////////////////////
// BcStream
class BcStream
{
private:
	BcStream( const BcStream& ) = delete;

public:
	template< typename _Ty >
	class Object
	{
	public:
		Object():
			Stream_( nullptr ),
			Offset_( 0 ),
			Elements_( 0 )
		{

		}

		Object( BcStream& Stream, size_t Offset, size_t Elements ):
			Stream_( &Stream ),
			Offset_( Offset ),
			Elements_( Elements )
		{

		}

		Object( const Object& Other ):
			Stream_( Other.Stream_ ),
			Offset_( Other.Offset_ ),
			Elements_( Other.Elements_ )
		{

		}

		_Ty* get()
		{
			BcAssert( Offset_ <= ( Stream_->dataSize() + sizeof( _Ty ) ) );
			return reinterpret_cast< _Ty* >( Stream_->pData() + Offset_ );
		}

		const _Ty* get() const
		{
			BcAssert( Offset_ <= ( Stream_->dataSize() + sizeof( _Ty ) ) );
			return reinterpret_cast< const _Ty* >( Stream_->pData() + Offset_ );
		}

		_Ty& operator * ()
		{
			return *get();
		}

		_Ty* operator -> ()
		{
			return get();
		}

		const _Ty* operator -> () const
		{
			return get();
		}

		_Ty& operator [] ( size_t Idx )
		{
			BcAssert( Idx < Elements_ );
			return get()[ Idx ];
		}

		const _Ty& operator [] ( size_t Idx ) const
		{
			BcAssert( Idx < Elements_ );
			return get()[ Idx ];
		}

	private:
		BcStream* Stream_;
		size_t Offset_;
		size_t Elements_;
	};

public:
	BcStream( BcBool bSwapEndian = BcFalse, BcSize AllocSize = 512, BcSize InitialSize = 0 );
	BcStream( BcStream&& Other );
	~BcStream();

	/**
	 * Create initial buffer.
	 */
	void create( BcBool bSwapEndian = BcFalse, BcSize AllocSize = 512, BcSize InitialSize = 0 );

	/**
	 * Free buffer.
	 */
	void free();

	/**
	 * Release buffer.
	 */
	BcU8* release( BcSize& Size );

	/**
	 * Reallocate.
	 */
	void realloc( BcSize NewSize );

	/**
	 * Buffer's full size.
	 */
	BcSize bufferSize();

	/**
	 * Buffer's data size.
	 */
	BcSize dataSize();

	/**
	 * Raw data access.
	 */
	BcU8* pData();

	/**
	 * Clear.
	 */
	void clear();

	/**
	 * Push data into buffer.
	 */
	BcSize push( const void* pData, BcSize nBytes );

	/**
	 * Alloc memory.
	 */
	BcU8* alloc( size_t Size );

	/**
	 * Allocate object.
	 * Construction will be performed, however destruction will NOT be performed on any objects.
	 */
	template< typename _Ty >
	Object< _Ty > alloc( size_t Elements = 1 )
	{
		size_t CurrentPosition = CurrentPosition_;
		_Ty* Data = reinterpret_cast< _Ty* >( alloc( sizeof( _Ty ) * Elements ) );
		for( size_t Idx = 0; Idx < Elements; ++Idx )
		{
			new ( Data + Idx ) _Ty();
		}
		return Object< _Ty >( *this, CurrentPosition, Elements );
	}

	/**
	 * Get object.
	 * Will return an object of where a pointer is located. 
	 */
	template< typename _Ty >
	Object< _Ty > get( _Ty* Ptr )
	{
		BcAssert( (BcU8*)Ptr >= pData() && (BcU8*)Ptr <= ( pData() + dataSize() - sizeof( _Ty ) ) );
		return Object< _Ty >( *this, (BcU8*)Ptr - pData(), 1 );
	}


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
