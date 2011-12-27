/**************************************************************************
*
* File:		BcPath.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		File system path
*		
*
*
* 
**************************************************************************/

#include "BcPath.h"

//////////////////////////////////////////////////////////////////////////
// Statics
const BcChar BcPath::Seperator = '/';

//////////////////////////////////////////////////////////////////////////
// Ctor
BcPath::BcPath()
{

}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcPath::BcPath( const BcPath& Value ):
	InternalValue_( *Value )
{
	fixSeperators();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcPath::BcPath( const std::string& Value ):
	InternalValue_( Value )
{
	fixSeperators();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
BcPath::BcPath( const BcChar* pValue ):
	InternalValue_( pValue )
{
	fixSeperators();
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
BcPath::~BcPath()
{
	
}

//////////////////////////////////////////////////////////////////////////
// operator *
const BcChar* BcPath::operator * () const
{
	return InternalValue_.c_str();
}

//////////////////////////////////////////////////////////////////////////
// getExtension
const BcChar* BcPath::getExtension() const
{
	std::string::size_type ExtensionPosition = InternalValue_.rfind( "." );

	if( ExtensionPosition != std::string::npos )
	{
		// Ensure we're not masked by a trailing seperator.
		std::string Extension = InternalValue_.substr( ExtensionPosition + 1 );

		if( Extension.find( "/" ) == std::string::npos )
		{
			return &InternalValue_[ ExtensionPosition + 1 ];
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// join
void BcPath::join( const BcPath& PathA )
{
	InternalValue_ = appendTrailingSeperator( InternalValue_ );
	InternalValue_ += stripLeadingSeperator( PathA.InternalValue_ );
}

//////////////////////////////////////////////////////////////////////////
// join
void BcPath::join( const BcPath& PathA, const BcPath& PathB )
{
	join( PathA );
	join( PathB );
}

//////////////////////////////////////////////////////////////////////////
// join
void BcPath::join( const BcPath& PathA, const BcPath& PathB, const BcPath& PathC )
{
	join( PathA );
	join( PathB );
	join( PathC );
}

//////////////////////////////////////////////////////////////////////////
// fixSeperators
void BcPath::fixSeperators()
{
	std::string NewValue;
	for( BcU32 Idx = 0; Idx < InternalValue_.size(); ++Idx )
	{
		BcChar Char = InternalValue_[ Idx ];
		if( Char == '\\' )
		{
			NewValue += Seperator;
		}
		else
		{
			NewValue += Char;
		}
	}
	
	InternalValue_ = NewValue;
}

//////////////////////////////////////////////////////////////////////////
// stripLeadingSeperator
//static
std::string BcPath::stripLeadingSeperator( const std::string& Path )
{
	if( Path[0] == Seperator )
	{
		return Path.substr( 1 );
	}

	return Path;
}

//////////////////////////////////////////////////////////////////////////
// appendTrailingSeperator
//static
std::string BcPath::appendTrailingSeperator( const std::string& Path )
{
	if( Path[ Path.size() - 1 ] != Seperator )
	{
		return Path + Seperator;
	}

	return Path;
}
