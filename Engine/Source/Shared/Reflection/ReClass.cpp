#include "Reflection/ReClass.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( Class );

void Class::StaticRegisterClass()
{
	static const Field Fields[] = 
	{
		Field( "Super_",		&Class::Super_ ),
		Field( "Fields_",		&Class::Fields_ ),
	};
		
	RegisterClass< Class, Type >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Class
Class::Class():
	Super_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Class
Class::Class( const std::string& Name ):
	Super_( nullptr )
{
	setName( Name );
}
	
//////////////////////////////////////////////////////////////////////////
// setSuper
void Class::setSuper( const Class* Super )
{
	Super_ = Super;
}
	
//////////////////////////////////////////////////////////////////////////
// getSuper
const Class* Class::getSuper() const
{
	return Super_;
}
	
//////////////////////////////////////////////////////////////////////////
// hasBaseClass
BcBool Class::hasBaseClass( const Class* pClass ) const
{
	if( this == pClass )
	{
		return true;
	}
	else
	{
		const Class* pSuperClass = getSuper();
		if( pSuperClass != nullptr )
		{
			return pSuperClass->hasBaseClass( pClass );
		}
	}
	return false;
}
	
//////////////////////////////////////////////////////////////////////////
// setFields
void Class::setFields( BcU32 NoofFields, const Field* pFields )
{
	Fields_.reserve( NoofFields );
	for( BcU32 Idx = 0; Idx < NoofFields; ++Idx )
	{
		Fields_.push_back( &pFields[ Idx ] );
	}
}
	
//////////////////////////////////////////////////////////////////////////
// getField
const Field* Class::getField( BcU32 Idx ) const
{
	return Fields_[ Idx ];
}
	
//////////////////////////////////////////////////////////////////////////
// getNoofFields
BcU32 Class::getNoofFields() const
{
	return Fields_.size();
}
