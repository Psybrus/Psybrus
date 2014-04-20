#ifndef __REFLECTION_ATTRIBUTE_H__
#define __REFLECTION_ATTRIBUTE_H__

#include "Reflection/RePrimitive.h"

//////////////////////////////////////////////////////////////////////////
// ReAttribute
class ReAttribute:
    public RePrimitive
{
public:
    REFLECTION_DECLARE_DERIVED( ReAttribute, RePrimitive );

public:
    ReAttribute();
    virtual ~ReAttribute();
};

#endif // __REFLECTION_ATTRIBUTE_H__
