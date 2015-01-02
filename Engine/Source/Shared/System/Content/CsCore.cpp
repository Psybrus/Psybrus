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
#include "System/Content/CsRedirector.h"

#if !PLATFORM_HTML5
#include <boost/filesystem.hpp>
#endif // !PLATFORM_HTML5

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
	// Should be handled in CsPackage. Look into doing it.
	processCreateResources();
	processLoadingResources();
	processLoadedResource();
	freeUnreferencedPackages();
	processUnloadingResources();
	processCallbacks();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual 
void CsCore::close()
{
	// Verify we have no more resources to be created or loading.
	BcVerifyMsg( CreateResources_.size() == 0, "CsCore: Resources to be created, but system is closing!" );
	BcVerifyMsg( LoadingResources_.size() == 0, "CsCore: Resources currently loading, but system is closing!" );

	static BcF32 UnloadTimeout = 5.0f;
	BcTimer UnloadTimer;
	UnloadTimer.mark();
	while( PackageList_.size() > 0 )
	{
		freeUnreferencedPackages();

		// Finish processing unloading resources.
		if( UnloadingResources_.size() > 0 )
		{
			processUnloadingResources();
		}

		// If we hit the timeout, just break out and shut down cleanly.
		if( UnloadTimer.time() > UnloadTimeout )
		{
			break;
		}
	}

	if( LoadedResources_.size() > 0 )
	{
		BcPrintf( "==========================================\n" );
		BcPrintf( "CsCore: Dump Resource On Exit:\n" );
		BcPrintf( "==========================================\n" );
	
		TResourceListIterator It( LoadedResources_.begin() );
		while( It != LoadedResources_.end() )
		{
			CsResource* pResource = (*It);
			BcPrintf( "%s:%s \n", (*pResource->getName()).c_str(), (*pResource->getTypeName()).c_str() );
			++It;
		}
		BcPrintf( "==========================================\n" );
	}

	// Verify we don't have any left floating loaded or unloading.
	BcVerifyMsg( LoadedResources_.size() == 0, "CsCore: Resources still loaded, but system is closing! Has the scene cleaned up properly?" );
	BcVerifyMsg( UnloadingResources_.size() == 0, "CsCore: Resources still unloading, but system is closing!" );
}

//////////////////////////////////////////////////////////////////////////
// freeUnreferencedPackages
void CsCore::freeUnreferencedPackages()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Search for existing package and move to unreferenced list.
	for( TPackageListIterator It( PackageList_.begin() ); It != PackageList_.end();  )
	{
		CsPackage* pPackage = (*It);
		if( pPackage->hasUnreferencedResources() )
		{
			pPackage->releaseUnreferencedResources();

			// If we've got no valid resources we can move to unreferenced list to destroy.
			if( pPackage->haveAnyValidResources() == BcFalse )
			{
				UnreferencedPackageList_.push_back( pPackage );
				It = PackageList_.erase( It );
			}
			else
			{
				++It;
			}
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
// destroyResource
void CsCore::destroyResource( CsResource* pResource )
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );
		
	// Find the resource in the list.
	TResourceListIterator FoundIt = LoadedResources_.end();
	
	for( TResourceListIterator It( LoadedResources_.begin() ); It != LoadedResources_.end(); ++It )
	{
		if( (*It) == pResource )
		{
			FoundIt = It;
			break;
		}
	}
	
	// If it's in the loaded list we need to put it into the unloading list,
	// otherwise delete it right away (imported resource).
	if( FoundIt != LoadedResources_.end() )
	{
		// Remove from list.
		LoadedResources_.erase( FoundIt );
	
		// Put into unloading list.
		UnloadingResources_.push_back( pResource );
	}
	else
	{
		pResource->getClass()->destruct( pResource );
		BcMemFree( pResource );
	}
}

//////////////////////////////////////////////////////////////////////////
// getNoofResources
size_t CsCore::getNoofResources()
{
	return LoadedResources_.size();
}

