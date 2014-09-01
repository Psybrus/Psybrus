#ifndef __REFLECTION_CLASSSERIALISER_BASICTYPE_H__
#define __REFLECTION_CLASSSERIALISER_BASICTYPE_H__

#include "Reflection/ReClassSerialiser.h"

#include <sstream>
#include <iomanip>

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_BasicType.
template < typename _Ty >
class ReClassSerialiser_BasicType:
		public ReClassSerialiser
{
public:
    typedef typename ReBaseTypeConversion< _Ty >::BaseType BaseType;
    typedef typename ReBaseTypeConversion< _Ty >::CastType CastType;
    static const int precision = ReBaseTypeConversion< _Ty >::precision;

public:
    ReClassSerialiser_BasicType( const std::string& Name ):
		ReClassSerialiser( Name )
	{
		Class_->setType< BaseType >( this );
	}

    virtual ~ReClassSerialiser_BasicType() {}

	void construct( void* pMemory ) const
	{
		new ( pMemory ) BaseType();
	}

	void constructNoInit( void* pMemory ) const
	{
		new ( pMemory ) BaseType();
	}

	void destruct( void* pMemory ) const
	{
		reinterpret_cast< BaseType* >( pMemory )->~_Ty();
	}

	void* create() const
	{
		return new BaseType();
	}
	
	void* createNoInit() const
	{
		return new BaseType();
	}

	void destroy( void* Object ) const
	{
		delete reinterpret_cast< BaseType* >( Object );
	}

	BcU32 getBinaryDataSize( void* pMemory ) const
	{
		return sizeof( BaseType );
	}

			
	BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
	{
		const BaseType* pValue( reinterpret_cast< const BaseType* >( pInstance ) );
		Serialiser << *pValue;
		return true;
	}

	BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
	{
		BaseType* pValue( reinterpret_cast< BaseType* >( pInstance ) );
		Serialiser >> *pValue;
		return true;
	}

	BcBool serialiseToString( const void* pInstance, std::string& OutString ) const
	{
		const BaseType* pValue( reinterpret_cast< const BaseType* >( pInstance ) );
		std::stringstream OutStream;

		OutStream << std::setprecision( precision );

		if( OutStream << static_cast< CastType >( *pValue ) )
		{
			OutString = OutStream.str();
		}

		return true;
	}

	BcBool serialiseFromString( void* pInstance, const std::string& InString ) const
	{
		BaseType* pValue( reinterpret_cast< BaseType* >( pInstance ) );
		CastType InValue;
		std::stringstream InStream( InString );

		InStream >> InValue;
		*pValue = static_cast< BaseType >( InValue );

		return true;
	}

	BcBool copy( void* pDst, void* pSrc ) const
	{
		BaseType& Dst = *reinterpret_cast< BaseType* >( pDst );
		BaseType& Src = *reinterpret_cast< BaseType* >( pSrc );
		Dst = Src;
		return true;
	}
};

#endif
