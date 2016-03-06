/**************************************************************************
*
* File:		CsCore.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Content/CsCore.h"
#include "System/Content/CsSerialiserPackageObjectCodec.h"
#include "System/Content/CsRedirector.h"

#include "System/File/FsCore.h"

#include "Serialisation/SeJsonReader.h"

SYS_CREATOR( CsCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
CsCore::CsCore()
{
	IsCollectingGarbage_ = BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsCore::~CsCore()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void CsCore::open()
{
	// Register types for reflection.
	//CsResource::StaticRegisterReflection();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual 
void CsCore::update()
{
	processResources();
	freeUnreferencedPackages();
	processCallbacks();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual 
void CsCore::close()
{
	static BcF32 UnloadTimeout = 5.0f;
	BcTimer UnloadTimer;
	UnloadTimer.mark();
	
	do	
	{
		freeUnreferencedPackages();

		// Finish processing unloading resources.
		if( Resources_.size() > 0 )
		{
			processResources();
		}

		// If we hit the timeout, just break out and shut down cleanly.
		if( UnloadTimer.time() > UnloadTimeout )
		{
			break;
		}
	}
	while( PackageList_.size() > 0 || UnloadingPackageList_.size() > 0 );

	if( Resources_.size() > 0 )
	{
		PSY_LOG( "==========================================\n" );
		PSY_LOG( "CsCore: Dump Resource On Exit:\n" );
		PSY_LOG( "==========================================\n" );
	
		TResourceListIterator It( Resources_.begin() );
		while( It != Resources_.end() )
		{
			CsResource* pResource = (*It);
			PSY_LOG( "Init stage: %u, %s:%s \n", 
				pResource->getInitStage(), 
				(*pResource->getName()).c_str(), 
				(*pResource->getTypeName()).c_str() );
			++It;
		}
		PSY_LOG( "==========================================\n" );
	}

	// Verify we don't have any left floating loaded or unloading.
	BcVerifyMsg( ProcessingResources_.size() == 0, "CsCore: Resources still loaded, but system is closing! Has the scene cleaned up properly?" );
	BcVerifyMsg( Resources_.size() == 0, "CsCore: Resources still unloading, but system is closing!" );
}

//////////////////////////////////////////////////////////////////////////
// freeUnreferencedPackages
void CsCore::freeUnreferencedPackages()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Search for existing package and move to unloading list.
	for( TPackageListIterator It( PackageList_.begin() ); It != PackageList_.end();  )
	{
		CsPackage* pPackage = (*It);
		if( pPackage->hasUnreferencedResources() )
		{
			UnloadingPackageList_.emplace_back( pPackage );
			It = PackageList_.erase( It );
		}
		else
		{
			++It;
		}
	}

	// Work on unloading packages.
	for( TPackageListIterator It( UnloadingPackageList_.begin() ); It != UnloadingPackageList_.end();  )
	{
		CsPackage* pPackage = (*It);
		pPackage->releaseUnreferencedResources();

		// If we've got no valid resources we can move to unreferenced list to destroy.
		if( pPackage->haveAnyValidResources() == BcFalse )
		{
			UnreferencedPackageList_.push_back( pPackage );
			It = UnloadingPackageList_.erase( It );
		}
		else
		{
			++It;
		}
	}

	// NOTE: WIP.
	// Delete all unreferenced packages.
	for( TPackageListIterator It( UnreferencedPackageList_.begin() ); It != UnreferencedPackageList_.end(); ++It )
	{
		CsPackage* pPackage = (*It);

		delete pPackage;
	}

	// Clear unreferenced packages.
	UnreferencedPackageList_.clear();
}

//////////////////////////////////////////////////////////////////////////
// getResourceType
BcName CsCore::getResourceType( BcU32 Idx ) const
{
	// NOTE: Change the map to an array. We want fast lookups by index too :(
	return BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getNoofResourceTypes
BcU32 CsCore::getNoofResourceTypes() const
{
	return 0;
}

//////////////////////////////////////////////////////////////////////////
// allocResource
CsResource* CsCore::allocResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage )
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	CsResource* pResource = static_cast< CsResource* >( ReConstructObject( Class, *Name, pPackage, nullptr ) );
	pResource->setIndex( Index );
	
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// getNoofResources
size_t CsCore::getNoofResources()
{
	return Resources_.size();
}

//////////////////////////////////////////////////////////////////////////
// getResource
ReObjectRef< CsResource > CsCore::getResource( size_t Idx )
{
	return Resources_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getResource
ReObjectRef< CsResource > CsCore::getResource( const BcChar* pFullName )
{
	ReObjectRef< CsResource > Handle;
	if( pFullName != NULL )
	{
		BcChar FullNameBuffer[ 1024 ] = { 0 };
		BcAssertMsg( BcStrLength( pFullName ) < sizeof( FullNameBuffer ), "CsPackageImporter: Full name too long." );
		BcStrCopy( FullNameBuffer, sizeof( FullNameBuffer ), pFullName );
		BcChar* pPackageNameBuffer = NULL;
		BcChar* pResourceNameBuffer = NULL;
		BcChar* pTypeNameBuffer = NULL;

		BcAssertMsg( BcStrStr( FullNameBuffer, "." ) != NULL, "CsCore: Missing package from \"%s\"", FullNameBuffer );
		BcAssertMsg( BcStrStr( FullNameBuffer, ":" ) != NULL, "CsCore: Missing type from \"%s\"", FullNameBuffer );
		
		pPackageNameBuffer = &FullNameBuffer[ 0 ];
		pResourceNameBuffer = BcStrStr( FullNameBuffer, "." );
		pTypeNameBuffer = BcStrStr( FullNameBuffer, ":" );
		*pResourceNameBuffer++ = '\0';
		*pTypeNameBuffer++ = '\0';

		BcName PackageName = pPackageNameBuffer;
		BcName ResourceName = pResourceNameBuffer;

		internalFindResource( PackageName, ResourceName, ReManager::GetClass( pTypeNameBuffer ), Handle );

		BcAssertMsg( Handle.isValid(), "CsCore: Unable to find \"%s\"", FullNameBuffer );
	}

	return Handle;
}

//////////////////////////////////////////////////////////////////////////
// requestPackage
CsPackage* CsCore::requestPackage( const BcName& Package )
{
	CsPackage* pPackage = findPackage( Package );
	if( pPackage != NULL && !pPackage->isUnloading() )
	{
		return pPackage;
	}

	BcAssert( pPackage == nullptr || pPackage->isUnloading() );
	if( pPackage != nullptr )
	{
		PSY_LOG( "WARNING: Requesting package \"%s\", but it is currently unloading."
			"This is inefficient as it means reloading it against.",
			(*Package).c_str() );
	}

	// Check for a packed package.
	BcPath PackedPackage( *CsPaths::PACKED_CONTENT + "/" + *Package + ".pak" );
	BcBool PackageExists = FsCore::pImpl()->fileExists( PackedPackage.c_str() );

#if (PLATFORM_WINDOWS || PLATFORM_LINUX || PLATFORM_OSX) && !PSY_PRODUCTION
	// Non production builds should launch importer after a dependency check.
	bool ShouldImport = false;

	// Read in dependencies.
	FsStats Stats;
	BcPath OutputDependencies( *CsPaths::INTERMEDIATE + "/" + *Package + ".pak/deps.json" );

	// Import package & output dependencies changed?
	if( PackageExists )
	{
		if(	BcFileSystemExists( OutputDependencies.c_str() ) )
		{
			CsPackageDependencies Dependencies;
			CsSerialiserPackageObjectCodec ObjectCodec( nullptr, (BcU32)bcRFF_ALL, (BcU32)bcRFF_TRANSIENT, 0 );
			SeJsonReader Reader( &ObjectCodec );
			Reader.load( OutputDependencies.c_str() );
			Reader << Dependencies;
			ShouldImport |= Dependencies.haveChanged();
		}
		else
		{
			ShouldImport = true;
		}
	}
	else
	{
		ShouldImport = true;
	}

	// 
	if( ShouldImport )
	{
#if PSY_DEBUG
		const char* ImporterName = "Importer-Debug";
#endif
#if PSY_RELEASE
		const char* ImporterName = "Importer-Release";
#endif

#if PLATFORM_WINDOWS
		const char* Prefix = "";
		const char* Suffix = ".exe";
#else
		const char* Prefix = "./";
		const char* Suffix = "";
#endif

		bool Success = false;
		while( !Success )
		{
			// Grab log time.
			std::array< char, 1024 > LogTime;
			LogTime.fill( 0 );
			auto Time = std::time( nullptr );
			auto LocalTime = *std::localtime( &Time );
			strftime( LogTime.data(), LogTime.size(), "%Y-%m-%d-%H-%M-%S.png", &LocalTime );

			// Log filename.
			std::array< char, 1024 > LogFilename;
			LogFilename.fill( 0 );
			BcSPrintf( LogFilename.data(), LogFilename.size(), "%s_%s.log", ImporterName, LogTime.data() );

			// Build commandline.
			std::array< char, 1024 > CommandLine;
			CommandLine.fill( 0 );
			BcSPrintf( CommandLine.data(), CommandLine.size(), "%s%s%s -p %s/%s.pkg >> %s", 
				Prefix, ImporterName, Suffix, CsPaths::CONTENT.c_str(), (*Package).c_str(),
				LogFilename.data());

			PSY_LOG( "Launching importer for package \"%s\" with commandline: %s", (*Package).c_str(), CommandLine.data() );
			int RetCode = std::system( CommandLine.data() );
			if( RetCode != 0 )
			{
				std::array< char, 1024 > Message;
				Message.fill( 0 );
				BcSPrintf( Message.data(), Message.size(), "Importer failed.\nSee log \"%s\" for details.\n\nWould you like to re-run?", LogFilename.data() );
				auto RetVal = BcMessageBox( "ERROR", Message.data(), bcMBT_YESNOCANCEL, bcMBI_ERROR );

				if( RetVal == bcMBR_NO )
				{
					break;
				}
				else if( RetVal == bcMBR_CANCEL )
				{
					exit( RetCode );
				}
			}
			else
			{
				BcFileSystemRemove( LogFilename.data() );
				Success = true;
			}
		}
	}
#endif

	// If it exists, create it. Internally it will trigger it's own load.
	if( PackageExists )
	{
		pPackage = new CsPackage( Package, PackedPackage );
		PackageList_.push_back( pPackage );
	}
	else
	{
		BcAssertMsg( BcFalse, "CsCore: Can't import package, missing \"%s\"", PackedPackage.c_str() );
	}

	//
	return pPackage;	
}

//////////////////////////////////////////////////////////////////////////
// requestPackageReadyCallback
void CsCore::requestPackageReadyCallback( const BcName& Package, const CsPackageReadyCallback& Callback, BcU32 ID )
{
	TPackageReadyCallback PackageReadyCallback =
	{
		Package,
		Callback,
		ID
	};

	PackageReadyCallbackList_.push_back( PackageReadyCallback );
}

//////////////////////////////////////////////////////////////////////////
// findPackage
CsPackage* CsCore::findPackage( const BcName& Package )
{
	// Search for existing package.
	for( TPackageListIterator It( PackageList_.begin() ); It != PackageList_.end(); ++It )
	{
		if( (*It)->getName() == Package )
		{
			return (*It);
		}
	}

	return NULL;
}

//////////////////////////////////////////////////////////////////////////
// processResources
void CsCore::processResources()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Iterate processing resources.
	for( auto Resource : ProcessingResources_ )
	{
		switch( Resource->getInitStage() )
		{
		case CsResource::INIT_STAGE_INITIAL:
			{
				BcAssertMsg( 0, "Invalid init stage for processing." );
			}
			break;

		case CsResource::INIT_STAGE_CREATE:
			{
				BcAssertMsg( 0, "Invalid init stage for processing." );
			}
			break;

		case CsResource::INIT_STAGE_READY:
			{
				BcAssertMsg( 0, "Invalid init stage for processing." );
			}
			break;

		case CsResource::INIT_STAGE_DESTROY:
			{
				auto FoundIt = std::find( Resources_.begin(), Resources_.end(), Resource );
				BcAssert( FoundIt != Resources_.end() );
				Resources_.erase( FoundIt );
				delete Resource;
			}
			break;
		}
	}

	ProcessingResources_.clear();
}

//////////////////////////////////////////////////////////////////////////
// processCallbacks
void CsCore::processCallbacks()
{
	TPackageReadyCallbackListIterator It = PackageReadyCallbackList_.begin();

	while( It != PackageReadyCallbackList_.end() )
	{
		TPackageReadyCallback& CallbackData( *It );
		CsPackage* pPackage = findPackage( CallbackData.Package_ );
		if( pPackage != NULL &&
			pPackage->isReady() )
		{
			CallbackData.Callback_( pPackage, CallbackData.ID_ );
			It = PackageReadyCallbackList_.erase( It );
		}
		else
		{
			++It;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// internalAddResource
void CsCore::internalAddResource( CsResource* Resource )
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Put into resource list.
	BcAssert( std::find( Resources_.begin(), Resources_.end(), Resource ) == Resources_.end() );
	Resources_.push_back( Resource );
}

//////////////////////////////////////////////////////////////////////////
// internalAddResourceForProcessing
void CsCore::internalAddResourceForProcessing( CsResource* Resource )
{
	// Put into preprocess resources list.
	auto FoundIt = 
		std::find( 
			ProcessingResources_.begin(), 
			ProcessingResources_.end(), 
			Resource );

	if( FoundIt == ProcessingResources_.end() )
	{
		ProcessingResources_.push_back( Resource );
	}
}

//////////////////////////////////////////////////////////////////////////
// internalCreateResource
BcBool CsCore::internalCreateResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage, ReObjectRef< CsResource >& Handle )
{
	// Generate a unique name for the resource.
	BcName UniqueName = Name.isValid() ? Name : BcName( Class->getName() ).getUnique();

	// Allocate resource with a unique name.
	Handle = allocResource( UniqueName, Class, Index, pPackage );

	//
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalRequestResource
BcBool CsCore::internalRequestResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle )
{
	// Find package
	CsPackage* pPackage = findPackage( Package );
	if( pPackage || Package == BcName::NONE )
	{
		// If we can't find resource, throw an error.
		if( internalFindResource( Package, Name, Class, Handle ) == BcFalse )
		{
			PSY_LOG( "CsCore::requestResource: Resource not availible \"%s.%s:%s\" requested.\n", (*Package).c_str(), (*Name).c_str(), (*Class->getName()).c_str() );
		}
	}
	else
	{
		PSY_LOG( "CsCore::requestResource: Invalid package \"%s\" requested.\n", (*Package).c_str() );
	}
	
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalFindResource
BcBool CsCore::internalFindResource( const BcName& Package, const BcName& Name, const ReClass* Class, ReObjectRef< CsResource >& Handle )
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Make the handle null, this method must return a failure correctly.
	Handle = nullptr;

	// Function to find matching resource in a given list.
	// Used to facilitate looking into multiple lists, but we
	// have unified in to one list for all resources,
	// and individual ones for processing.
	auto findResourceInList = [ this, Package, Name, Class ]( TResourceList& List ) -> CsResource*
	{
		for( auto Resource : List )
		{
			if( Package != BcName::NONE )
			{
				if( Resource->getPackageName() == Package && 
					Resource->getName() == Name )
				{
					// Go into redirector targets.
					while( Resource->isTypeOf< CsRedirector >() )
					{
						ReObjectRef< CsRedirector > Redirector( Resource );
						Resource = Redirector->getResource();
					}
	
					if( Resource->isTypeOf( Class ) )
					{
						return Resource;
					}
				}
			}
			else
			{
				if( Resource->getName() == Name )
				{
					// Go into redirector targets.
					while( Resource->isTypeOf< CsRedirector >() )
					{
						ReObjectRef< CsRedirector > Redirector( Resource );
						Resource = Redirector->getResource();
					}
	
					if( Resource->isTypeOf( Class ) )
					{
						return Resource;
					}
				}
			}
		}

		return nullptr;
	};

	CsResource* Resource = findResourceInList( Resources_ );

	// Assign to handle and return.
	Handle = Resource;
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// getResourceByUniqueId
ReObjectRef< CsResource > CsCore::getResourceByUniqueId(BcU32 UId)
{
	return ReObject::StaticFindByUniqueId( UId );
}
