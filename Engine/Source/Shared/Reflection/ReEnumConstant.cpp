#include "Reflection/ReEnumConstant.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( ReEnumConstant );

void ReEnumConstant::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Value_", &ReEnumConstant::Value_ ),
	};
		
	ReRegisterClass< ReEnumConstant, RePrimitive >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// EnumConstant
ReEnumConstant::ReEnumConstant():
	Value_( 0 )
{
	
}

//////////////////////////////////////////////////////////////////////////
// EnumConstant
ReEnumConstant::ReEnumConstant( BcName Name, BcU32 Value ):
	Value_( Value )
{
	setName( Name );
}
	
//////////////////////////////////////////////////////////////////////////
// setValue
void ReEnumConstant::setValue( BcU32 Value )
{
	Value_ = Value;
}
	
//////////////////////////////////////////////////////////////////////////
// getValue
BcU32 ReEnumConstant::getValue() const
{
	return Value_;
}
