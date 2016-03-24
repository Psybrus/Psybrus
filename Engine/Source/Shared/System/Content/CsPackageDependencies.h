#pragma once

#include "System/Content/CsTypes.h"
#include "System/Content/CsPlatformParams.h"
#include "System/Content/CsResourceImporter.h"

#include "Reflection/ReReflection.h"

#include <map>
#include <set>

#if 0
//////////////////////////////////////////////////////////////////////////
// CsDependencyAttribute
class CsDependencyAttribute :
	public ReAttribute
{
public:
	//REFLECTION_DECLARE_DERIVED( CsDependencyAttribute, ReAttribute );

	/**
	 * Construct with a dependency name. Hashed when dependency checking.
	 * @param StaticDependencyName Name of dependency. Must be a literal string.
	 */
	CsDependencyAttribute( const char* DependencyString );

	CsResourceImporterUPtr getImporter() const;
	BcU32 getVersionId() const;
	BcU32 getPriority() const;

private:
	const ReClass* ImporterClass_;
	BcU32 VersionId_;
	BcU32 Priority_;
};
#endif


//////////////////////////////////////////////////////////////////////////
// CsPackageDependencies
struct CsPackageDependencies
{
	REFLECTION_DECLARE_BASIC( CsPackageDependencies );

	CsPackageDependencies(){};

	void addClass( const ReClass* Class );

	bool haveChanged() const;

	typedef std::set< CsDependency > TDependencyList;
	typedef std::map< BcName, BcU32 > TClassHashMap;
	
	TDependencyList Dependencies_;
	TClassHashMap ClassDependencies_;
};

