#pragma once

#include "Base/BcTypes.h"
#include "Reflection/ReReflection.h"

#include "Base/BcName.h"
#include "Base/BcPath.h"

//////////////////////////////////////////////////////////////////////////
// CsPlatformParams
struct CsPlatformParams
{
	REFLECTION_DECLARE_BASIC( CsPlatformParams );

	CsPlatformParams(){};

	/**
	 * Check filter string vs import filters.
	 * If @a Filters_ contains "a", "b", and "c", the result for the
	 * following @a InFilters will be:
	 * - "(a)" - true
	 * - "(d)" - false
	 * - "(a,b)" - true
	 * - "(a,b,c)" - true
	 * - "(a,b,d)" - false
	 * - "(a,b,c,d)" - false
	 * @return true if all filters in string are in @a Filters_.
	 */
	BcBool checkFilterString( const std::string& InFilter ) const;

	/**
	 * Get package intermediate path.
	 * @param Package Name of package.
	 * @return Path to folder of intermediates for @a Package.
	 */
	BcPath getPackageIntermediatePath( const BcName& Package ) const;

	/**
	 * Get package packed path.
	 * @param Package Name of package. BcName::INVALID to get just packed path.
	 * @return Path to packed package file.
	 */
	BcPath getPackagePackedPath( const BcName& Package ) const;

	/// Name.
	std::string Name_;

	/// Used to filter out object properties in the "(filter1,filter2)" blocks.
	std::vector< std::string > Filters_;

	/// Intermediate path.
	std::string IntermediatePath_;

	/// Packed content path.
	std::string PackedContentPath_;
};

