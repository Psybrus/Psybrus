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

ReEnum::~ReEnum()
{
	for( auto* EnumConstant : EnumConstants_ )
	{
		delete EnumConstant;
	}
}

void ReEnum::setConstants( ReEnumConstant** EnumConstants, BcU32 Elements )
{
	EnumConstants_.reserve( Elements );
	for( BcU32 Idx = 0; Idx < Elements; ++Idx )
	{
		EnumConstants_.push_back( EnumConstants[ Idx ] );
	}
}

const ReEnumConstant* ReEnum::getEnumConstant( BcU32 Value ) const
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

const ReEnumConstant* ReEnum::getEnumConstant( const std::string& Name ) const
{
	for( BcU32 Idx = 0; Idx < EnumConstants_.size(); ++Idx )
	{
		if( BcStrCompare(  (*EnumConstants_[ Idx ]->getName()).c_str(), Name.c_str() ) )
		{
			return EnumConstants_[ Idx ];
		}
	}

	return nullptr;
}

const std::vector< const ReEnumConstant* >& ReEnum::getEnumConstants() const
{
	return EnumConstants_;
}

