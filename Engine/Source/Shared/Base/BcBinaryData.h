#ifndef __BINARYDATA_H__
#define __BINARYDATA_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcMemory.h"

//////////////////////////////////////////////////////////////////////////
// BcBinaryData
class BcBinaryData
{
public:
	static const size_t OWN_DATA_FLAG = (size_t)1 << (size_t)( ( sizeof( size_t ) * 8 ) - 1 ); // MSB of the data size can be used for flagging if we own the data.

public:
	class Stream
	{
	public:
		Stream( BcBinaryData& BcBinaryData, size_t CurrOffset = 0 ):
			BinaryData_( BcBinaryData ),
			CurrOffset_( CurrOffset )
		{
		}

		Stream( const Stream& Other ):
			BinaryData_( Other.BinaryData_ ),
			CurrOffset_( Other.CurrOffset_ )
		{
		}

		Stream& operator = ( const Stream& Other )
		{
			BinaryData_ = Other.BinaryData_;
			CurrOffset_ = Other.CurrOffset_;
			return *this;
		}

		inline void setOffset( size_t Offset )
		{
			BcAssertMsg( Offset < BinaryData_.getDataSize(), "Offset too large." );
			CurrOffset_ = Offset;
		}

		inline size_t getOffset() const
		{
			return CurrOffset_;
		}

		inline void push( const void* pData, size_t DataSize )
		{
			BcAssertMsg( CurrOffset_ + DataSize <= BinaryData_.getDataSize(), "Overflow detected." );
			BcMemCopy( BinaryData_.getData< BcU8 >( CurrOffset_ ), pData, DataSize );
			CurrOffset_ += DataSize;
		}

		inline void pop( void* pData, size_t DataSize ) const
		{
			BcAssertMsg( CurrOffset_ + DataSize <= BinaryData_.getDataSize(), "Overflow detected." );
			BcMemCopy( pData, BinaryData_.getData< BcU8 >( CurrOffset_ ), DataSize );
			CurrOffset_ += DataSize;
		}

		template < typename _Ty >
		inline Stream& operator << ( const _Ty& pData )
		{
			push( &pData, sizeof( _Ty ) );
			return *this;
		}

		template < typename _Ty >
		inline const Stream& operator >> ( _Ty& pData ) const
		{
			pop( &pData, sizeof( _Ty ) );
			return *this;
		}

	private:
		BcBinaryData& BinaryData_;
		mutable size_t CurrOffset_;
	};

public:
	BcBinaryData();
	BcBinaryData( void* pData, size_t DataSize, BcBool Copy = BcFalse );
	BcBinaryData( size_t DataSize );
	BcBinaryData( const BcBinaryData& Other );
	BcBinaryData( BcBinaryData&& Other );
	~BcBinaryData();

	BcBinaryData& operator = ( const BcBinaryData& Other );

	/**
	 * Compare data.
	 */
	BcBool operator == ( const BcBinaryData& Other ) const;

	/**
	 * Compare data.
	 */
	BcBool operator != ( const BcBinaryData& Other ) const;

	/**
		* @brief Create a stream.
		* @param Offset Offset to start it at.
		* @return Stream.
		*/
	Stream stream( size_t Offset = 0 );
		
	/**
		* @brief Get data at offset for type.
		* @param _Ty Type
		* @param ByteOffset Offset in bytes.
		* @return Pointer to data.
		*/
	template< typename _Ty >
	inline _Ty* getData( size_t ByteOffset = 0 )
	{
		BcAssertMsg( ( ByteOffset + sizeof( _Ty ) ) < DataSize_, "Overflow." );
		return reinterpret_cast< _Ty* >( &pData_[ ByteOffset ] );
	}

	/**
		* @brief Get data at offset for type.
		* @param _Ty Type
		* @param ByteOffset Offset in bytes.
		* @return Pointer to data.
		*/
	template< typename _Ty >
	inline const _Ty* getData( size_t ByteOffset = 0 ) const
	{
		BcAssertMsg( ( ByteOffset + sizeof( _Ty ) ) < DataSize_, "Overflow." );
		return reinterpret_cast< const _Ty* >( &pData_[ ByteOffset ] );
	}

	/**
		* @brief Get data size.
		* @return Data size in bytes.
		*/
	size_t						getDataSize() const;

	/**
		* @brief Have we got data?
		* @return True if yes.
		*/
	bool						haveData() const;

private:
	void						internalFree();
				
private:
	BcU8*						pData_;
	size_t						DataSize_;
};

inline BcBinaryData::Stream BcBinaryData::stream( size_t Offset )
{
	return BcBinaryData::Stream( *this, Offset );
}

inline size_t BcBinaryData::getDataSize() const
{
	return DataSize_ & ~OWN_DATA_FLAG;
}

inline bool BcBinaryData::haveData() const
{
	return pData_ != nullptr;
}

#endif // __BINARYDATA_H__
