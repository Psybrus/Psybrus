#ifndef __REFLECTION_CLASSSERIALISER_COMPLEXTYPE_H__
#define __REFLECTION_CLASSSERIALISER_COMPLEXTYPE_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_ComplexType.
template < typename _Ty >
class ClassSerialiser_ComplexType:
		public ClassSerialiser_AbstractComplexType< _Ty >
{
public:
	ClassSerialiser_ComplexType( const std::string& Name ): ClassSerialiser_AbstractComplexType< _Ty >( Name ) {}
	virtual ~ClassSerialiser_ComplexType() {}

	void construct( void* pMemory ) const
	{
		new ( pMemory ) _Ty();
	}

	void constructNoInit( void* pMemory ) const
	{
		new ( pMemory ) _Ty( NOINIT );
	}

	void destruct( void* pMemory ) const
	{
		reinterpret_cast< _Ty* >( pMemory )->~_Ty();
	}

	BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const
	{
		return false;
	}

	BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const 
	{
		return false;
	}

	BcBool copy( void* pDst, void* pSrc ) const
	{
		return false;
	}
};

#endif
