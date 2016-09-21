/**************************************************************************
*
* File:		ScnEntityObjectCodec.cpp
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*
*
*
*
*
**************************************************************************/

#include "System/Scene/Import/ScnEntityObjectCodec.h"
#include "System/Scene/ScnEntity.h"

#include "System/Content/CsPackage.h"
#include "System/Content/CsResource.h"

#include <algorithm>

namespace
{
	bool HasOwnerInHierarchy( ReObject* Owner, ReObject* Object )
	{
		while( Object != nullptr )
		{
			if( Owner == Object )
			{
				return true;
			}
			Object = Object->getOwner();
		}
		return false;
	}
}

ScnEntityObjectCodec::ScnEntityObjectCodec( class ScnEntity* RootEntity ):
	RootEntity_( RootEntity ),
	IncludeFieldFlags_( bcRFF_IMPORTER | bcRFF_OWNER ),
	ExcludeFieldFlags_( bcRFF_TRANSIENT ),
	PropagateFieldFlags_( bcRFF_IMPORTER | bcRFF_OWNER )
{
}

//virtual
BcBool ScnEntityObjectCodec::shouldSerialiseContents( 
	void* InData,
	const ReClass* InClass )
{
	BcBool RetVal = BcFalse;

	if( InClass->hasBaseClass( ReObject::StaticGetClass() ) )
	{
		CsResource* Resource = reinterpret_cast< CsResource* >( InData );
		if( HasOwnerInHierarchy( RootEntity_, Resource ) )
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
std::string ScnEntityObjectCodec::serialiseAsStringRef( 
	void* InData, 
	const ReClass* InClass )
{
	std::string RetVal;
	
	// Check if it's a resource.
	if( InClass->hasBaseClass( CsResource::StaticGetClass() ) )
	{
		CsResource* Resource = reinterpret_cast< CsResource* >( InData );
		if( !HasOwnerInHierarchy( RootEntity_, Resource ) )
		{
			ReObject* ResourcePackage = Resource->getPackage();
			if( ResourcePackage != nullptr )
			{
				BcChar OutChars[ 128 ] = { 0 };
				BcSPrintf( OutChars, sizeof( OutChars ) - 1, "$(%s:%s.%s)",  
					(*Resource->getClass()->getName()).c_str(), 
					(*ResourcePackage->getName()).c_str(),
					(*Resource->getName()).c_str() );
				RetVal = OutChars;
			}
		}
	}

	if( RetVal.empty() )
	{
		// Work out the super.
		if( InClass->hasBaseClass( ReObject::StaticGetClass() ) )
		{
			ReObject* Resource = reinterpret_cast< ReObject* >( InData );
			InClass = Resource->getClass();
		}

		// Default formatting.
		BcChar OutChars[ 128 ] = { 0 };
		BcSPrintf( OutChars, sizeof( OutChars ) - 1, 
			"$(%llu)",
			( (unsigned long long)InData ) );
		RetVal = OutChars;
	}

	return RetVal;
}

//virtual
BcBool ScnEntityObjectCodec::isMatchingField( 
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
	return BcStrCompareCaseInsensitive( FieldName.c_str(), Name.c_str() );
}

//virtual
BcBool ScnEntityObjectCodec::shouldSerialiseField( 
	void* InData, 
	BcU32 ParentFlags,
	const class ReField* Field )
{
	BcU32 Flags = Field->getFlags() | ( ParentFlags & PropagateFieldFlags_ );
	BcBool ShouldSerialise = 
		( ( ( Flags & ExcludeFieldFlags_ ) == 0 ) && 
		( ( Flags & IncludeFieldFlags_ ) != 0 ) );
	if( ShouldSerialise )
	{
		// If the field is an ReObject, we should check if it's a CsPackage.
		// - We don't want to serialise CsPackages.
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
BcBool ScnEntityObjectCodec::findObject( 
		void*& OutObject, const ReClass* Type, BcU32 Key )
{
	// If we don't have a package then create a dummy object for the serialiser.
	if( RootEntity_ == nullptr )
	{
		// Only support ReObjects. Can't patch in reference into to other objects.
		if( Type->hasBaseClass( ReObject::StaticGetClass() ))
		{
			// Always construct a dummy cross ref object.
			ReObject* Object = Type->create< ReObject >();
			Object->setName( BcName( "$CrossRef", Key ) );
			OutObject = Object;
			return BcTrue;
		}
		return BcFalse;
	}
	else
	{
		auto Package = RootEntity_->getPackage();
		if( Package != nullptr )
		{
			OutObject = Package->getCrossRefResource( Key );
		}
		return OutObject != nullptr;
	}
}

//virtual
BcBool ScnEntityObjectCodec::findObject( 
		void*& OutObject, const ReClass* Type, const std::string& Key )
{
	if( BcStrIsNumber( Key.c_str() ) )
	{
		return findObject( OutObject, Type, BcStrAtoi( Key.c_str() ) );
	}
	else
	{
		OutObject = nullptr;
	}
	return BcFalse;
}