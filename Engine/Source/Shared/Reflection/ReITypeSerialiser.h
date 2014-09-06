#ifndef __REFLECTION_ITYPESERIALISER_H__
#define __REFLECTION_ITYPESERIALISER_H__

#include "Reflection/ReManager.h"

//////////////////////////////////////////////////////////////////////////
// ITypeSerialiser
class ReITypeSerialiser
{
public:
    REFLECTION_DECLARE_BASE( ReITypeSerialiser );

public:
    ReITypeSerialiser(){};
    virtual ~ReITypeSerialiser(){};

	/**
		* @brief Construct.
		*/
	virtual void construct( void* ) const = 0;

	/**
		* @brief Construct no init
		*/
	virtual void constructNoInit( void* ) const = 0;

	/**
		* @brief Destruct.
		*/
	virtual void destruct( void* ) const = 0;

	/**
		* @brief Create.
		*/
	virtual void* create() const = 0;

	/**
		* @brief Create no init
		*/
	virtual void* createNoInit() const = 0;

	/**
		* @brief Destroy.
		*/
	virtual void destroy( void* ) const = 0;

	/**
		* @brief Get binary data size.
		* @param pInstance Object instance.
		*/
	virtual BcU32 getBinaryDataSize( const void* pInstance ) const = 0;

	/**
		* @brief Serialise to binary data.
		* @param pInstance Object instance.
		* @param Serialiser Binary data serialiser to use for writing.
		* @return Success or not.
		*/
	virtual BcBool serialiseToBinary( const void* pInstance, BcBinaryData::Stream& Serialiser ) const = 0;

	/**
		* @brief Binary data.
		* @param pInstance Object instance.
		* @param Serialiser Binary data serialiser to use for writing.
		* @return Success or not.
		*/
	virtual BcBool serialiseFromBinary( void* pInstance, const BcBinaryData::Stream& Serialiser ) const = 0;

	/**
		* @brief Serialise to std::string.
		* @param pInstance Instance to serialise.
		* @param OutString std::string to write out to.
		* @return true if it can serialise to a string, false if not.
		*/
	virtual BcBool serialiseToString( const void* pInstance, std::string& OutString ) const = 0;

	/**
		* @brief Serialise from std::string.
		* @param pInstance Instance to serialise.
		* @param InString std::string to read in from.
		* @return true if it can serialise from a string, false if not.
		*/
	virtual BcBool serialiseFromString( void* pInstance, const std::string& InString ) const = 0;

	/**
		* @brief Copy from one to another. Should be a deep copy.
		* @param pDst Destination memory location.
		* @param pSrc Source memory location.
		*/
	virtual BcBool copy( void* pDst, void* pSrc ) const = 0;
};

#endif // __REFLECTION_ITYPESERIALISER_H__
