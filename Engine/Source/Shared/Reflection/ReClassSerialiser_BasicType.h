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

	void construct( void* pMemory ) const override
	{
		new ( pMemory ) BaseType();
	}

	void constructNoInit( void* pMemory ) const override
	{
		new ( pMemory ) BaseType();
	}

	void destruct( void* pMemory ) const override
	{
		reinterpret_cast< BaseType* >( pMemory )->~_Ty();
	}

	void* create() const override
	{
		return new BaseType();
	}
	
	void* createNoInit() const override
	{
		return new BaseType();
	}

	void destroy( void* Object ) const override
	{
		delete reinterpret_cast< BaseType* >( Object );
	}

	size_t getBinaryDataSize( const void* pMemory ) const override
	{
		return sizeof( BaseType );
	}

			
	BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const override
	{
		const BaseType* pValue( reinterpret_cast< const BaseType* >( pInstance ) );
		Serialiser << *pValue;
		return true;
	}

	BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const override
	{
		BaseType* pValue( reinterpret_cast< BaseType* >( pInstance ) );
		Serialiser >> *pValue;
		return true;
	}

	BcBool serialiseToString( const void* pInstance, std::string& OutString ) const override
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

	BcBool serialiseFromString( void* pInstance, const std::string& InString ) const override
	{
		BaseType* pValue( reinterpret_cast< BaseType* >( pInstance ) );
		CastType InValue;
		std::stringstream InStream( InString );

		InStream >> InValue;
		*pValue = static_cast< BaseType >( InValue );

		return true;
	}

	BcBool copy( void* pDst, void* pSrc ) const override
	{
		BaseType& Dst = *reinterpret_cast< BaseType* >( pDst );
		BaseType& Src = *reinterpret_cast< BaseType* >( pSrc );
		Dst = Src;
		return true;
	}
};

#endif
