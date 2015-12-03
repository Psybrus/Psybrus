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

	size_t getBinaryDataSize( const void* pInstance ) const override;
	BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const override;
	BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const override;
	BcBool serialiseToString( const void* pInstance, std::string& OutString ) const override;
	BcBool serialiseFromString( void* pInstance, const std::string& InString ) const override;
	BcBool copy( void* pDst, void* pSrc ) const override;

protected:
	ReClass*							Class_;
};


#endif
