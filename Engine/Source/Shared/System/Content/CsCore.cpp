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

	while( PackageList_.size() > 0 )
	{
		freeUnreferencedPackages();

		// Finish processing unloading resources.
		if( UnloadingResources_.size() > 0 )
		{
			processUnloadingResources();
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
			BcPrintf( "%s.%s:%s \n", (*pResource->getPackageName()).c_str(), (*pResource->getName()).c_str(), pResource->getTypeName().c_str() );
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
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

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
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	TResourceFactoryInfoMapIterator Iter = ResourceFactoryInfoMap_.find( Class );
	CsResource* pResource = NULL;
	
	if( Iter != ResourceFactoryInfoMap_.end() )
	{
		const ReClass* pClass = Iter->second.pClass_;
		void* pResourceBuffer = BcMemAlign( pClass->getSize() );

		pResource = pClass->construct< CsResource >( pResourceBuffer );
		pResource->preInitialise( Name, Index, pPackage );
	}
	
	return pResource;
}

//////////////////////////////////////////////////////////////////////////
// destroyResource
void CsCore::destroyResource( CsResource* pResource )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );
		
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
BcU32 CsCore::getNoofResources()
{
	return LoadedResources_.size();
}

//////////////////////////////////////////////////////////////////////////
// getResource
CsResourceRef<> CsCore::getResource( BcU32 Idx )
{
	return LoadedResources_[ Idx ];
}

