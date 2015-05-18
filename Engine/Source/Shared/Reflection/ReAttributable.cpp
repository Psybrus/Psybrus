#include "Reflection/ReAttributable.h"
#include "Reflection/ReClass.h"

#include "Reflection/ReReflection.h"

REFLECTION_DEFINE_DERIVED( ReAttributable );

void ReAttributable::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Attributes_",		&ReAttributable::Attributes_ ),
	};
		
	ReRegisterClass< ReAttributable, RePrimitive >( Fields );
}

ReAttributable::ReAttributable()
{

}

//virtual
ReAttributable::~ReAttributable()
{
	for( auto* Attribute : Attributes_ )
	{
		delete Attribute;
	}
}

ReAttributable& ReAttributable::addAttribute( ReAttribute* Attribute )
{
	Attribute->setOwner( this );
	Attributes_.push_front( Attribute );
	return *this;
}


ReAttribute* ReAttributable::getAttribute( const ReClass* Class ) const
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
