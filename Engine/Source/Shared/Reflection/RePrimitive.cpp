#include "Reflection/RePrimitive.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( Primitive );

void Primitive::StaticRegisterClass()
{
	RegisterClass< Primitive, Object >();
}

//////////////////////////////////////////////////////////////////////////
// Primitive
Primitive::Primitive()
{
	
}
