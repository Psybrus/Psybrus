#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"
#include "Reflection/ReField.h"
#include "Reflection/ReReflection.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_DERIVED( ReEnum );

void ReEnum::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "EnumConstants_", &ReEnum::EnumConstants_ ),
	};
		
	ReRegisterClass< ReEnum, ReClass >( Fields );
}

//////////////////////////////////////////////////////////////////////////
// Enum
ReEnum::ReEnum()
{
	
}

ReEnum::ReEnum( BcName Name ):
	ReClass( Name )
{
			
}

void ReEnum::setConstants( ReEnumConstant** EnumConstants, BcU32 Elements )
{
	EnumConstants_.reserve( Elements );
	for( BcU32 Idx = 0; Idx < Elements; ++Idx )
	{
		EnumConstants_.push_back( EnumConstants[ Idx ] );
	}
}

const ReEnumConstant* ReEnum::getEnumConstant( BcU32 Value )
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

const ReEnumConstant* ReEnum::getEnumConstant( const std::string& Name )
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
