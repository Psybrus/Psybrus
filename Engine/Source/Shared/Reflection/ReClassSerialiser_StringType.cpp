#include "Reflection/ReClassSerialiser_StringType.h"
#include "Reflection/ReClass.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
ReClassSerialiser_StringType::ReClassSerialiser_StringType( const std::string& Name ):
	ReClassSerialiser( Name )
{
	Class_->setType< std::string >( this );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
ReClassSerialiser_StringType::~ReClassSerialiser_StringType()
{

}

//////////////////////////////////////////////////////////////////////////
// construct
void ReClassSerialiser_StringType::construct( void* pMemory ) const
{
	new ( pMemory ) BaseType();
}

//////////////////////////////////////////////////////////////////////////
// constructNoInit
void ReClassSerialiser_StringType::constructNoInit( void* pMemory ) const
{
	new ( pMemory ) BaseType();
}

//////////////////////////////////////////////////////////////////////////
// destruct
void ReClassSerialiser_StringType::destruct( void* pMemory ) const
{
	reinterpret_cast< BaseType* >( pMemory )->~BaseType();
}

//////////////////////////////////////////////////////////////////////////
// create
void* ReClassSerialiser_StringType::create() const
{
	return new BaseType();
}
	
//////////////////////////////////////////////////////////////////////////
// createNoInit
void* ReClassSerialiser_StringType::createNoInit() const
{
	return new BaseType();
}

//////////////////////////////////////////////////////////////////////////
// destroy
void ReClassSerialiser_StringType::destroy( void* Object ) const
{
	delete reinterpret_cast< BaseType* >( Object );
}

//////////////////////////////////////////////////////////////////////////
// getBinaryDataSize
size_t ReClassSerialiser_StringType::getBinaryDataSize( const void* pMemory ) const
{
	return sizeof( char ) * ( reinterpret_cast< const BaseType* >( pMemory )->length() ) + sizeof( BcU32 );
}
						
//////////////////////////////////////////////////////////////////////////
// serialiseToBinary
BcBool ReClassSerialiser_StringType::serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
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
BcBool ReClassSerialiser_StringType::serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
{
	BaseType& Value( *reinterpret_cast< BaseType* >( pInstance ) );
	BcU32 Length = 0;
	Serialiser >> Length;
	Value.reserve( Length );
	for( size_t Idx = 0; Idx < Length; ++Idx )
	{
		char Char = 0;
		Serialiser >> Char;
		Value.push_back( Char );
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseToString
BcBool ReClassSerialiser_StringType::serialiseToString( const void* pInstance, std::string& OutString ) const
{
	const BaseType& Value( *reinterpret_cast< const BaseType* >( pInstance ) );
	OutString = Value;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// serialiseFromString
BcBool ReClassSerialiser_StringType::serialiseFromString( void* pInstance, const std::string& InString ) const
{
	BaseType& Value( *reinterpret_cast< std::string* >( pInstance ) );
	Value = InString;
	return true;
}

//////////////////////////////////////////////////////////////////////////
// copy
BcBool ReClassSerialiser_StringType::copy( void* pDst, void* pSrc ) const
{
	BaseType& Dst = *reinterpret_cast< BaseType* >( pDst );
	BaseType& Src = *reinterpret_cast< BaseType* >( pSrc );
	Dst = Src;
	return true;
}
