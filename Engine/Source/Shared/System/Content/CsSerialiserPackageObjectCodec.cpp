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

#include <algorithm>

CsSerialiserPackageObjectCodec::CsSerialiserPackageObjectCodec( 
		class CsPackage* Package,
		BcU32 IncludeFieldFlags,
		BcU32 ExcludeFieldFlags,
		BcU32 PropagateFieldFlags ):
	Package_( Package ),
	IncludeFieldFlags_( IncludeFieldFlags ),
	ExcludeFieldFlags_( ExcludeFieldFlags ),
	PropagateFieldFlags_( PropagateFieldFlags )
{

}

//virtual
BcBool CsSerialiserPackageObjectCodec::shouldSerialiseContents( 
	void* InData, 
	const ReClass* InType )
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
	const ReClass* InType )
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
				BcChar OutChars[ 128 ];
				BcSPrintf( OutChars, "$(%s:%s.%s)",  
					(*Resource->getClass()->getName()).c_str(), 
					(*ResourceRootOwner->getName()).c_str(),
					(*Resource->getName()).c_str() );
				RetVal = OutChars;
			}
		}
	}

	if( RetVal.empty() )
	{
		// Default formatting.
		BcChar OutChars[ 128 ];
		BcSPrintf( OutChars, "$(%s:%s.%llu)",   
			(*InType->getName()).c_str(),
			( "this" ),
			( (unsigned long long)InData ) );
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
	
	// Remove underscores.
	auto EndIt = std::remove_if( FieldName.begin(), FieldName.end(),
		[]( char InChar )
		{
			return InChar == '_';
		} );
	FieldName.erase( EndIt, FieldName.end() );

	// Case insensitive compare.
	// Not UTF-8 safe. We shouldn't need to worry as input names should be ASCII (should later perform checks for this).
	return BcStrCompare( FieldName.c_str(), Name.c_str() );
}

//virtual
BcBool CsSerialiserPackageObjectCodec::shouldSerialiseField( 
	void* InData, 
	BcU32 ParentFlags,
	const class ReField* Field )
{
	BcU32 Flags = Field->getFlags() | ( ParentFlags & PropagateFieldFlags_ );
	BcBool ShouldSerialise = 
		( ( ( Flags & ExcludeFieldFlags_ ) == 0 ) && 
		( ( Flags & IncludeFieldFlags_ ) != 0 ) );
		
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


//virtual 
BcBool CsSerialiserPackageObjectCodec::findObject( 
		void*& OutObject, const ReClass* Type, BcU32 Key )
{
	OutObject = Package_->getCrossRefResource( Key );
	return OutObject != nullptr;
}

//virtual
BcBool CsSerialiserPackageObjectCodec::findObject( 
		void*& OutObject, const ReClass* Type, const std::string& Key )
{
	OutObject = nullptr;
	return BcFalse;
}