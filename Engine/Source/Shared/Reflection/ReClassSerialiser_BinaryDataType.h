#ifndef __REFLECTION_ClassSerialiser_BinaryDataType_H__
#define __REFLECTION_ClassSerialiser_BinaryDataType_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_BinaryDataType.
class ReClassSerialiser_BinaryDataType:
		public ReClassSerialiser
{
public:
	typedef std::string BaseType;

public:
    ReClassSerialiser_BinaryDataType( const std::string& Name );
    virtual ~ReClassSerialiser_BinaryDataType();
	void construct( void* pMemory ) const;
	void constructNoInit( void* pMemory ) const;
	void destruct( void* pMemory ) const;
	void* create() const;
	void* createNoInit() const;
	void destroy( void* Object ) const;
	BcU32 getBinaryDataSize( void* pMemory ) const;						
	BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const;
	BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const;
	BcBool serialiseToString( const void* pInstance, std::string& OutString ) const;
	BcBool serialiseFromString( void* pInstance, const std::string& InString ) const;
	BcBool copy( void* pDst, void* pSrc ) const;
};

#endif
