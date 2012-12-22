/**************************************************************************
*
* File:		BcName.h
* Author: 	Neil Richardson
* Ver/Date:	27/11/10
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#ifndef __BCNAME_H__
#define __BCNAME_H__

#include "Base/BcTypes.h"
#include "Base/BcDebug.h"
#include "Base/BcString.h"

//////////////////////////////////////////////////////////////////////////
// BcNameEntry
struct BcNameEntry
{
	enum
	{
		ENTRY_SIZE_BYTES = 32,									// How large each entry should be.
		ID_SIZE_BYTES = sizeof( BcU32 ),						// How many bytes the ID takes up.
		MAX_STRING_LENGTH = ENTRY_SIZE_BYTES - ID_SIZE_BYTES,	// String size taking previous 2 values into account.
		ENTRY_RESERVE_COUNT = 1024								// How many entries to reserve ahead of time to save reallocation.
	};

	BcChar Value_[ MAX_STRING_LENGTH ];
	BcU32 ID_;
};

//////////////////////////////////////////////////////////////////////////
// BcNameEntryList
typedef std::vector< BcNameEntry > BcNameEntryList;

//////////////////////////////////////////////////////////////////////////
// BcName
class BcName
{
public:
	static BcName INVALID;
	static BcName NONE;

public:
	enum
	{
		ENTRY_RESERVE_COUNT = 1024								// How many entries to reserve ahead of time to save reallocation.
	};

public:
	BcName();													// Constructs invalid name.
	BcName( const std::string& String );						// Constructs name, autoassigns ID.
	BcName( const std::string& String, BcU32 ID );				// Constructs name, manual ID assignment. BcErrorCode means no ID.
	BcName( const BcChar* pString );							// Constructs name, autoassigns ID.
	BcName( const BcChar* pString, BcU32 ID );					// Constructs name, manual ID assignment. BcErrorCode means no ID.
	BcName( const BcName& Other );								// Copy name.
	BcName& operator = ( const BcName& Other );					// Assign name.

	/**
	 * Convert into string.
	 */
	std::string operator * () const;

	/**
	 * Get value.
	 */
	std::string getValue() const;

	/**
	 * Get ID.
	 */
	BcU32 getID() const;

	/**
	 * Get unique(ish) name.
	 */
	BcName getUnique() const;

	/**
	 * Is valid?
	 */
	BcBool isValid() const;

	bool operator == ( const BcName& Other ) const;
	bool operator != ( const BcName& Other ) const;
	bool operator < ( const BcName& Other ) const;

private:
	static BcBool validate( const BcChar* pString );

private:
	void setInternal( const std::string& Value, BcU32 ID );

private:
	BcU32 EntryIndex_;
	BcU32 ID_;

private:
	static BcNameEntryList* pStringEntries_;

	/**
	 * Get string entries.
	 */
	static BcNameEntryList& getStringEntries();

	/**
	 * Get entry index. Will return a new entry if it didn't exist.
	 */
	static BcU32 getEntryIndex( const std::string& Value );

	/**
	 * Increment entry ID.
	 */
	static void incrementEntryID( BcU32 Index );

	/**
	 * Validate string is valid as a name.
	 */
	static BcBool isNameValid( const std::string& Value );
};


#endif
