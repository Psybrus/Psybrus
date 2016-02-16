#include "Reflection/ReManager.h"

#include "Reflection/ReClass.h"
#include "Reflection/ReClassSerialiser_BasicType.h"
#include "Reflection/ReClassSerialiser_NameType.h"
#include "Reflection/ReClassSerialiser_StringType.h"
#include "Reflection/ReClassSerialiser_BinaryDataType.h"
#include "Reflection/ReEnum.h"
#include "Reflection/ReEnumConstant.h"

#include "Base/BcMisc.h"

//////////////////////////////////////////////////////////////////////////
// Definitions
REFLECTION_DEFINE_BASE( ReITypeSerialiser );

// TODO: Move into Manager class.
			
typedef std::map< BcName, ReClass* > TypeMap;

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

	ReClass* GetType( BcName Name )
	{
		// Try find class.
		TypeMap::iterator FoundTypeIt = Types_.find( Name );

		ReClass* FoundType = nullptr;
		if( FoundTypeIt != Types_.end() )
		{
			FoundType = FoundTypeIt->second;
		}

		return FoundType;
	}

	ReClass* GetClass( BcName Name )
	{
		auto NameString = (*Name);

		// If it begins with "class", "struct", or "enum", strip it.
		// NOTE: We should move this work into the demangling stuff.
		if( NameString.substr( 0, 6 ) == "class " ||
			NameString.substr( 0, 7 ) == "struct " ||
			NameString.substr( 0, 5 ) == "enum " )
		{
			Name = BcName( NameString.substr( NameString.find( " " ) + 1, NameString.length() - 1 ) );
		}

		// Try find class.
		ReClass* FoundType = GetType( Name );
		if( FoundType == nullptr )
		{
			BcAssertMsg( BcIsGameThread(), "Reflection can only modify database on the game thread." );
			FoundType = new ReClass( Name );
			Types_[ Name ] = FoundType;
		}

		// Use dynamic cast here instead of our internal RTTI.
		// It may still be under construction so not yet valid.
		return dynamic_cast< ReClass* >( FoundType );
	}

	void GetClassesOfBase( std::vector< const ReClass* >& OutClasses, const ReClass* Base )
	{
		for( auto TypeIter : Types_ )
		{
			auto Type = TypeIter.second;
			const ReClass* OutClass = static_cast< const ReClass* >( Type );
			if( OutClass->hasBaseClass( Base ) )
			{
				OutClasses.push_back( OutClass );
			}
		}
	}

	ReEnum* GetEnum( BcName Name )
	{
		auto NameString = ( *Name );

		// If it begins with "class", "struct", or "enum", strip it.
		// NOTE: We should move this work into the demangling stuff.
		if( NameString.substr( 0, 5 ) == "enum " )
		{
			Name = BcName( NameString.substr( NameString.find( " " ) + 1, NameString.length() - 1 ) );
		}

		// Try find class.
		ReClass* FoundType = GetType( Name );
		if( FoundType == nullptr )
		{
			BcAssertMsg( BcIsGameThread(), "Reflection can only modify database on the game thread." );
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
	GetClass( "BcSize" )->setType< BcSize >( new ReClassSerialiser_BasicType< BcSize >( "BcSize" ) );
	GetClass( "size_t" )->setType< size_t >( new ReClassSerialiser_BasicType< size_t >( "size_t" ) );
	GetClass( "BcBool" )->setType< BcBool >( new ReClassSerialiser_BasicType< BcBool >( "BcBool" ) );
	GetClass( "bool" )->setType< bool >( new ReClassSerialiser_BasicType< bool >( "bool" ) );
	GetClass( "string" )->setType< std::string >( new ReClassSerialiser_StringType( "string" ) );
	GetClass( "class BcName" )->setType< BcName >( new ReClassSerialiser_NameType( "class BcName" ) );
	GetClass( "class BcBinaryData" )->setType< BcBinaryData >( new ReClassSerialiser_BinaryDataType( "class BcBinaryData" ) );

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
ReClass* ReManager::GetType( BcName Name )
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
		Ret.push_back( static_cast< ReClass* >( It.second ) );
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
// GetEnums
std::vector< ReEnum* > ReManager::GetEnums()
{
	std::vector< ReEnum* > Ret;
	for( auto& It : Factory::Instance().Types_ )
	{
		if( It.second->isTypeOf< ReEnum >() )
		{
			Ret.push_back( static_cast< ReEnum* >( It.second ) );
		}
	}
	return Ret;
}


//////////////////////////////////////////////////////////////////////////
// GetClassesOfBase
void ReManager::GetClassesOfBase( std::vector< const ReClass* >& OutClasses, const ReClass* Base )
{
	return Factory::Instance().GetClassesOfBase( OutClasses, Base );		
}
