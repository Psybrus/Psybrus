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

#include "BcTypes.h"
#include "BcDebug.h"
#include "BcString.h"

#include <string>

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
#ifdef PSY_DEBUG
	std::string		String_;			///< Debugging.
#endif
};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcHash::BcHash()
{
	Value_ = BcErrorCode;
#ifdef PSY_DEBUG
	String_ = "BcErrorCode";
#endif
}

inline BcHash::BcHash( BcU32 Value )
{
	Value_ = Value;
#ifdef PSY_DEBUG
	String_ = "BcU32 Value";
#endif
}

inline BcHash::BcHash( const BcChar* pString )
{
	Value_ = generateHash( pString );
#ifdef PSY_DEBUG
	String_ = pString;
#endif
}

inline BcHash::BcHash( const BcU8* pData, BcU32 Bytes )
{
	Value_ = generateHash( pData, Bytes );
#ifdef PSY_DEBUG
	String_ = "Data Value";
#endif
}

inline BcHash::BcHash( const BcHash& Hash )
{
	Value_ = Hash.Value_;
#ifdef PSY_DEBUG
	String_ = Hash.String_;
#endif
}

inline BcHash::operator BcU32() const
{
	return Value_;
}

inline BcHash& BcHash::operator = ( BcU32 Value )
{
	Value_ = Value;
#ifdef PSY_DEBUG
	String_ =  "BcU32 Value";
#endif
	return *this;
}

inline BcHash& BcHash::operator = ( const BcHash& Hash )
{
	Value_ = Hash.Value_;
#ifdef PSY_DEBUG
	String_ = Hash.String_;
#endif
	return *this;
}

inline BcBool BcHash::operator == ( const BcHash& Hash ) const
{
#ifdef PSY_DEBUG
	if( Value_ == Hash.Value_ )
	{
		BcAssertMsg( String_ == Hash.String_, "BcHash: Collision detected." );
	}
#endif

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
