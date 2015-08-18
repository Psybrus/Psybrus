#include "System/Content/CsPlatformParams.h"

#include "Base/BcHash.h"

#include <regex>

#if PSY_IMPORT_PIPELINE
#include <boost/filesystem.hpp>
#endif // PSY_IMPORT_PIPELINE

//////////////////////////////////////////////////////////////////////////
// Regex
static std::regex GRegex_Filter( "^@\\((.*?)\\)" );								// Matches "(String)".

//////////////////////////////////////////////////////////////////////////
// CsPackageDependencies
REFLECTION_DEFINE_BASIC( CsPlatformParams );

void CsPlatformParams::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "Name_", &CsPlatformParams::Name_ ),
		new ReField( "Filters_", &CsPlatformParams::Filters_ ),
		new ReField( "IntermediatePath_", &CsPlatformParams::IntermediatePath_ ),
		new ReField( "PackedContentPath_", &CsPlatformParams::PackedContentPath_ ),
	};

	ReRegisterClass< CsPlatformParams >( Fields );
};

//////////////////////////////////////////////////////////////////////////
// checkFilterString
BcBool CsPlatformParams::checkFilterString( const std::string& InFilter ) const
{
	std::cmatch Match;
	std::regex_match( InFilter.c_str(), Match, GRegex_Filter );
	if( Match.size() != 2 )
	{
		return BcFalse;
	}
	std::stringstream StrStream( Match[ 1 ] );
	std::string Token;
	while( std::getline( StrStream, Token, ',' ) )
	{
		auto It = std::find( Filters_.begin(), Filters_.end(), Token );
		if( It == Filters_.end() )
		{
			return BcFalse;
		}
	}

	return BcTrue;
}

//////////////////////////////////////////////////////////////////////////
// getPackageIntermediatePath
BcPath CsPlatformParams::getPackageIntermediatePath( const BcName& Package ) const
{
	BcPath Path;
	if( Package != BcName::INVALID )
	{
		Path.join( IntermediatePath_, *Package + ".pak" );
	}
	else
	{
		Path = IntermediatePath_;
	}

#if !PLATFORM_HTML5 && !PLATFORM_ANDROID
	boost::filesystem::create_directories( *Path );
#endif // !PLATFORM_HTML5 && !PLATFORM_ANDROID

	return Path;
}

//////////////////////////////////////////////////////////////////////////
// getPackagePackedPath
BcPath CsPlatformParams::getPackagePackedPath( const BcName& Package ) const
{
	BcPath Path;
	if( Package != BcName::INVALID )
	{
		Path.join( PackedContentPath_, *Package + ".pak" );
	}
	else
	{
		Path = PackedContentPath_;
	}
	return Path;
}
