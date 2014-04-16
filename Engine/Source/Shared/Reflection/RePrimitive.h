#ifndef __REFLECTION_PRIMITIVE_H__
#define __REFLECTION_PRIMITIVE_H__

#include "Reflection/ReManager.h"
#include "Reflection/ReObject.h"

//////////////////////////////////////////////////////////////////////////
// Primitive
class RePrimitive:
	public ReObject
{
public:
    REFLECTION_DECLARE_DERIVED( RePrimitive, ReObject );

public:
    RePrimitive();
    virtual ~RePrimitive(){};

private:

};

#endif
