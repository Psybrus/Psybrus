#include "Reflection/ReAttribute.h"

#include "Reflection/ReReflection.h"

REFLECTION_DEFINE_DERIVED( ReAttribute );

void ReAttribute::StaticRegisterClass()
{
	ReRegisterClass< ReAttribute, RePrimitive >();
}


ReAttribute::ReAttribute()
{

}

//virtual
ReAttribute::~ReAttribute()
{

}

