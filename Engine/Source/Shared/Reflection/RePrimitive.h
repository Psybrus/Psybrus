#ifndef __REFLECTION_PRIMITIVE_H__
#define __REFLECTION_PRIMITIVE_H__

#include "Reflection/ReManager.h"
#include "Reflection/ReObject.h"

//////////////////////////////////////////////////////////////////////////
// Primitive
class Primitive:
	public Object
{
public:
	REFLECTION_DECLARE_DERIVED( Primitive, Object );

public:
	Primitive();
	virtual ~Primitive(){};

private:

};

#endif