//////////////////////////////////////////////////////////////////////////
// getResource
ReObjectRef< CsResource > CsCore::getResource( size_t Idx )
{
	return LoadedResources_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getResource
ReObjectRef< CsResource > CsCore::getResource( const BcChar* pFullName )
{
	ReObjectRef< CsResource > Handle;
	if( pFullName != NULL )
	{
		BcChar FullNameBuffer[ 1024 ];
		BcAssertMsg( BcStrLength( pFullName ) < sizeof( FullNameBuffer ), "CsPackageImporter: Full name too long." );
		BcStrCopy( FullNameBuffer, pFullName );
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
	if( pPackage != NULL )
	{
		return pPackage;
	}

	// Check for a packed package.
	BcPath PackedPackage( getPackagePackedPath( Package ) );
	BcPath ImportPackage( getPackageImportPath( Package ) );
	BcBool PackageExists = FsCore::pImpl()->fileExists( (*PackedPackage).c_str() ) || FsCore::pImpl()->fileExists( (*ImportPackage).c_str() );

	// If it exists, create it. Internally it will trigger it's own load.
	if( PackageExists )
	{
		pPackage = new CsPackage( Package );
		PackageList_.push_back( pPackage );
	}	
	else
	{
		BcAssertMsg( BcFalse, "CsCore: Can't import package, missing \"%s\" or \"%s\"", (*PackedPackage).c_str(), (*ImportPackage).c_str() );
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
// getPackageImportPath
BcPath CsCore::getPackageImportPath( const BcName& Package )
{
	BcPath Path;
	Path.join( "Content", *Package + ".pkg" );
	return Path;
}

//////////////////////////////////////////////////////////////////////////
// getPackageIntermediatePath
BcPath CsCore::getPackageIntermediatePath( const BcName& Package )
{
	BcPath Path;
	if( Package != BcName::INVALID )
	{
		Path.join( "IntermediateContent", *Package + ".pak" );
	}
	else
	{
		Path = "IntermediateContent";
	}

#if !PLATFORM_HTML5
	boost::filesystem::create_directories( *Path );
#endif // !PLATFORM_HTML5

	return Path;
}

//////////////////////////////////////////////////////////////////////////
// getPackagePackedPath
BcPath CsCore::getPackagePackedPath( const BcName& Package )
{
	BcPath Path;
	if( Package != BcName::INVALID )
	{
		Path.join( "PackedContent", *Package + ".pak" );
	}
	else
	{
		Path = "PackedContent";
	}
	return Path;
}

//////////////////////////////////////////////////////////////////////////
// getIntermediatePath
BcPath CsCore::getIntermediatePath( const std::string& SubFolder )
{
	BcPath Path;
	Path.join( "IntermediateContent", SubFolder + ".pak" );

#if !PLATFORM_HTML5
	boost::filesystem::create_directories( *Path );
#endif // !PLATFORM_HTML5
	return Path;
}

//////////////////////////////////////////////////////////////////////////
// processCreateResources
void CsCore::processCreateResources()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	// Copy precreate in.
	auto CreateIt( PrecreateResources_.begin() );
	while( CreateIt != PrecreateResources_.end() )
	{
		BcAssert( (*CreateIt)->getName() != BcName::INVALID );
		CreateResources_.push_back( *CreateIt );
		++CreateIt;
	}
	PrecreateResources_.clear();
	
	// Iterate create resources.
	auto It( CreateResources_.begin() );
	while( It != CreateResources_.end() )
	{
		ReObjectRef< CsResource > ResourceHandle = (*It);
			
		// Create resource.
		if( ResourceHandle->getInitStage() >= CsResource::INIT_STAGE_CREATE )
		{
			// Only create if still in the create stage, otherwise skip a stage.
			if( ResourceHandle->getInitStage() == CsResource::INIT_STAGE_CREATE )
			{
				ResourceHandle->create();
			}
	
			// Remove from list.
			It = CreateResources_.erase( It );
	
			// Put into loading list.
			LoadingResources_.push_back( ResourceHandle );
		}
		else
		{
			++It;
		}
	}

}

//////////////////////////////////////////////////////////////////////////
// processLoadingResources
void CsCore::processLoadingResources()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	auto It( LoadingResources_.begin() );
	while( It != LoadingResources_.end() )
	{
		ReObjectRef< CsResource > ResourceHandle = (*It);
		
		// If resource is ready remove it from the list.
		if( ResourceHandle.isValid() && ResourceHandle->isReady() )
		{
			It = LoadingResources_.erase( It );
			
			// Put into loaded list.
			LoadedResources_.push_back( ResourceHandle );
		}
		else
		{
			++It;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// processLoadedResource
void CsCore::processLoadedResource()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );
	
	static BcBool DumpResources = BcFalse;
	
	if( DumpResources )
	{
		BcPrintf( "==========================================\n" );
		BcPrintf( "CsCore: Dump Resource:\n" );
		BcPrintf( "==========================================\n" );
	}
	
	TResourceListIterator It( LoadedResources_.begin() );
	while( It != LoadedResources_.end() )
	{
		CsResource* pResource = (*It);
		BcUnusedVar( pResource );
		
		// NOTE: Placeholder for doing stuff. Probably don't need to other
		//       than for debug purposes.
		if( DumpResources )
		{
			BcPrintf( "%s.%s:%s \n", (*pResource->getPackageName()).c_str(), (*pResource->getName()).c_str(), (*pResource->getTypeName()).c_str() );
		}
	
		++It;
	}

	if( DumpResources )
	{
		BcPrintf( "==========================================\n" );
		DumpResources = BcFalse;
	}
}

//////////////////////////////////////////////////////////////////////////
// processUnloadingResources
void CsCore::processUnloadingResources()
{
	std::lock_guard< std::recursive_mutex > Lock( ContainerLock_ );

	while( UnloadingResources_.size() > 0 )
	{
		TResourceList ResourceList = UnloadingResources_;

		TResourceListIterator It( ResourceList.begin() );
		while( It != ResourceList.end() )
		{
			CsResource* pResource = (*It);
			
			// Destroy resource.
			pResource->destroy();
			delete pResource;
			
			// Next.
			++It;
		}
	
		UnloadingResources_.clear();
	}
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
	PrecreateResources_.push_back( Resource );
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
			BcPrintf( "CsCore::requestResource: Resource not availible \"%s.%s:%s\" requested.\n", (*Package).c_str(), (*Name).c_str(), (*Class->getName()).c_str() );
		}
	}
	else
	{
		BcPrintf( "CsCore::requestResource: Invalid package \"%s\" requested.\n", (*Package).c_str() );
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

	// Look in create, loading, and loaded lists.
	// Honestly, I don't like having multiple lists for everything as it makes this method a filthy
	// mess. BUT, it does mean I will have a much easier time debugging, and also process the minimum
	// amount of resources.
	// THIS IS A MESS. FIX THIS FUCKING SHIT.
	// This is now slightly less of a fucking mess, but I still don't like it.
	auto FindResourceInList = [ this, Package, Name, Class ]( TResourceList& List ) -> CsResource*
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

	CsResource* Resource = FindResourceInList( CreateResources_ );
	if( Resource == nullptr )
	{
		Resource = FindResourceInList( LoadingResources_ );
	}
	if( Resource == nullptr )
	{
		Resource = FindResourceInList( LoadedResources_ );
	}

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
