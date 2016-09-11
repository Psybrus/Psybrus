#include "System/Content/CsPlatformParams.h"

#include "Base/BcFile.h"
#include "Base/BcHash.h"

#include <regex>

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
		new ReField( "ImportParams_", &CsPlatformParams::ImportParams_ ),
	};

	ReRegisterClass< CsPlatformParams >( Fields );
};

//////////////////////////////////////////////////////////////////////////
// Ctor
CsPlatformParams::CsPlatformParams()
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
CsPlatformParams::~CsPlatformParams()
{
	for( auto ImportParams : ImportParams_ )
	{
		delete ImportParams;
	}
	ImportParams_.clear();
}

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

#if PSY_IMPORT_PIPELINE
	BcFileSystemCreateDirectories( (*Path).c_str() );
#endif // PSY_IMPORT_PIPELINE

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

//////////////////////////////////////////////////////////////////////////
// getImportParams
const ReObject* CsPlatformParams::getImportParams( const ReClass* Class ) const
{
	for( auto ImportParams : ImportParams_ )
	{
		if( ImportParams->isTypeOf( Class ) )
		{
			return ImportParams;
		}
	}
	return nullptr;
}
