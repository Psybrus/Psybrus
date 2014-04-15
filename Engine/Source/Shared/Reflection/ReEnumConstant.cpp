#include "Reflection/ReEnumConstant.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( EnumConstant );

void EnumConstant::StaticRegisterClass()
{
	static const Field Fields[] = 
	{
		Field( "Value_", &EnumConstant::Value_ ),
	};
		
	RegisterClass< EnumConstant, Primitive >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// EnumConstant
EnumConstant::EnumConstant():
	Value_( 0 )
{
	
}

//////////////////////////////////////////////////////////////////////////
// EnumConstant
EnumConstant::EnumConstant( const std::string& Name, BcU32 Value ):
	Value_( Value )
{
	setName( Name );
}
	
//////////////////////////////////////////////////////////////////////////
// setValue
void EnumConstant::setValue( BcU32 Value )
{
	Value_ = Value;
}
	
//////////////////////////////////////////////////////////////////////////
// getValue
BcU32 EnumConstant::getValue() const
{
	return Value_;
}
