/**************************************************************************
*
* File:		BcHash.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		Hash key object.
*		
*
*
* 
**************************************************************************/

#ifndef __BCHASH_H__
#define __BCHASH_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcString.h"

//////////////////////////////////////////////////////////////////////////
// Hash types
#define HASH_CRC32			0
#define HASH_SDBM			1
#define HASH_DJB			2
#define HASH_AP				3

#define HASH_TYPE			HASH_SDBM

//////////////////////////////////////////////////////////////////////////
// BcHash
class BcHash
{
public:
	BcHash();
	BcHash( BcU32 Value );
	BcHash( const BcChar* pString );
	BcHash( const BcU8* pData, BcU32 Bytes );
	BcHash( const BcHash& Hash );
	operator BcU32() const;
	BcHash& operator = ( BcU32 Hash );
	BcHash& operator = ( const BcHash& Hash );
	BcBool operator == ( const BcHash& Hash ) const;
	BcBool operator != ( const BcHash& Hash ) const;
	BcBool operator < ( const BcHash& Hash ) const;
	BcBool operator > ( const BcHash& Hash ) const;

public:
	static BcU32	GenerateCRC32( const void* pInData, BcU32 Size );
	static BcU32	GenerateSDBM( const void* pInData, BcU32 Size );
	static BcU32	GenerateDJB( const void* pInData, BcU32 Size );
	static BcU32	GenerateAP( const void* pInData, BcU32 Size );

private:
	/**
	*	Hash a string.
	*/
	static BcHash	generateHash( const BcChar* pString );

	/**
	*	Hash data.
	*/
	static BcHash	generateHash( const BcU8* pData, BcU32 Bytes );

private:
	BcU32			Value_;				///< 32bit hash value.
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcHash::BcHash()
{
	Value_ = BcErrorCode;
}

inline BcHash::BcHash( BcU32 Value )
{
	Value_ = Value;
}

inline BcHash::BcHash( const BcChar* pString )
{
	Value_ = generateHash( pString );
}

inline BcHash::BcHash( const BcU8* pData, BcU32 Bytes )
{
	Value_ = generateHash( pData, Bytes );
}

inline BcHash::BcHash( const BcHash& Hash )
{
	Value_ = Hash.Value_;
}

inline BcHash::operator BcU32() const
{
	return Value_;
}

inline BcHash& BcHash::operator = ( BcU32 Value )
{
	Value_ = Value;
	return *this;
}

inline BcHash& BcHash::operator = ( const BcHash& Hash )
{
	Value_ = Hash.Value_;
	return *this;
}

inline BcBool BcHash::operator == ( const BcHash& Hash ) const
{
	return Value_ == Hash.Value_;
}

inline BcBool BcHash::operator != ( const BcHash& Hash ) const
{
	return Value_ != Hash.Value_;
}

inline BcBool BcHash::operator < ( const BcHash& Hash ) const
{
	return Value_ < Hash.Value_;
}

inline BcBool BcHash::operator > ( const BcHash& Hash ) const
{
	return Value_ > Hash.Value_;
}

#endif
