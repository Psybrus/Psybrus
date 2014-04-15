#include "Reflection/ReManager.h"

#include "Reflection/ReClass.h"
#include "Reflection/ReClassSerialiser_BasicType.h"
#include "Reflection/ReClassSerialiser_StringType.h"
#include "Reflection/ReClassSerialiser_BinaryDataType.h"
#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_BASE( ITypeSerialiser );

// TODO: Move into Manager class.
			
typedef std::map< std::string, Type* > TypeMap;

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

	Type* GetType( const std::string& Name )
	{
		// Try find class.
		TypeMap::iterator FoundTypeIt = Types_.find( Name );
		Type* FoundType = nullptr;
		if( FoundTypeIt != Types_.end() )
		{
			FoundType = FoundTypeIt->second;
		}

		return FoundType;
	}

	Class* GetClass( const std::string& Name )
	{
		// Try find class.
		Type* FoundType = GetType( Name );
		if( FoundType == nullptr )
		{
			FoundType = new Class( Name );
			Types_[ Name ] = FoundType;
		}

		if( FoundType->isTypeOf< Class >() )
		{
			return static_cast< Class* >( FoundType );
		}
		else
		{
			return nullptr;
		}
	}

	void GetClassesOfBase( std::vector< const Class* >& OutClasses, const Class* Base )
	{
		for( auto TypeIter : Types_ )
		{
			auto Type = TypeIter.second;
			if( Type->isTypeOf< Class >() )
			{
				const Class* OutClass = static_cast< const Class* >( Type );
				if( OutClass->hasBaseClass( Base ) )
				{
					OutClasses.push_back( OutClass );
				}
			}
		}
	}

	Enum* GetEnum( const std::string& Name )
	{
		// Try find class.
		Type* FoundType = GetType( Name );
		if( FoundType == nullptr )
		{
			FoundType = new Enum( Name );
			Types_[ Name ] = FoundType;
		}

		if( FoundType->isTypeOf< Enum >() )
		{
			return static_cast< Enum* >( FoundType );
		}
		else
		{
			return nullptr;
		}
	}

	TypeMap Types_;
};

static BcBool IsInitialised_ = false;
static ClassSerialiser* ClassSerialisers_ = nullptr;

//////////////////////////////////////////////////////////////////////////
// Init
void Init()
{
	// Don't initialsise twice.
	if( IsInitialised_ )
	{
		return;
	}

	// Setup basic types.
	GetClass( "BcU8" )->setType< BcU8 >( new ClassSerialiser_BasicType< BcU8 >( "BcU8" ) );
	GetClass( "BcU16" )->setType< BcU16 >( new ClassSerialiser_BasicType< BcU16 >( "BcU16" ) );
	GetClass( "BcU32" )->setType< BcU32 >( new ClassSerialiser_BasicType< BcU32 >( "BcU32" ) );
	GetClass( "BcU64" )->setType< BcU64 >( new ClassSerialiser_BasicType< BcU64 >( "BcU64" ) );
	GetClass( "BcS8" )->setType< BcS8 >( new ClassSerialiser_BasicType< BcS8 >( "BcS8" ) );
	GetClass( "BcS16" )->setType< BcS16 >( new ClassSerialiser_BasicType< BcS16 >( "BcS16" ) );
	GetClass( "BcS32" )->setType< BcS32 >( new ClassSerialiser_BasicType< BcS32 >( "BcS32" ) );
	GetClass( "BcS64" )->setType< BcS64 >( new ClassSerialiser_BasicType< BcS64 >( "BcS64" ) );
	GetClass( "BcF32" )->setType< BcF32 >( new ClassSerialiser_BasicType< BcF32 >( "BcF32" ) );
	GetClass( "BcF64" )->setType< BcF64 >( new ClassSerialiser_BasicType< BcF64 >( "BcF64" ) );
	GetClass( "BcBool" )->setType< BcBool >( new ClassSerialiser_BasicType< BcBool >( "BcBool" ) );
	GetClass( "string" )->setType< std::string >( new ClassSerialiser_StringType( "string" ) );
	GetClass( "class BcBinaryData" )->setType< BcBinaryData >( new ClassSerialiser_BinaryDataType( "class BcBinaryData" ) );

	Primitive::StaticRegisterClass();
	Type::StaticRegisterClass();
	Class::StaticRegisterClass();
	Enum::StaticRegisterClass();
	EnumConstant::StaticRegisterClass();
	Field::StaticRegisterClass();
	Object::StaticRegisterClass();
			

	IsInitialised_ = true;
}

//////////////////////////////////////////////////////////////////////////
// Fini
void Fini()
{
	IsInitialised_ = false;
}
	
//////////////////////////////////////////////////////////////////////////
// GetType
Type* GetType( const std::string& Name )
{
	return Factory::Instance().GetType( Name );		
}

//////////////////////////////////////////////////////////////////////////
// GetClass
Class* GetClass( const std::string& Name )
{
	return Factory::Instance().GetClass( Name );		
}

//////////////////////////////////////////////////////////////////////////
// GetEnum
Enum* GetEnum( const std::string& Name )
{
	return Factory::Instance().GetEnum( Name );		
}

//////////////////////////////////////////////////////////////////////////
// GetClassesOfBase
void GetClassesOfBase( std::vector< const Class* >& OutClasses, const Class* Base )
{
	return Factory::Instance().GetClassesOfBase( OutClasses, Base );		
}
