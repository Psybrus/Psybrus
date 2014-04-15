#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( Enum );

void Enum::StaticRegisterClass()
{
	static const Field Fields[] = 
	{
		Field( "EnumConstants_", &Enum::EnumConstants_ ),
	};
		
	RegisterClass< Enum, Class >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Enum
Enum::Enum()
{
	
}

Enum::Enum( const std::string& Name ):
	Class( Name )
{
			
}

void Enum::setConstants( const EnumConstant* EnumConstants, BcU32 Elements )
{
	EnumConstants_.reserve( Elements );
	for( BcU32 Idx = 0; Idx < Elements; ++Idx )
	{
		EnumConstants_.push_back( &EnumConstants[ Idx ] );
	}
}

const EnumConstant* Enum::getEnumConstant( BcU32 Value )
{
	for( BcU32 Idx = 0; Idx < EnumConstants_.size(); ++Idx )
	{
		if( EnumConstants_[ Idx ]->getValue() == Value )
		{
			return EnumConstants_[ Idx ];
		}
	}

	return nullptr;
}

const EnumConstant* Enum::getEnumConstant( const std::string& Name )
{
	for( BcU32 Idx = 0; Idx < EnumConstants_.size(); ++Idx )
	{
		if( EnumConstants_[ Idx ]->getName() == Name )
		{
			return EnumConstants_[ Idx ];
		}
	}

	return nullptr;
}
