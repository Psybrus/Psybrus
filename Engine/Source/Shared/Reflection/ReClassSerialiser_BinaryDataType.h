#ifndef __REFLECTION_ClassSerialiser_BinaryDataType_H__
#define __REFLECTION_ClassSerialiser_BinaryDataType_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_BinaryDataType.
class ClassSerialiser_BinaryDataType:
		public ClassSerialiser
{
public:
	typedef std::string BaseType;

public:
	ClassSerialiser_BinaryDataType( const std::string& Name );
	virtual ~ClassSerialiser_BinaryDataType();
	void construct( void* pMemory ) const;
	void constructNoInit( void* pMemory ) const;
	void destruct( void* pMemory ) const;
	BcU32 getBinaryDataSize( void* pMemory ) const;						
	BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const;
	BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const;
	BcBool serialiseToString( const void* pInstance, std::string& OutString ) const;
	BcBool serialiseFromString( void* pInstance, const std::string& InString ) const;
	BcBool copy( void* pDst, void* pSrc ) const;
};

#endif
