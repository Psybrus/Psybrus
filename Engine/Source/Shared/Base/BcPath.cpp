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
std::string BcPath::operator * () const
{
	return InternalValue_;
}

//////////////////////////////////////////////////////////////////////////
// getFileName
std::string BcPath::getFileName() const
{
	std::string::size_type PathPosition = InternalValue_.rfind( Seperator );

	if( PathPosition != std::string::npos )
	{
		return &InternalValue_[ PathPosition + 1 ];
	}

	return InternalValue_;
}

//////////////////////////////////////////////////////////////////////////
// getExtension
std::string BcPath::getExtension() const
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
// getFileNameNoExtension
std::string BcPath::getFileNameNoExtension() const
{
	std::string::size_type PathPosition = InternalValue_.rfind( Seperator );
	std::string::size_type ExtensionPosition = InternalValue_.rfind( "." );

	if( ExtensionPosition != std::string::npos )
	{
		if( PathPosition != std::string::npos )
		{
			std::string FileNameNoExtension = InternalValue_.substr( PathPosition + 1, ExtensionPosition - ( PathPosition + 1 ) );
			return FileNameNoExtension;
		}
		else
		{
			std::string FileNameNoExtension = InternalValue_.substr( 0, ExtensionPosition );
			return FileNameNoExtension;
		}
	}

	return InternalValue_.c_str();
}

//////////////////////////////////////////////////////////////////////////
// append
void BcPath::append( const BcPath& PathA )
{
	InternalValue_ += PathA.InternalValue_;
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
