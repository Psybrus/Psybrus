#include "Reflection/ReType.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( Type );

void Type::StaticRegisterClass()
{
	static const Field Fields[] = 
	{
		Field( "Size_", &Type::Size_ ),
	};
		
	RegisterClass< Type, Primitive >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
Type::Type():
	Size_( 0 ),
	Serialiser_( nullptr )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
Type::~Type()
{
	delete Serialiser_;
	Serialiser_ = nullptr;
}

	
//////////////////////////////////////////////////////////////////////////
// getSize
BcU32 Type::getSize() const
{
	return Size_;
}
