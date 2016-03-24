#include "Psybrus.h"
#include "System/Content/CsCore.h"
#include "System/Content/CsPackageImporter.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"

#include "System/File/FsCore.h"
#include "System/SysKernel.h"

#include "Serialisation/SeJsonReader.h"

#include "Base/BcCommandLine.h"
#include "Base/BcFile.h"

#include <vector>
#include <iostream>

#if PLATFORM_LINUX || PLATFORM_OSX
#include <dirent.h>
#elif PLATFORM_WINDOWS
#include <filesystem>
#endif

//////////////////////////////////////////////////////////////////////////
// GPsySetupParams
PsySetupParams GPsySetupParams( "Importer", psySF_IMPORTER );	

//////////////////////////////////////////////////////////////////////////
// FindPackages
std::vector< BcPath > FindPackages( const CsPlatformParams& Params )
{
	std::vector< BcPath > FoundPackages;

#if PLATFORM_LINUX || PLATFORM_OSX
	if( DIR* SearchDir = opendir( "Content" ) )
	{
		dirent Data;
		dirent* FoundEntry = nullptr;
		while( readdir_r( SearchDir, &Data, &FoundEntry ) == 0 && FoundEntry != nullptr )
		{
			if( strstr( FoundEntry->d_name, ".pkg" ) )
			{
				std::array< char, 256 > FullPath;
				BcSPrintf( FullPath.data(), FullPath.size(), "Content/%s", FoundEntry->d_name );
				FoundPackages.emplace_back( FullPath.data() );
			}
		}
		closedir(SearchDir);
	}
#elif PLATFORM_WINDOWS
	using namespace std::experimental::filesystem;
	path Path( "Content/" );

	auto It = directory_iterator( Path );
	while( It != directory_iterator() )
	{
		directory_entry Entry = *It;
		if( Entry.path().extension().string()  == ".pkg" )
		{
			BcPath PackagePath( Entry.path().string() );
			FoundPackages.emplace_back( PackagePath );
		}
		++It;
	}
#endif

	PSY_LOG( "Found %u packages.", FoundPackages.size() );
	
	return FoundPackages;
}

//////////////////////////////////////////////////////////////////////////
// CheckPackages
std::vector< BcPath > CheckPackages( const CsPlatformParams& Params, const std::vector< BcPath >& PackagePaths )
{
	std::vector< BcPath > CheckedPackages;

	for( const auto& PackagePath : PackagePaths )
	{
		BcBool ShouldImport = BcFalse;

		auto PackageName = PackagePath.getFileNameNoExtension();

		// Read in dependencies.
		FsStats Stats;
		std::string PackedPackage = *Params.getPackagePackedPath( PackageName );
		std::string OutputDependencies = *Params.getPackageIntermediatePath( PackageName ) + "/deps.json";

		// Import package if it doesn't exist.
		if( !BcFileSystemExists( PackedPackage.c_str() ) )
		{
			ShouldImport = BcTrue;
		}

		// Import package & output dependencies changed?
		if( BcFileSystemExists( (*PackagePath).c_str() ) )
		{
			if(	BcFileSystemExists( OutputDependencies.c_str() ) )
			{
				PSY_LOG( "Found dependency info \"%s\", checking if we need to build.\n", OutputDependencies.c_str() );
				PSY_LOGSCOPEDINDENT;
				
				CsPackageDependencies Dependencies;
				CsSerialiserPackageObjectCodec ObjectCodec( nullptr, (BcU32)bcRFF_ALL, (BcU32)bcRFF_TRANSIENT, 0 );
				SeJsonReader Reader( &ObjectCodec );
				Reader.load( OutputDependencies.c_str() );
				Reader << Dependencies;
				ShouldImport |= Dependencies.haveChanged();
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
			PSY_LOG( "ERROR: Can't find package \"%s\", skipping dependency checking.\n", (*PackagePath).c_str() );
		}

		if( ShouldImport )
		{
			CheckedPackages.emplace_back( PackagePath );
		}
	}

	return CheckedPackages;
}

//////////////////////////////////////////////////////////////////////////
// ImportPackages
void ImportPackages( const CsPlatformParams& Params, const std::vector< BcPath >& PackagePaths )
{
	for( const auto& PackagePath : PackagePaths )
	{
		auto PackageName = PackagePath.getFileNameNoExtension();

		CsPackageImporter Importer( Params, PackageName, PackagePath );
		try
		{
			BcBool ImportSucceeded = Importer.import();
			if( !ImportSucceeded )
			{
				PSY_LOG( "ERROR: Failure importing %s (%s)", 
					PackageName.c_str(), PackagePath.c_str() );
				exit( 1 );
			}
		}
		catch( CsImportException Exception )
		{
			PSY_LOG( "ERROR: Failure importing %s (%s):\t%s:\n %s", 
				PackageName.c_str(), PackagePath.c_str(), 
				Exception.file(), Exception.error() );
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// PrintUsage
void PrintUsage()
{
	PSY_LOG( "Args:" );

	PSY_LOG( "\t-c/--config-file [file] - Load config file to use for platform parameters." );
	PSY_LOG( "\t-p/--package-file [file] - Individual package to import." );

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
	CsPlatformParams Params;

	std::string ConfigFile;
	if( !GCommandLine_.getArg( 'c', "config-file", ConfigFile ) )
	{
		PrintUsage();

		ConfigFile = "../Psybrus/Dist/Platforms/pc.json";
		PSY_LOG( "WARNING: No config file specified." );
		PSY_LOG( "INFO: Using default for PC: %s", ConfigFile.c_str() );
	}

	// Try loading params file.
	if( BcFileSystemExists( ConfigFile.c_str() ) )
	{
		CsSerialiserPackageObjectCodec ObjectCodec( nullptr, (BcU32)bcRFF_ALL, (BcU32)bcRFF_TRANSIENT, 0 );
		SeJsonReader Reader( &ObjectCodec );
		Reader.load( ConfigFile.c_str() );
		Reader << Params;

		if( Params.Name_.size() == 0 )
		{
			PSY_LOG( "ERROR: Config file params are missing name." );
			exit( 1 );
		}
		if( Params.Filters_.size() == 0 )
		{
			PSY_LOG( "ERROR: Config file params are missing filters." );
			exit( 1 );
		}
		if( Params.IntermediatePath_.size() == 0 )
		{
			PSY_LOG( "ERROR: Config file params are missing intermediatepath." );
			exit( 1 );
		}
		if( Params.PackedContentPath_.size() == 0 )
		{
			PSY_LOG( "ERROR: Config file params are missing packedcontentpath." );
			exit( 1 );
		}

		PSY_LOG( "INFO: Loaded %s. Packing for platform %s.", ConfigFile.c_str(), Params.Name_.c_str() );
	}
	else
	{
		PSY_LOG( "ERROR: Unable to load config %s.", ConfigFile.c_str() );
		exit( 1 );
	}

	std::vector< BcPath > Packages;

	// If no package is specified, then search.
	std::string PackageName;
	if( GCommandLine_.getArg( 'p', "package-file", PackageName ) )
	{
		Packages.emplace_back( PackageName );

	}
	else
	{
		Packages = FindPackages( Params );
	}

	// Check packages, filter out what we don't want, or what doesn't need updating.
	Packages = CheckPackages( Params, Packages );

	// Log.
	for( const auto& Package : Packages )
	{
		PSY_LOG( "INFO: Reimporting %s.", Package.c_str() );

	}

	// Import all packages to pass check phase.
	ImportPackages( Params, Packages );
}
