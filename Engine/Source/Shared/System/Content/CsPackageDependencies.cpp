#include "System/Content/CsPackageDependencies.h"

namespace
{
BcU32 getClassHash( const ReClass* Class )
{
	const std::string ClassName = *Class->getName();

	BcU32 CRC32 = Class->getSuper() ? getClassHash( Class->getSuper() ) : 0;
	CRC32 = BcHash::GenerateCRC32( CRC32, ClassName.data(), ClassName.size() );

	BcU32 Flags = Class->getFlags();
	CRC32 = BcHash::GenerateCRC32( CRC32, &Flags, sizeof( Flags ) );
	if( Class->getFlags() & bcRFF_POD )
	{
		size_t Size = Class->getSize();
		CRC32 = BcHash::GenerateCRC32( CRC32, &Size, sizeof( Size ) );
	}

	for( auto Field : Class->getFields() )
	{
		const std::string FieldName = *Field->getName();
		CRC32 = BcHash::GenerateCRC32( CRC32, FieldName.data(), FieldName.size() );

		if( Field->isContainer() )
		{
			if( Field->getKeyType() )
			{
				const std::string FieldTypeName = *Field->getKeyType()->getName();
				CRC32 = BcHash::GenerateCRC32( CRC32, FieldTypeName.data(), FieldTypeName.size() );
			}

			if( Field->getValueType() )
			{
				const std::string FieldTypeName = *Field->getValueType()->getName();
				CRC32 = BcHash::GenerateCRC32( CRC32, FieldTypeName.data(), FieldTypeName.size() );
			}
		}
		else
		{
			const std::string FieldTypeName = *Field->getType()->getName();
			CRC32 = BcHash::GenerateCRC32( CRC32, FieldTypeName.data(), FieldTypeName.size() );
			if( Field->getFlags() & bcRFF_POD )
			{
				const size_t FieldSize = Field->getSize();
				CRC32 = BcHash::GenerateCRC32( CRC32, &FieldSize, sizeof( FieldSize ) );
			}
		}
	}

	return CRC32;
}

BcU32 getEnumHash( const ReEnum* Enum )
{
	BcU32 CRC32 = getClassHash( Enum );
	for( auto EnumConstant : Enum->getEnumConstants() )
	{
		const std::string EnumConstantName = *EnumConstant->getName();
		const BcU32 EnumConstantValue = EnumConstant->getValue();
		CRC32 = BcHash::GenerateCRC32( CRC32, EnumConstantName.data(), EnumConstantName.size() );
		CRC32 = BcHash::GenerateCRC32( CRC32, &EnumConstantValue, sizeof( EnumConstantValue ) );
	}
	return CRC32;
}

BcU32 getHash( const ReClass* Class )
{
	if( Class->isTypeOf< ReEnum >() )
	{
		return getEnumHash( static_cast< const ReEnum* >( Class ) );
	}
	return getClassHash( Class );
}


}

//////////////////////////////////////////////////////////////////////////
// CsPackageDependencies
REFLECTION_DEFINE_BASIC( CsPackageDependencies );

void CsPackageDependencies::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Dependencies_", &CsPackageDependencies::Dependencies_ ),
		new ReField( "ClassDependencies_", &CsPackageDependencies::ClassDependencies_ ),
	};

	ReRegisterClass< CsPackageDependencies >( Fields );
};


//////////////////////////////////////////////////////////////////////////
// addClass
void CsPackageDependencies::addClass( const ReClass* Class )
{
	if( ClassDependencies_.find( Class->getName() ) == ClassDependencies_.end() )
	{
		BcU32 Hash = getHash( Class );
		ClassDependencies_.insert( std::make_pair( Class->getName(), Hash ) );

		// Recurse to add other dependent classes.
		for( auto Idx = 0; Idx < Class->getNoofFields(); ++Idx )
		{
			const ReField* Field = Class->getField( Idx );
			addClass( Field->getType() );

			// If it's a container, add key/value types.
			if( Field->isContainer() )
			{
				if( Field->getKeyType() )
				{
					addClass( Field->getKeyType() );
				}
				if( Field->getValueType() )
				{
					addClass( Field->getValueType() );
				}
			}
		}

		// Add super to include all the fields.
		if( Class->getSuper() )
		{
			addClass( Class->getSuper() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// haveChanged
bool CsPackageDependencies::haveChanged() const
{
	for( const auto& Dependency : Dependencies_ )
	{
		if( Dependency.hasChanged() )
		{
			PSY_LOG( "INFO: File \"%s\" has changed.", Dependency.getFileName().c_str() );
			return true;
		}
	}
	for( const auto& ClassDependency : ClassDependencies_ )
	{
		const ReClass* Class = ReManager::GetClass( ClassDependency.first );
		if( ClassDependency.second != getHash( Class ) )
		{
			PSY_LOG( "INFO: Class \"%s\" has changed.", (*Class->getName()).c_str() );
			return true;
		}
	}

	return false;
}
