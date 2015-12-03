#ifndef __REFLECTION_CLASSSERIALISER_NAMETYPE_H__
#define __REFLECTION_CLASSSERIALISER_NAMETYPE_H__

#include "Reflection/ReClassSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser_NameType.
class ReClassSerialiser_NameType:
		public ReClassSerialiser
{
public:
	typedef BcName BaseType;

public:
    ReClassSerialiser_NameType( const std::string& Name );
    virtual ~ReClassSerialiser_NameType();
	void construct( void* pMemory ) const override;
	void constructNoInit( void* pMemory ) const override;
	void destruct( void* pMemory ) const override;
	void* create() const override;
	void* createNoInit() const override;
	void destroy( void* Object ) const override;
	size_t getBinaryDataSize( const void* pMemory ) const override;	
	BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const override;
	BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const override;
	BcBool serialiseToString( const void* pInstance, std::string& OutString ) const override;
	BcBool serialiseFromString( void* pInstance, const std::string& InString ) const override;
	BcBool copy( void* pDst, void* pSrc ) const override;

};

#endif
