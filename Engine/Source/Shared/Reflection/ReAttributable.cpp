#include "Reflection/ReAttributable.h"
#include "Reflection/ReClass.h"

REFLECTION_DEFINE_DERIVED( ReAttributable );

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
		if( Attribute->isTypeOf( Class->getName() ) )
		{
			RetVal = Attribute;
			break;
		}
	}

	return RetVal;
}
