#ifndef __REFLECTION_CLASSSERIALISER_H__
#define __REFLECTION_CLASSSERIALISER_H__

#include "Reflection/ReITypeSerialiser.h"

//////////////////////////////////////////////////////////////////////////
// ClassSerialiser
class ReClassSerialiser:
    public ReITypeSerialiser
{
public:
    REFLECTION_DECLARE_DERIVED( ReClassSerialiser, ReITypeSerialiser );

public:
    ReClassSerialiser( BcName Name );
    virtual ~ReClassSerialiser();

	virtual size_t getBinaryDataSize( const void* pInstance ) const;
	virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const;
	virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const;
	virtual BcBool serialiseToString( const void* pInstance, std::string& OutString ) const;
	virtual BcBool serialiseFromString( void* pInstance, const std::string& InString ) const;
	virtual BcBool copy( void* pDst, void* pSrc ) const;

protected:
	ReClass*							Class_;
};


#endif
