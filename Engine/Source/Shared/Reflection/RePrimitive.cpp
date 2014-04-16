#include "Reflection/RePrimitive.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( RePrimitive );

void RePrimitive::StaticRegisterClass()
{
	ReRegisterClass< RePrimitive, ReObject >();
}

//////////////////////////////////////////////////////////////////////////
// Primitive
RePrimitive::RePrimitive()
{
	
}
