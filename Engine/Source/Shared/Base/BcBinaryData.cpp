#include "Base/BcBinaryData.h"

BcBinaryData::BcBinaryData():
	pData_( nullptr ),
	DataSize_( 0 )
{

}
	
BcBinaryData::BcBinaryData( void* pData, size_t DataSize )
{
	BcAssertMsg( pData != nullptr, "If passing a pointer to data, it must not be null." );
	BcAssertMsg( ( DataSize & OWN_DATA_FLAG ) == 0, "Data size includes our signal flag to determine ownership. Can't use." );
	pData_ = reinterpret_cast< BcU8* >( pData );
	DataSize_ = DataSize;
}

BcBinaryData::BcBinaryData( size_t DataSize )
{
	pData_ = reinterpret_cast< BcU8* >( BcMemAlign( DataSize, 16 ) );
	DataSize_ = DataSize;
}

BcBinaryData::BcBinaryData( const BcBinaryData& Other )
{
	if( ( Other.DataSize_ & OWN_DATA_FLAG ) )
	{
		pData_ = reinterpret_cast< BcU8* >( BcMemAlign( Other.getDataSize(), 16 ) );
	}
	else
	{
		pData_ = Other.pData_;
	}

	DataSize_ = Other.DataSize_;
	BcMemCopy( pData_, Other.pData_, getDataSize() );		
}

BcBinaryData::BcBinaryData( BcBinaryData&& Other )
{
	pData_ = std::move( Other.pData_ );
	DataSize_ = std::move( Other.DataSize_ );
}

BcBinaryData::~BcBinaryData()
{
	internalFree();
}

void BcBinaryData::internalFree()
{
	// Free if we have data.
	if( ( DataSize_ & OWN_DATA_FLAG ) != 0 )
	{
		BcMemFree( pData_ );
	}
}
