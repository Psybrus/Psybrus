#include "Reflection/ReClassSerialiser_BinaryDataType.h"
#include "Reflection/ReClass.h"
#include "Base/BcBinaryData.h"

extern "C"
{
	#include "b64/cencode.h"
	#include "b64/cdecode.h"
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ClassSerialiser_BinaryDataType::ClassSerialiser_BinaryDataType( const std::string& Name ):
	ClassSerialiser( Name )
{
	Class_->setType< BcBinaryData >( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ClassSerialiser_BinaryDataType::~ClassSerialiser_BinaryDataType()
{

}

//////////////////////////////////////////////////////////////////////////
// construct
void ClassSerialiser_BinaryDataType::construct( void* pMemory ) const
{
	new ( pMemory ) BcBinaryData();
}

//////////////////////////////////////////////////////////////////////////
// constructNoInit
void ClassSerialiser_BinaryDataType::constructNoInit( void* pMemory ) const
{
	new ( pMemory ) BcBinaryData();
}

//////////////////////////////////////////////////////////////////////////
// destruct
void ClassSerialiser_BinaryDataType::destruct( void* pMemory ) const
{
	reinterpret_cast< BcBinaryData* >( pMemory )->~BcBinaryData();
}

//////////////////////////////////////////////////////////////////////////
// getBinaryDataSize
BcU32 ClassSerialiser_BinaryDataType::getBinaryDataSize( void* pMemory ) const
{
	const BcBinaryData* pBinaryData( reinterpret_cast< const BcBinaryData* >( pMemory ) );
	return sizeof( BcU64 ) + pBinaryData->getDataSize();
}
						
//////////////////////////////////////////////////////////////////////////
// serialiseToBinary
BcBool ClassSerialiser_BinaryDataType::serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
{
	const BcBinaryData* pBinaryData( reinterpret_cast< const BcBinaryData* >( pInstance ) );
	Serialiser << BcU64( pBinaryData->getDataSize() );
	Serialiser.push( pBinaryData->getData< BcU8 >(), pBinaryData->getDataSize() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromBinary
BcBool ClassSerialiser_BinaryDataType::serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
{
	BcBinaryData* pBinaryData( reinterpret_cast< BcBinaryData* >( pInstance ) );
	BcU64 Length = 0;
	Serialiser >> Length;
	BcAssert( Length < std::numeric_limits< size_t >::max() );
	*pBinaryData = BcBinaryData( (size_t)Length );
	Serialiser.pop( pBinaryData->getData< BcU8 >(), pBinaryData->getDataSize() );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseToString
BcBool ClassSerialiser_BinaryDataType::serialiseToString( const void* pInstance, std::string& OutString ) const
{
	const BcBinaryData* pBinaryData( reinterpret_cast< const BcBinaryData* >( pInstance ) );
	BcU64 BytesRequired = ( pBinaryData->getDataSize() * 4 / 3 + 4 );
	BcAssert( BytesRequired < std::numeric_limits< size_t >::max() );
	OutString.resize( (size_t)BytesRequired + 1 );
	base64_encodestate EncodeState;
	base64_init_encodestate( &EncodeState );
	auto OutBytes = base64_encode_block( pBinaryData->getData< char >(), pBinaryData->getDataSize(), &OutString[ 0 ], 0, &EncodeState );
	base64_encode_blockend( &OutString[ OutBytes ], 0, &EncodeState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ClassSerialiser_BinaryDataType::serialiseFromString( void* pInstance, const std::string& InString ) const
{
	BcBinaryData* pBinaryData( reinterpret_cast< BcBinaryData* >( pInstance ) );
	BcU64 BytesRequired = ( InString.length() * 3 / 4 );
	BcAssert( BytesRequired < std::numeric_limits< size_t >::max() );
	*pBinaryData = BcBinaryData( (size_t)BytesRequired );
	Memory::Zero( pBinaryData->getData< BcU8 >(), pBinaryData->getDataSize() );
	base64_decodestate DecodeState;
	base64_init_decodestate( &DecodeState );
	auto InBytes = base64_decode_block( &InString[ 0 ], InString.length(), pBinaryData->getData< char >(), &DecodeState );
	return true;
}

//////////////////////////////////////////////////////////////////////////
// copy
BcBool ClassSerialiser_BinaryDataType::copy( void* pDst, void* pSrc ) const
{
	BcBinaryData& Dst = *reinterpret_cast< BcBinaryData* >( pDst );
	BcBinaryData& Src = *reinterpret_cast< BcBinaryData* >( pSrc );
	Dst = Src;
	return true;
}
