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

CsSerialiserPackageObjectCodec::CsSerialiserPackageObjectCodec( 
		class CsPackage* Package,
		BcU32 IncludeFieldFlags,
		BcU32 ExcludeFieldFlags ):
	Package_( Package ),
	IncludeFieldFlags_( IncludeFieldFlags ),
	ExcludeFieldFlags_( ExcludeFieldFlags )
{

}

//virtual
BcBool CsSerialiserPackageObjectCodec::shouldSerialiseContents( 
	void* InData, 
	const ReType* InType )
{
	BcBool RetVal = BcFalse;
	if( InType->isTypeOf< ReClass >() && Package_ != nullptr )
	{
		const ReClass* InClass = static_cast< const ReClass* >( InType );

		if( InClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			CsResource* Resource = reinterpret_cast< CsResource* >( InData );
			ReObject* ResourceRootOwner = Resource->getRootOwner();
			if( Package_ == ResourceRootOwner )
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
std::string CsSerialiserPackageObjectCodec::serialiseAsStringRef( 
	void* InData, 
	const ReType* InType )
{
	std::string RetVal;

	if( InType->isTypeOf< ReClass >() && Package_ != nullptr )
	{
		const ReClass* InClass = static_cast< const ReClass* >( InType );

		// Check if it's a resource.
		if( InClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			CsResource* Resource = reinterpret_cast< CsResource* >( InData );
			ReObject* ResourceRootOwner = Resource->getRootOwner();
			if( ResourceRootOwner != nullptr )
			{
				RetVal = boost::str( boost::format( "$(%1%:%2%.%3%)" ) % 
					(*Resource->getClass()->getName()) % 
					(*ResourceRootOwner->getName()) %
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
BcBool CsSerialiserPackageObjectCodec::isMatchingField( 
	const class ReField* Field, 
	const std::string& Name )
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

//virtual
BcBool CsSerialiserPackageObjectCodec::shouldSerialiseField( 
	void* InData, 
	const class ReField* Field )
{
	BcBool ShouldSerialise = 
		( ( ( Field->getFlags() & ExcludeFieldFlags_ ) == 0 ) && 
		( ( Field->getFlags() & IncludeFieldFlags_ ) != 0 ) ) || 
		Field->getFlags() == 0;

	// If the field is an ReObject, we should check if it's a CsPackage.
	// - We don't want to serialise CsPackages.
	if( Field->getType()->isTypeOf< ReClass >() )
	{
		const ReClass* InClass = static_cast< const ReClass* >( Field->getType() );

		// Check if it's an object.
		if( InClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			ReFieldAccessor Accessor( InData, Field );
			if( !Accessor.isContainerType() )
			{
				const ReClass* UpperClass = Accessor.getUpperClass();
				if( UpperClass == CsPackage::StaticGetClass() )
				{
					return BcFalse;
				}
			}
		}
	}
	
	return ShouldSerialise;
}
