#ifndef __REFLECTION_ClassSerialiser_EnumType_H__
#define __REFLECTION_ClassSerialiser_EnumType_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_EnumType.
class ReClassSerialiser_EnumType:
		public ReClassSerialiser
{
public:
    ReClassSerialiser_EnumType( BcName Name );
    virtual ~ReClassSerialiser_EnumType();
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
