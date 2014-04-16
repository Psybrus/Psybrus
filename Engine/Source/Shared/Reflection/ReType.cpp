#include "Reflection/ReType.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( ReType );

void ReType::StaticRegisterClass()
{
	static const ReField Fields[] = 
	{
		ReField( "Size_", &ReType::Size_ ),
	};
		
	ReRegisterClass< ReType, RePrimitive >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ReType::ReType():
	Size_( 0 ),
	Serialiser_( nullptr )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ReType::~ReType()
{
	delete Serialiser_;
	Serialiser_ = nullptr;
}

	
//////////////////////////////////////////////////////////////////////////
// getSize
BcU32 ReType::getSize() const
{
	return Size_;
}
