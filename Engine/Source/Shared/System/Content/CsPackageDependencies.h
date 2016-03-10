#pragma once

#include "System/Content/CsTypes.h"
#include "System/Content/CsPlatformParams.h"

#include "Reflection/ReReflection.h"

#include <map>
#include <set>

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

