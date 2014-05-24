#include "Base/BcBinaryData.h"

BcBinaryData::BcBinaryData():
	pData_( nullptr ),
	DataSize_( 0 )
{

}
	
BcBinaryData::BcBinaryData( void* pData, size_t DataSize, BcBool Copy )
{
	BcAssertMsg( pData != nullptr, "If passing a pointer to data, it must not be null." );
	BcAssertMsg( ( DataSize & OWN_DATA_FLAG ) == 0, "Data size includes our signal flag to determine ownership. Can't use." );
	if( Copy == BcFalse )
	{
		pData_ = reinterpret_cast< BcU8* >( pData );
		DataSize_ = DataSize;
	}
	else
	{
		pData_ = reinterpret_cast< BcU8* >( BcMemAlign( DataSize, 16 ) );
		BcMemCopy( pData_, pData, DataSize );
		DataSize_ = DataSize | OWN_DATA_FLAG;
	}
}

BcBinaryData::BcBinaryData( size_t DataSize )
{
	pData_ = reinterpret_cast< BcU8* >( BcMemAlign( DataSize, 16 ) );
	DataSize_ = DataSize | OWN_DATA_FLAG;
}

BcBinaryData::BcBinaryData( const BcBinaryData& Other )
{
	pData_ = reinterpret_cast< BcU8* >( BcMemAlign( Other.getDataSize(), 16 ) );
	DataSize_ = Other.DataSize_ | OWN_DATA_FLAG;
	BcMemCopy( pData_, Other.pData_, getDataSize() );		
}

BcBinaryData::BcBinaryData( BcBinaryData&& Other )
{
	internalFree();
	pData_ = std::move( Other.pData_ );
	DataSize_ = std::move( Other.DataSize_ );
}

BcBinaryData::~BcBinaryData()
{
	internalFree();
}

BcBinaryData& BcBinaryData::operator = ( const BcBinaryData& Other )
{
	pData_ = reinterpret_cast< BcU8* >( BcMemAlign( Other.getDataSize(), 16 ) );
	DataSize_ = Other.DataSize_ | OWN_DATA_FLAG;
	BcMemCopy( pData_, Other.pData_, getDataSize() );		
	return *this;
}

BcBool BcBinaryData::operator == ( const BcBinaryData& Other ) const
{
	if( getDataSize() == Other.getDataSize() )
	{
		return BcMemCompare( getData< BcU8* >(), Other.getData< BcU8* >(), getDataSize() );
	}
	return BcFalse;
}

BcBool BcBinaryData::operator != ( const BcBinaryData& Other ) const
{
	if( getDataSize() == Other.getDataSize() )
	{
		return !BcMemCompare( getData< BcU8* >(), Other.getData< BcU8* >(), getDataSize() );
	}
	return BcTrue;
}

void BcBinaryData::internalFree()
{
	// Free if we have data.
	if( ( DataSize_ & OWN_DATA_FLAG ) != 0 )
	{
		BcMemFree( pData_ );
	}

	pData_ = nullptr;
	DataSize_ = 0;
}
