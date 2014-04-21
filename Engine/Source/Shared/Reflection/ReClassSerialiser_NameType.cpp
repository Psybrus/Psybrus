#include "Reflection/ReClassSerialiser_NameType.h"
#include "Reflection/ReClass.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ReClassSerialiser_NameType::ReClassSerialiser_NameType( const std::string& Name ):
	ReClassSerialiser( Name )
{
	Class_->setType< BcName >( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ReClassSerialiser_NameType::~ReClassSerialiser_NameType()
{

}

//////////////////////////////////////////////////////////////////////////
// construct
void ReClassSerialiser_NameType::construct( void* pMemory ) const
{
	new ( pMemory ) BaseType();
}

//////////////////////////////////////////////////////////////////////////
// constructNoInit
void ReClassSerialiser_NameType::constructNoInit( void* pMemory ) const
{
	new ( pMemory ) BaseType();
}

//////////////////////////////////////////////////////////////////////////
// destruct
void ReClassSerialiser_NameType::destruct( void* pMemory ) const
{
	reinterpret_cast< BaseType* >( pMemory )->~BaseType();
}

//////////////////////////////////////////////////////////////////////////
// getBinaryDataSize
BcU32 ReClassSerialiser_NameType::getBinaryDataSize( void* pMemory ) const
{
	return sizeof( char ) * ( ( **reinterpret_cast< BaseType* >( pMemory ) ).length() ) + sizeof( BcU32 );
}
						
//////////////////////////////////////////////////////////////////////////
// serialiseToBinary
BcBool ReClassSerialiser_NameType::serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
{
	std::string StringValue;
	const BaseType& Value( *reinterpret_cast< const BaseType* >( pInstance ) );
	StringValue = *Value;
	Serialiser << BcU32( StringValue.length() );
	for( size_t Idx = 0; Idx < StringValue.length(); ++Idx )
	{
		Serialiser << StringValue[ Idx ];
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromBinary
BcBool ReClassSerialiser_NameType::serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
{
	std::string StringValue;
	BaseType& Value( *reinterpret_cast< BaseType* >( pInstance ) );
	BcU32 Length = 0;
	Serialiser >> Length;
	StringValue.reserve( Length );
	for( size_t Idx = 0; Idx < Length; ++Idx )
	{
		char Char = 0;
		Serialiser >> Char;
		StringValue.push_back( Char );
	}
	Value = StringValue;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseToString
BcBool ReClassSerialiser_NameType::serialiseToString( const void* pInstance, std::string& OutString ) const
{
	const BaseType& Value( *reinterpret_cast< const BaseType* >( pInstance ) );
	OutString = *Value;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ReClassSerialiser_NameType::serialiseFromString( void* pInstance, const std::string& InString ) const
{
	BaseType& Value( *reinterpret_cast< BcName* >( pInstance ) );
	Value = InString;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// copy
BcBool ReClassSerialiser_NameType::copy( void* pDst, void* pSrc ) const
{
	BaseType& Dst = *reinterpret_cast< BaseType* >( pDst );
	BaseType& Src = *reinterpret_cast< BaseType* >( pSrc );
	Dst = Src;
	return true;
}
