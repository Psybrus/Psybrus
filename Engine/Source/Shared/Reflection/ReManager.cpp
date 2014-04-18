#include "Reflection/ReManager.h"

#include "Reflection/ReClass.h"
#include "Reflection/ReClassSerialiser_BasicType.h"
#include "Reflection/ReClassSerialiser_StringType.h"
#include "Reflection/ReClassSerialiser_BinaryDataType.h"
#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_BASE( ReITypeSerialiser );

// TODO: Move into Manager class.
			
typedef std::map< BcName, ReType* > TypeMap;

struct Factory
{
	static Factory& Instance()
	{
		static Factory Factory;
		return Factory;
	}

	Factory()
	{

	}

	~Factory()
	{
		// Delete remaining types help us have a clean exit.
		// NOTE: May want to do it with a GC pass?
		for( auto It( Types_.begin() ); It != Types_.end(); ++It )
		{
			delete (*It).second;
		}

		Types_.clear();
	}

    ReType* GetType( BcName Name )
	{
		// Try find class.
		TypeMap::iterator FoundTypeIt = Types_.find( Name );
        ReType* FoundType = nullptr;
		if( FoundTypeIt != Types_.end() )
		{
			FoundType = FoundTypeIt->second;
		}

		return FoundType;
	}

	ReClass* GetClass( BcName Name )
	{
		// If it doesn't begin with Class, prepend it.
		// HACK HACK
		if( (*Name).substr( 0, 5 ) != "class" &&
			(*Name).substr( 0, 6 ) != "struct" &&
			(*Name).substr( 0, 4 ) != "enum" )
		{
			Name = BcName( std::string( "class " + *Name ) );
		}

		// Try find class.
        ReType* FoundType = GetType( Name );
		if( FoundType == nullptr )
		{
			FoundType = new ReClass( Name );
			Types_[ Name ] = FoundType;
		}

		if( FoundType->isTypeOf< ReClass >() )
		{
			return static_cast< ReClass* >( FoundType );
		}
		else
		{
			return nullptr;
		}
	}

	void GetClassesOfBase( std::vector< const ReClass* >& OutClasses, const ReClass* Base )
	{
		for( auto TypeIter : Types_ )
		{
			auto Type = TypeIter.second;
			if( Type->isTypeOf< ReClass >() )
			{
				const ReClass* OutClass = static_cast< const ReClass* >( Type );
				if( OutClass->hasBaseClass( Base ) )
				{
					OutClasses.push_back( OutClass );
				}
			}
		}
	}

	ReEnum* GetEnum( BcName Name )
	{
		// Try find class.
        ReType* FoundType = GetType( Name );
		if( FoundType == nullptr )
		{
			FoundType = new ReEnum( Name );
			Types_[ Name ] = FoundType;
		}

		if( FoundType->isTypeOf< ReEnum >() )
		{
			return static_cast< ReEnum* >( FoundType );
		}
		else
		{
			return nullptr;
		}
	}

	TypeMap Types_;
};

static BcBool IsInitialised_ = false;
static ReClassSerialiser* ClassSerialisers_ = nullptr;

//////////////////////////////////////////////////////////////////////////
// Init
void ReManager::Init()
{
	// Don't initialsise twice.
	if( IsInitialised_ )
	{
		return;
	}

	// Setup basic types.
	GetClass( "BcU8" )->setType< BcU8 >( new ReClassSerialiser_BasicType< BcU8 >( "BcU8" ) );
	GetClass( "BcU16" )->setType< BcU16 >( new ReClassSerialiser_BasicType< BcU16 >( "BcU16" ) );
	GetClass( "BcU32" )->setType< BcU32 >( new ReClassSerialiser_BasicType< BcU32 >( "BcU32" ) );
	GetClass( "BcU64" )->setType< BcU64 >( new ReClassSerialiser_BasicType< BcU64 >( "BcU64" ) );
	GetClass( "BcS8" )->setType< BcS8 >( new ReClassSerialiser_BasicType< BcS8 >( "BcS8" ) );
	GetClass( "BcS16" )->setType< BcS16 >( new ReClassSerialiser_BasicType< BcS16 >( "BcS16" ) );
	GetClass( "BcS32" )->setType< BcS32 >( new ReClassSerialiser_BasicType< BcS32 >( "BcS32" ) );
	GetClass( "BcS64" )->setType< BcS64 >( new ReClassSerialiser_BasicType< BcS64 >( "BcS64" ) );
	GetClass( "BcF32" )->setType< BcF32 >( new ReClassSerialiser_BasicType< BcF32 >( "BcF32" ) );
	GetClass( "BcF64" )->setType< BcF64 >( new ReClassSerialiser_BasicType< BcF64 >( "BcF64" ) );
	GetClass( "BcBool" )->setType< BcBool >( new ReClassSerialiser_BasicType< BcBool >( "BcBool" ) );
	GetClass( "string" )->setType< std::string >( new ReClassSerialiser_StringType( "string" ) );
	GetClass( "class BcBinaryData" )->setType< BcBinaryData >( new ReClassSerialiser_BinaryDataType( "class BcBinaryData" ) );

    RePrimitive::StaticRegisterClass();
    ReType::StaticRegisterClass();
	ReClass::StaticRegisterClass();
	ReEnum::StaticRegisterClass();
	ReEnumConstant::StaticRegisterClass();
	ReField::StaticRegisterClass();
    ReObject::StaticRegisterClass();
			

	IsInitialised_ = true;
}

//////////////////////////////////////////////////////////////////////////
// Fini
void ReManager::Fini()
{
	IsInitialised_ = false;
}
	
//////////////////////////////////////////////////////////////////////////
// GetType
ReType* ReManager::GetType( BcName Name )
{
	return Factory::Instance().GetType( Name );		
}

//////////////////////////////////////////////////////////////////////////
// GetClass
ReClass* ReManager::GetClass( BcName Name )
{
	return Factory::Instance().GetClass( Name );		
}

//////////////////////////////////////////////////////////////////////////
// GetClasses
std::vector< ReClass* > ReManager::GetClasses()
{
	std::vector< ReClass* > Ret;
	for( auto& It : Factory::Instance().Types_ )
	{
		if( It.second->isTypeOf< ReClass >() )
		{
			Ret.push_back( static_cast< ReClass* >( It.second ) );
		}
	}
	return Ret;
}


//////////////////////////////////////////////////////////////////////////
// GetEnum
ReEnum* ReManager::GetEnum( BcName Name )
{
	return Factory::Instance().GetEnum( Name );		
}

//////////////////////////////////////////////////////////////////////////
// GetClassesOfBase
void ReManager::GetClassesOfBase( std::vector< const ReClass* >& OutClasses, const ReClass* Base )
{
	return Factory::Instance().GetClassesOfBase( OutClasses, Base );		
}
