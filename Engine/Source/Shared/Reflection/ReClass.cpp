#include "Reflection/ReClass.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( ReClass );

void ReClass::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Super_",		&ReClass::Super_ ),
		ReField( "Fields_",		&ReClass::Fields_ ),
	};
		
	ReRegisterClass< ReClass, ReType >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Class
ReClass::ReClass():
	Super_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Class
ReClass::ReClass( BcName Name ):
	Super_( nullptr )
{
	setName( Name );
}
	
//////////////////////////////////////////////////////////////////////////
// setSuper
void ReClass::setSuper( const ReClass* Super )
{
	Super_ = Super;
}
	
//////////////////////////////////////////////////////////////////////////
// getSuper
const ReClass* ReClass::getSuper() const
{
	return Super_;
}
	
//////////////////////////////////////////////////////////////////////////
// hasBaseClass
BcBool ReClass::hasBaseClass( const ReClass* pClass ) const
{
	if( this == pClass )
	{
		return true;
	}
	else
	{
		const ReClass* pSuperClass = getSuper();
		if( pSuperClass != nullptr )
		{
			return pSuperClass->hasBaseClass( pClass );
		}
	}
	return false;
}
	
//////////////////////////////////////////////////////////////////////////
// setFields
void ReClass::setFields( BcU32 NoofFields, const ReField* pFields )
{
	BcAssertMsg( Fields_.size() == 0, "Fields already set." );
	Fields_.reserve( NoofFields );
	for( BcU32 Idx = 0; Idx < NoofFields; ++Idx )
	{
		Fields_.push_back( &pFields[ Idx ] );
	}
}
	
//////////////////////////////////////////////////////////////////////////
// getField
const ReField* ReClass::getField( BcU32 Idx ) const
{
	return Fields_[ Idx ];
}
	
//////////////////////////////////////////////////////////////////////////
// getNoofFields
BcU32 ReClass::getNoofFields() const
{
	return Fields_.size();
}
