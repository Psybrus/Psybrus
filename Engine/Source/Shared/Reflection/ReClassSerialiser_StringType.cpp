#include "Reflection/ReClassSerialiser_StringType.h"
#include "Reflection/ReClass.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ClassSerialiser_StringType::ClassSerialiser_StringType( const std::string& Name ):
	ClassSerialiser( Name )
{
	Class_->setType< std::string >( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ClassSerialiser_StringType::~ClassSerialiser_StringType()
{

}

//////////////////////////////////////////////////////////////////////////
// construct
void ClassSerialiser_StringType::construct( void* pMemory ) const
{
	new ( pMemory ) BaseType();
}

//////////////////////////////////////////////////////////////////////////
// constructNoInit
void ClassSerialiser_StringType::constructNoInit( void* pMemory ) const
{
	new ( pMemory ) BaseType();
}

//////////////////////////////////////////////////////////////////////////
// destruct
void ClassSerialiser_StringType::destruct( void* pMemory ) const
{
	reinterpret_cast< BaseType* >( pMemory )->~BaseType();
}

//////////////////////////////////////////////////////////////////////////
// getBinaryDataSize
BcU32 ClassSerialiser_StringType::getBinaryDataSize( void* pMemory ) const
{
	return sizeof( char ) * ( reinterpret_cast< BaseType* >( pMemory )->length() ) + sizeof( BcU32 );
}
						
//////////////////////////////////////////////////////////////////////////
// serialiseToBinary
BcBool ClassSerialiser_StringType::serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
{
	const BaseType& Value( *reinterpret_cast< const BaseType* >( pInstance ) );
	Serialiser << BcU32( Value.length() );
	for( size_t Idx = 0; Idx < Value.length(); ++Idx )
	{
		Serialiser << Value[ Idx ];
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromBinary
BcBool ClassSerialiser_StringType::serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
{
	BaseType& Value( *reinterpret_cast< BaseType* >( pInstance ) );
	BcU32 Length = 0;
	Serialiser >> Length;
	Value.reserve( Length );
	for( size_t Idx = 0; Idx < Value.length(); ++Idx )
	{
		char Char = 0;
		Serialiser >> Char;
		Value.push_back( Char );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseToString
BcBool ClassSerialiser_StringType::serialiseToString( const void* pInstance, std::string& OutString ) const
{
	const BaseType& Value( *reinterpret_cast< const BaseType* >( pInstance ) );
	OutString = Value;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ClassSerialiser_StringType::serialiseFromString( void* pInstance, const std::string& InString ) const
{
	BaseType& Value( *reinterpret_cast< std::string* >( pInstance ) );
	Value = InString;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// copy
BcBool ClassSerialiser_StringType::copy( void* pDst, void* pSrc ) const
{
	BaseType& Dst = *reinterpret_cast< BaseType* >( pDst );
	BaseType& Src = *reinterpret_cast< BaseType* >( pSrc );
	Dst = Src;
	return true;
}
