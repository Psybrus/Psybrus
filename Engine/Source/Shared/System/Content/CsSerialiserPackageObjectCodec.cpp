/**************************************************************************
*
* File:		CsSerialiserPackageObjectCodec.cpp
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Content/CsSerialiserPackageObjectCodec.h"
#include "System/Content/CsPackage.h"
#include "System/Content/CsResource.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

CsSerialiserPackageObjectCodec::CsSerialiserPackageObjectCodec( class CsPackage* Package ):
	Package_( Package )
{

}

//virtual
BcBool CsSerialiserPackageObjectCodec::shouldSerialise( void* InData, const ReType* InType )
{
	BcBool RetVal = BcFalse;
	if( InType->isTypeOf< ReClass >() && Package_ != nullptr )
	{
		const ReClass* InClass = static_cast< const ReClass* >( InType );
		if( InClass->hasBaseClass( CsResource::StaticGetClass() ) )
		{
			CsResource* Resource = reinterpret_cast< CsResource* >( InData );
			CsPackage* ResourcePackage = Resource->getPackage();
			if( Package_ != ResourcePackage )
			{
				RetVal = BcTrue;
			}
		}
		else
		{
			RetVal = BcTrue;
		}
	}
	else
	{
		RetVal = BcTrue;
	}

	return RetVal;
}

//virtual
std::string CsSerialiserPackageObjectCodec::serialiseAsStringRef( void* InData, const ReType* InType )
{
	std::string RetVal;

	if( InType->isTypeOf< ReClass >() && Package_ != nullptr )
	{
		const ReClass* InClass = static_cast< const ReClass* >( InType );

		// Check if it's a resource.
		if( InClass->hasBaseClass( CsResource::StaticGetClass() ) )
		{
			CsResource* Resource = reinterpret_cast< CsResource* >( InData );
			CsPackage* ResourcePackage = Resource->getPackage();
			if( ResourcePackage != nullptr )
			{
				RetVal = boost::str( boost::format( "$(%1%:%2%.%3%)" ) % 
					(*InClass->getName()) % 
					(*ResourcePackage->getName()) %
					(*Resource->getName()) );
			}
		}
	}

	if( RetVal.empty() )
	{
		// Default formatting.
		RetVal = boost::str( boost::format( "$(%1%:%2%.%3%)" ) % 
			(*InType->getName()) % 
			( "this" ) %
			( (BcU64)InData ) );
	}

	return RetVal;
}

//virtual
BcBool CsSerialiserPackageObjectCodec::isMatchingField( const class ReField* Field, const std::string& Name )
{
	// Just check against field name first.
	std::string FieldName = *Field->getName();
	if( *Field->getName() == Name )
	{
		return BcTrue;
	}
	
	// Attempt case insensitive comparison without underscores.
	boost::erase_all( FieldName, "_" );

	// Not UTF-8 safe. We shouldn't need to worry as input names should be ASCII (should later perform checks for this).
	return boost::iequals( FieldName, Name );
}