//////////////////////////////////////////////////////////////////////////
// getResource
CsResourceRef<> CsCore::getResource( const BcChar* pFullName )
{
	CsResourceRef<> Handle;
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
// getPackagePackedPath
BcPath CsCore::getPackagePackedPath( const BcName& Package )
{
	BcPath Path;
	Path.join( "PackedContent", *Package + ".pak" );
	return Path;
}

//////////////////////////////////////////////////////////////////////////
// processCreateResources
void CsCore::processCreateResources()
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	// Copy precreate in.
	TResourceHandleListIterator CreateIt( PrecreateResources_.begin() );
	while( CreateIt != PrecreateResources_.end() )
	{
		CreateResources_.push_back( *CreateIt );
		++CreateIt;
	}
	PrecreateResources_.clear();
	
	// Iterate create resources.
	TResourceHandleListIterator It( CreateResources_.begin() );
	while( It != CreateResources_.end() )
	{
		CsResourceRef<> ResourceHandle = (*It);
			
		// Create resource.
		if( ResourceHandle->getInitStage() == CsResource::INIT_STAGE_CREATE )
		{
			ResourceHandle->create();
	
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
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	TResourceHandleListIterator It( LoadingResources_.begin() );
	while( It != LoadingResources_.end() )
	{
		CsResourceRef<> ResourceHandle = (*It);
		
		// If resource is ready remove it from the list.
		if( ResourceHandle.isReady() )
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
	BcScopedLock< BcMutex > Lock( ContainerLock_ );
	
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
			BcPrintf( "%s.%s:%s \n", (*pResource->getPackageName()).c_str(), (*pResource->getName()).c_str(), pResource->getTypeName().c_str() );
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
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	while( UnloadingResources_.size() > 0 )
	{
		TResourceList ResourceList = UnloadingResources_;

		TResourceListIterator It( ResourceList.begin() );
		while( It != ResourceList.end() )
		{
			CsResource* pResource = (*It);
			
			// Destroy resource.
			pResource->destroy();
			
			// Free resource.
			pResource->getClass()->destruct( pResource );
			BcMemFree( pResource );
			
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
// internalRegisterResource
void CsCore::internalRegisterResource( const ReClass* pClass )
{
	TResourceFactoryInfo FactoryInfo;
	
	FactoryInfo.pClass_ = pClass;
	
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	ResourceFactoryInfoMap_[ pClass ] = FactoryInfo;
}

//////////////////////////////////////////////////////////////////////////
// internalUnRegisterResource
void CsCore::internalUnRegisterResource( const ReClass* Class )
{
	TResourceFactoryInfoMapIterator It = ResourceFactoryInfoMap_.find( Class );

	if( It != ResourceFactoryInfoMap_.end() )
	{
		ResourceFactoryInfoMap_.erase( It );
	}
}

//////////////////////////////////////////////////////////////////////////
// internalCreateResource
BcBool CsCore::internalCreateResource( const BcName& Name, const ReClass* Class, BcU32 Index, CsPackage* pPackage, CsResourceRef<>& Handle )
{
	// Generate a unique name for the resource.
	BcName UniqueName = Name.isValid() ? Name : BcName( Class->getTypeName() ).getUnique();

	// Allocate resource with a unique name.
	Handle = allocResource( UniqueName, Class, Index, pPackage );
	
	// Put into create list.
	if( Handle.isValid() )
	{
		BcScopedLock< BcMutex > Lock( ContainerLock_ );

		PrecreateResources_.push_back( Handle );
	}
	
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalRequestResource
BcBool CsCore::internalRequestResource( const BcName& Package, const BcName& Name, const ReClass* Class, CsResourceRef<>& Handle )
{
	// Find package
	CsPackage* pPackage = findPackage( Package );
	if( pPackage || Package == BcName::NONE )
	{
		// If we can't find resource, throw an error.
		if( internalFindResource( Package, Name, Class, Handle ) == BcFalse )
		{
			BcPrintf( "CsCore::requestResource: Resource not availible \"%s.%s:%s\" requested.\n", (*Package).c_str(), (*Name).c_str(),  Class->getTypeName().c_str() );
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
BcBool CsCore::internalFindResource( const BcName& Package, const BcName& Name, const ReClass* Class, CsResourceRef<>& Handle )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	// Make the handle null, this method must return a failure correctly.
	Handle = NULL;

	// Look in create, loading, and loaded lists.
	// Honestly, I don't like having multiple lists for everything as it makes this method a filthy
	// mess. BUT, it does mean I will have a much easier time debugging, and also process the minimum
	// amount of resources.
	// THIS IS A MESS. FIX THIS FUCKING SHIT.
	for( TResourceHandleListIterator It( CreateResources_.begin() ); It != CreateResources_.end(); ++It )
	{
		if( Package != BcName::NONE )
		{
			if( (*It)->getPackageName() == Package && (*It)->getName() == Name && (*It)->isTypeOf( Class ) )
			{
				Handle = (*It);
				return BcTrue;
			}
		}
		else
		{
			if( (*It)->getName() == Name && (*It)->isTypeOf( Class ) )
			{
				Handle = (*It);
				return BcTrue;
			}
		}
	}

	for( TResourceHandleListIterator It( LoadingResources_.begin() ); It != LoadingResources_.end(); ++It )
	{
		if( Package != BcName::NONE )
		{
			if( (*It)->getPackageName() == Package && (*It)->getName() == Name && (*It)->isTypeOf( Class ) )
			{
				Handle = (*It);
				return BcTrue;
			}
		}
		else
		{
			if( (*It)->getName() == Name && (*It)->isTypeOf( Class ) )
			{
				Handle = (*It);
				return BcTrue;
			}
		}
	}

	for( TResourceListIterator It( LoadedResources_.begin() ); It != LoadedResources_.end(); ++It )
	{
		if( Package != BcName::NONE )
		{
			if( (*It)->getPackageName() == Package && (*It)->getName() == Name && (*It)->isTypeOf( Class ) )
			{
				Handle = (*It);
				return BcTrue;
			}
		}
		else
		{
			if( (*It)->getName() == Name && (*It)->isTypeOf( Class ) )
			{
				Handle = (*It);
				return BcTrue;
			}
		}
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// getResourceByUniqueId
CsResourceRef<> CsCore::getResourceByUniqueId(BcU32 UId)
{
	for (BcU32 Idx = 0; Idx < LoadedResources_.size(); ++Idx)
	{
		if (LoadedResources_[Idx]->getUniqueId() == UId)
		{
			return LoadedResources_[Idx];
		}
	}
	return NULL;
}
