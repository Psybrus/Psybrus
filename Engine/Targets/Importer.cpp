#include "Psybrus.h"
#include "System/Content/CsCore.h"
#include "System/Content/CsPackageImporter.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "System/File/FsCore.h"

#include "Serialisation/SeJsonReader.h"

#include <iostream>
#include <boost/filesystem.hpp>

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Importer", psySF_IMPORTER );	

//////////////////////////////////////////////////////////////////////////
// FindPackages
std::vector< BcPath > FindPackages( const CsPackageImportParams& Params )
{
	return {};
}

//////////////////////////////////////////////////////////////////////////
// ImportPackage
void ImportPackage( const CsPackageImportParams& Params, const BcPath& ImportPackage )
{
	auto PackageName = ImportPackage.getFileNameNoExtension();

	// Read in dependencies.
	FsStats Stats;
	std::string PackedPackage = *Params.getPackagePackedPath( PackageName );
	std::string OutputDependencies = *Params.getPackageIntermediatePath( PackageName ) + "/deps.json";
	BcBool ShouldImport = BcFalse;

	// Import package if it doesn't exist.
	if( !boost::filesystem::exists( PackedPackage ) )
	{
		ShouldImport = BcTrue;
	}

	// Import package & output dependencies changed?
	if( boost::filesystem::exists( *ImportPackage ) )
	{
		if(	boost::filesystem::exists( OutputDependencies ) )
		{
			PSY_LOG( "Found dependency info \"%s\", checking if we need to build.\n", OutputDependencies.c_str() );
			PSY_LOGSCOPEDINDENT;

			CsPackageDependencies Dependencies;

			CsSerialiserPackageObjectCodec ObjectCodec( nullptr, (BcU32)bcRFF_ALL, (BcU32)bcRFF_TRANSIENT, 0 );
			SeJsonReader Reader( &ObjectCodec );
			Reader.load( OutputDependencies.c_str() );
			Reader << Dependencies;

			// Check other dependencies.
			if( !ShouldImport )
			{
				for( const auto& Dependency : Dependencies.Dependencies_ )
				{
					if( Dependency.hasChanged() )
					{
						PSY_LOG( "WARNING: \"%s\" has changed.\n", Dependency.getFileName().c_str() );
						ShouldImport = BcTrue;
						break;
					}
					else
					{
						PSY_LOG( "\"%s\" has not changed.\n", Dependency.getFileName().c_str() );						
					}
				}
			}
		}
		else
		{
			PSY_LOG( "WARNING: Can't find package dependency info \"%s\", perform full build.\n", OutputDependencies.c_str() );

			// No deps file, assume worst.
			ShouldImport = BcTrue;
		}
	}
	else
	{
		PSY_LOG( "ERROR: Can't find package \"%s\", skipping dependency checking.\n", (*ImportPackage).c_str() );
	}

	// Reimport.
	if( ShouldImport )
	{
		CsPackageImporter Importer( Params, PackageName, ImportPackage );
		try
		{
			BcBool ImportSucceeded = Importer.import();
			if( !ImportSucceeded )
			{
				PSY_LOG( "Failure importing" );
				exit( 1 );
			}
		}
		catch( CsImportException Exception )
		{
			PSY_LOG( "ERROR: %s:\n %s", Exception.file(), Exception.error() );
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// PsyToolInit
void PsyToolInit()
{

}

//////////////////////////////////////////////////////////////////////////
// PsyToolMain
void PsyToolMain()
{
	using namespace boost::filesystem;
	path Path( "Content/" );

	CsPackageImportParams Params;
	Params.Filters_.push_back( "pc" );
	Params.Filters_.push_back( "high" );

	Params.IntermediatePath_ = "Intermediate";
	Params.PackedContentPath_ = "PackedContent";

	auto It = directory_iterator( Path );
	while( It != directory_iterator() )
	{
		directory_entry Entry = *It;
		std::cout << Entry.path().extension().string()  << std::endl;
		if( Entry.path().extension().string()  == ".pkg" )
		{
			BcPath PackagePath( Entry.path().string() );
			ImportPackage( Params, PackagePath );
		}
		++It;
	}
}
