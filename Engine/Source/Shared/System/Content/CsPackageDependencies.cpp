#include "System/Content/CsPackageDependencies.h"

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
		BcU32 Hash = Class->getHash();
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
		if( ClassDependency.second != Class->getHash() )
		{
			PSY_LOG( "INFO: Class \"%s\" has changed.", (*Class->getName()).c_str() );
			return true;
		}
	}

	return false;
}
