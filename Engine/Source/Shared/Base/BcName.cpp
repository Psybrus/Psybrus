/**************************************************************************
*
* File:		BcName.cpp
* Author: 	Neil Richardson
* Ver/Date:	27/11/10
* Description:
*		
*		
*		
*		
* 
**************************************************************************/

#include "Base/BcName.h"
#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////////
// Statics
BcName BcName::INVALID;
BcName BcName::NONE( "None" );

BcNameEntryList* BcName::pStringEntries_ = nullptr;

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName():
	EntryIndex_( BcErrorCode ),
	ID_( BcErrorCode )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( BcU32 ID ):
	EntryIndex_( BcErrorCode ),
	ID_( ID )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const std::string& String )
{
	setInternal( String, BcErrorCode );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const std::string& String, BcU32 ID )
{
	setInternal( String, ID );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const BcChar* pString )
{
	setInternal( pString, BcErrorCode );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcName::BcName( const BcChar* pString, BcU32 ID )
{
	setInternal( pString, ID );
}

//////////////////////////////////////////////////////////////////////////
// Copy Ctor
BcName::BcName( const BcName& Other )
{
	EntryIndex_ = Other.EntryIndex_;
	ID_ = Other.ID_;
}

//////////////////////////////////////////////////////////////////////////
// operator =
BcName& BcName::operator = ( const BcName& Other )
{
	EntryIndex_ = Other.EntryIndex_;
	ID_ = Other.ID_;

	return (*this);
}

//////////////////////////////////////////////////////////////////////////
// Convert into string
std::string BcName::operator * () const
{
	const BcNameEntryList& StringEntries( getStringEntries() );

	if( EntryIndex_ < StringEntries.size() )
	{
		BcNameEntry Entry;
		{
			std::lock_guard< std::mutex > Lock( GetLock() );
			Entry = StringEntries[ EntryIndex_ ];
		}

		if( ID_ != BcErrorCode )
		{
			// Generate "value_id" string.
			static BcChar Buffer[ BcNameEntry::MAX_STRING_LENGTH + 12 ];

			BcSPrintf( Buffer, "%s_%u", &Entry.Value_[ 0 ], ID_ );
			return Buffer;
		}
		else
		{
			// No ID, just return raw string.
			return Entry.Value_;	
		}
	}

	return "<INVALID>";
}

//////////////////////////////////////////////////////////////////////////
// getValue.
std::string BcName::getValue() const
{
	BcVerifyMsg( EntryIndex_ != BcErrorCode, "BcName: Converting an invalid name to a string!" );
	const BcNameEntryList& StringEntries( getStringEntries() );

	if( EntryIndex_ < StringEntries.size() )
	{
		std::lock_guard< std::mutex > Lock( GetLock() );
		return StringEntries[ EntryIndex_ ].Value_;	
	}

	return "invalid";
}

//////////////////////////////////////////////////////////////////////////
// getID.
BcU32 BcName::getID() const
{
	return ID_;
}

//////////////////////////////////////////////////////////////////////////
// getUnique
BcName BcName::getUnique() const
{
	BcAssert( isValid() );
	BcNameEntryList& StringEntries( getStringEntries() );
	BcNameEntry SrcStringEntry;
	{
		std::lock_guard< std::mutex > Lock( GetLock() );
		SrcStringEntry = StringEntries[ EntryIndex_ ];
	}

	// Grab current id.
	BcU32 NewID = ID_;

	// If we haven't got an ID assigned already, then create one.
	if( ID_ == BcErrorCode )
	{
		// Advance ID for name.
		std::lock_guard< std::mutex > Lock( GetLock() );
		BcNameEntry& DstStringEntry( StringEntries[ EntryIndex_ ] );
		NewID = DstStringEntry.ID_++;
	}

	// Return new unique name.
	return BcName( SrcStringEntry.Value_, NewID );
}

//////////////////////////////////////////////////////////////////////////
// isValid
BcBool BcName::isValid() const
{
	return EntryIndex_ != BcErrorCode;
}

//////////////////////////////////////////////////////////////////////////
// operator ==
bool BcName::operator == ( const BcName& Other ) const
{
	return EntryIndex_ == Other.EntryIndex_ && ID_ == Other.ID_;
}

//////////////////////////////////////////////////////////////////////////
// operator !=
bool BcName::operator != ( const BcName& Other ) const
{
	return EntryIndex_ != Other.EntryIndex_ || ID_ != Other.ID_;
}

//////////////////////////////////////////////////////////////////////////
// operator <
bool BcName::operator < ( const BcName& Other ) const
{
	return EntryIndex_ < Other.EntryIndex_;
}

//////////////////////////////////////////////////////////////////////////
// setInternal.
void BcName::setInternal( const std::string& Value, BcU32 ID )
{
	// Check validity.
	BcVerifyMsg( isNameValid( Value ), "BcName: String (%s) contains invalid characters.", Value.c_str() );
	if( isNameValid( Value ) == BcFalse )
	{
		EntryIndex_ = BcErrorCode;
		ID_ = BcErrorCode;
		return;
	}

	// Entry index.
	BcU32 EntryIndex = BcErrorCode;

	// Find '_'
	std::string::size_type Position = Value.find_last_of( '_' );
		
	// If we have a trailing '_', strip out and use as ID if we can.
	if( Position != std::string::npos )
	{
		std::string NewID = Value.substr( Position + 1, Value.length() - ( Position + 1 ) );
		if( BcStrIsNumber( NewID.c_str() ) )
		{
			std::string NewValue = Value.substr( 0, Position );
			EntryIndex = getEntryIndex( NewValue );
			ID = BcStrAtoi( NewID.c_str() );
		}
		else
		{
			EntryIndex = getEntryIndex( Value );
		}
	}
	else
	{
		EntryIndex = getEntryIndex( Value );
	}

	if( EntryIndex != BcErrorCode )
	{
		// Store index and ID.
		EntryIndex_ = EntryIndex;

		// Store, and advance index.
		ID_ = ID;
	}
	else
	{
		EntryIndex_ = BcErrorCode;
		ID_ = BcErrorCode;
	}
}

//////////////////////////////////////////////////////////////////////////
// getStringEntries.
//static
BcNameEntryList& BcName::getStringEntries()
{
	BcAssertMsg( BcIsGameThread(), "Only safe for use on game thread!" );

	// Check if we've been initialised.
	if( pStringEntries_ == NULL )
	{
		pStringEntries_ = new BcNameEntryList();
		pStringEntries_->reserve( ENTRY_RESERVE_COUNT );
	}

	// Dereference and return.
	return *pStringEntries_;
}

//////////////////////////////////////////////////////////////////////////
// getEntryIndex.
//static
BcU32 BcName::getEntryIndex( const std::string& Value )
{
	// If string is too long, return invalid index.
	BcVerifyMsg( Value.length() < BcNameEntry::MAX_STRING_LENGTH, "BcName: String(%s) too long to store in name table.", Value.c_str() );
	if( Value.length() >= BcNameEntry::MAX_STRING_LENGTH )
	{
		return BcErrorCode;
	}

	// If string is too short, return invalid index. Don't warn.
	if( Value.length() == 0 )
	{
		return BcErrorCode;
	}

	// TODO: Store in a hash map?
	// Iterate over array to find if string exists.
	std::lock_guard< std::mutex > Lock( GetLock() );
	BcNameEntryList& StringEntries( getStringEntries() );
	for( BcU32 Idx = 0; Idx < StringEntries.size(); ++Idx )
	{
		BcNameEntry& Entry = StringEntries[ Idx ];

		// If we find the entry, return it's index.
		if( BcStrCompare( Value.c_str(), Entry.Value_ ) )
		{
			return Idx;
		}
	}

	// If we make it here, add a new entry.
	BcNameEntry NewEntry;
	BcStrCopyN( &NewEntry.Value_[ 0 ], Value.c_str(), BcNameEntry::MAX_STRING_LENGTH );
	NewEntry.ID_ = 0;
	StringEntries.push_back( NewEntry );

	// Check if entry count has went over reserve list size. Not a critical failure.
	BcVerifyMsg( StringEntries.size() < ENTRY_RESERVE_COUNT, "BcName: Reached end of reserve size." );

	// Return new entry index.
	return static_cast< BcU32 >( StringEntries.size() - 1 );
}

//////////////////////////////////////////////////////////////////////////
// isNameValid.
//static
BcBool BcName::isNameValid( const std::string& Value )
{
	if( !validate( Value.c_str() ) )
	{
		return BcFalse;
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// isCharValid.
//static
BcBool BcName::isCharValid( const BcChar Value )
{
	// Lets just enable all characters. Why restrict it?
	return true;
}

//////////////////////////////////////////////////////////////////////////
// validate.
//static
BcBool BcName::validate( const BcChar* pString )
{
	BcChar Char = 0;
	while( ( Char = *pString++ ) != 0 )
	{
		if( !isCharValid( Char ) )
		{
			return BcFalse;
		}
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// StripInvalidChars.
//static
std::string BcName::StripInvalidChars( const BcChar* pString )
{
	std::string Value;

	BcChar Char = 0;
	while( ( Char = *pString++ ) != 0 )
	{
		if( isCharValid( Char ) )
		{
			Value += Char;
		}
	}

	return Value;
}

//////////////////////////////////////////////////////////////////////////
// GetLock
std::mutex& BcName::GetLock()
{
	static std::mutex Lock;
	return Lock;
}
