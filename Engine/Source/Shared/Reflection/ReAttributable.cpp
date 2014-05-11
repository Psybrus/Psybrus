#include "Reflection/ReAttributable.h"
#include "Reflection/ReClass.h"

#include "Reflection/ReReflection.h"

REFLECTION_DEFINE_DERIVED( ReAttributable );

void ReAttributable::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Attributes_",		&ReAttributable::Attributes_ ),
	};
		
	ReRegisterClass< ReAttributable, RePrimitive >( Fields );
}

ReAttributable::ReAttributable()
{

}

//virtual
ReAttributable::~ReAttributable()
{

}

ReAttributable& ReAttributable::addAttribute( ReAttribute* Attribute )
{
	Attributes_.push_front( Attribute );
	return *this;
}


ReAttribute* ReAttributable::getAttribute( const ReClass* Class )
{
	ReAttribute* RetVal = nullptr;

	for( auto* Attribute : Attributes_ )
	{
		if( Attribute->isTypeOf( Class ) )
		{
			RetVal = Attribute;
			break;
		}
	}

	return RetVal;
}
