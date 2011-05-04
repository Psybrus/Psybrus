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

#include "CsCore.h"

#include "CsFileReader.h"
#include "CsFileWriter.h"

SYS_CREATOR( CsCore );

//////////////////////////////////////////////////////////////////////////
// Ctor
CsCore::CsCore()
{
	
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
#if PSY_SERVER
	// Bind file hooks.
	DelegateOnFileModified_ = FsEventMonitor::Delegate::bind< CsCore, &CsCore::eventOnFileModified >( this );
	
	FsCore::pImpl()->subscribe( fsEVT_MONITOR_MODIFIED, DelegateOnFileModified_ );
	FsCore::pImpl()->subscribe( fsEVT_MONITOR_CREATED, DelegateOnFileModified_ );
#endif
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual 
void CsCore::update()
{
	// TODO: Remove these, all is handled in CsResource now!
	processCreateResources();
	processLoadingResources();
	processLoadedResource();
	processUnloadingResources();


#ifdef PSY_SERVER
	{
		// Import everything in the import list.
		BcScopedLock< BcMutex > Lock( ContainerLock_ );
		CsResourceRef<> Handle;
		
		for( TImportListIterator Iter( ImportList_.begin() ); Iter != ImportList_.end(); ++Iter )
		{
			internalImportResource( (*Iter), Handle, NULL );
		}	
		
		ImportList_.clear();
	}
#endif
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual 
void CsCore::close()
{
	// NOTE: These can be hit at the moment, not fixing because it isn't
	//       going to affect actual gameplay. Will sort it out when I move
	//       everything into CsResource::process.
	BcAssert( CreateResources_.size() == 0 );
	BcAssert( LoadingResources_.size() == 0 );
	BcAssert( LoadedResources_.size() == 0 );
	BcAssert( UnloadingResources_.size() == 0 );
	
#if PSY_SERVER
	FsCore::pImpl()->unsubscribe( fsEVT_MONITOR_MODIFIED, DelegateOnFileModified_ );
	FsCore::pImpl()->unsubscribe( fsEVT_MONITOR_CREATED, DelegateOnFileModified_ );
#endif
}

//////////////////////////////////////////////////////////////////////////
// getResourceFullName
std::string CsCore::getResourceFullName( const std::string& Name, const std::string& Type ) const
{
	return Name + std::string( "." ) + Type;
}

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// internalImportResource
BcBool CsCore::internalImportResource( const std::string& FileName, CsResourceRef<>& Handle, CsDependancyList* pDependancyList )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );
	
	//
	BcPrintf( "CsCore::ImportResource: %s\n", FileName.c_str() );
	
	// Parse Json file.
	Json::Value Object;
	if( parseJsonFile( FileName, Object ) )
	{
		BcBool Success = BcFalse;
		
		if( pDependancyList == NULL )
		{
			CsDependancyList DependancyList;
			Success = internalImportObject( Object, Handle, &DependancyList );

			BcPrintf( "Dependancies for %s:\n", FileName.c_str() );
			for( CsDependancyListIterator Iter( DependancyList.begin() ); Iter != DependancyList.end(); ++Iter )
			{
				BcPrintf( " - %s\n", (*Iter).getFileName().c_str() );

				// Add file for monitoring.
				// TODO: Remove old files.
				FsCore::pImpl()->addFileMonitor( (*Iter).getFileName().c_str() );
			}
			
			// Store dependancy list in map for reimporting on file modification.
			DependancyMap_[ FileName ] = DependancyList;
		}
		else
		{
			Success = internalImportObject( Object, Handle, pDependancyList );
		}

		// Add to import map (doesn't matter if reference is bad, it's just for debugging)
		ResourceImportMap_[ FileName ] = Handle;
		
		return Success;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// internalImportObject
BcBool CsCore::internalImportObject( const Json::Value& Object, CsResourceRef<>& Handle, CsDependancyList* pDependancyList )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	// If we pass in a string value, call into the importResource that takes a file name.
	// If we pass in an object value, load it from that.
	if( Object.type() == Json::stringValue )
	{
		// We don't pass the dependancy list in if it's a file.
		return internalImportResource( Object.asString(), Handle, NULL );
	}
	else if( Object.type() == Json::objectValue )
	{
		Json::Value::Members Members = Object.getMemberNames();
		
		if( Members.size() == 1 )
		{
			const std::string& Type = Members[ 0 ];
			const Json::Value& Resource = Object[ Type ];
			
			// If the resource is a string, it's an alias - so we need to load the appropriate file.
			// If the resource is an object, then do appropriate loading.
			if( Resource.type() == Json::stringValue )
			{
				return internalImportResource( Resource.asString(), Handle, pDependancyList );				
			}
			else if( Resource.type() == Json::objectValue )
			{
				// Check the resource has a name, and it's a string.
				if( Resource.isMember( "name" ) && Resource[ "name" ].type() == Json::stringValue )
				{ 
					// Import resource by string.
					const std::string& Name = Resource[ "name" ].asString();
				
					// Check name is a valid length.
					// TODO: Check it only contains valid characters.
					if( Name.length() > 0 )
					{
						// Create a file writer for resource (using full name!)
						CsFile* pFile = createFileWriter( getResourceFullName( Name, Type ) );
					
						if( pFile != NULL )
						{
							// Allocate resource.
							Handle = allocResource( Name, Type, pFile );
						
							if( Handle.isValid() )
							{							
								// Import the resource immediately (blocking operation).
								if( Handle->import( Resource, *pDependancyList ) )
								{
									// Save file.
									pFile->save();
																										
									// Now we need to request the resource, this will delete the imported
									// one and replace it with a valid loaded resource.
									internalRequestResource( Name, Type, Handle );
								}	
								else
								{
									// Failed to import, so set handle to NULL to release it.
									Handle = NULL;
								}
							}
							else
							{
								delete pFile;
								BcPrintf( "CsCore: Can not allocate resource type %s.\n", Type.c_str() );
							}
						}
						else
						{
							BcPrintf( "CsCore: Can not allocate file writer.\n" );
						}	
					}
					else
					{
						BcPrintf( "CsCore: Json object does not contain valid name.\n" );
					}
				}
				else
				{
					BcPrintf( "CsCore: Json object does not contain name.\n" );
				}
			}
			else
			{
				BcPrintf( "CsCore: Json object is neither an alias or valid resource.\n" );
			}
		}
		else
		{
			BcPrintf( "CsCore: Json object contains more than 1 element.\n" );
		}
	}
	else
	{
		BcPrintf( "CsCore: Invalid Json object.\n" );		
	}
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// parseJsonFile
BcBool CsCore::parseJsonFile( const std::string& FileName, Json::Value& Root )
{
	BcBool Success = BcFalse;
	BcFile File;
	if( File.open( FileName.c_str() ) )
	{
		char* pData = new char[ File.size() ];
		File.read( pData, File.size() );
		
		Json::Reader Reader;
		
		if( Reader.parse( pData, pData + File.size(), Root ) )
		{
			Success = BcTrue;

			// Add file for monitoring.
			FsCore::pImpl()->addFileMonitor( FileName.c_str() );
		}
		else
		{
			BcPrintf( "CsCore: Failed to parse Json:\n %s\n", Reader.getFormatedErrorMessages().c_str() );
		}
		
		delete [] pData;
	}
	
	return Success;
}

//////////////////////////////////////////////////////////////////////////
// eventOnFileModified
eEvtReturn CsCore::eventOnFileModified( BcU32 EvtID, const FsEventMonitor& Event )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	// See if the file matches our import list.
	{
		TResourceRefMapIterator FoundIter = ResourceImportMap_.find( Event.FileName_ );
		
		// Add to import list.
		if( FoundIter != ResourceImportMap_.end() )
		{
			ImportList_.push_back( (*FoundIter).first );
			return evtRET_PASS;
		}
	}
	
	// Check dependancies.
	{
		for( TDependancyMapIterator Iter( DependancyMap_.begin() ); Iter != DependancyMap_.end(); ++Iter )
		{
			const std::string& ResourceName = (*Iter).first;
			CsDependancyList& DependancyList = (*Iter).second;
			
			for( CsDependancyListIterator DepIter( DependancyList.begin() ); DepIter != DependancyList.end(); ++DepIter )
			{
				// If the dependancy filename matches the one modified, then add resource to the dependancy list.
				if( (*DepIter).getFileName() == Event.FileName_ )
				{
					ImportList_.push_back( ResourceName );
					break;
				}
			}
		}
	}
			
	return evtRET_PASS;
}

#endif

//////////////////////////////////////////////////////////////////////////
// processCreateResources
void CsCore::processCreateResources()
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );
	
	TResourceHandleListIterator It( CreateResources_.begin() );
	while( It != CreateResources_.end() )
	{
		CsResourceRef<> ResourceHandle = (*It);
		
		// Create resource.
		ResourceHandle->create();
		
		// Remove from list.
		It = CreateResources_.erase( It );
	
		// Put into loading list.
		LoadingResources_.push_back( ResourceHandle );
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

	TResourceListIterator It( LoadedResources_.begin() );
	while( It != LoadedResources_.end() )
	{
		CsResource* pResource = (*It);
		BcUnusedVar( pResource );
		
		// NOTE: Placeholder for doing stuff. Probably don't need to other
		//       than for debug purposes.
		
		++It;
	}
}

//////////////////////////////////////////////////////////////////////////
// processUnloadingResources
void CsCore::processUnloadingResources()
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	TResourceListIterator It( UnloadingResources_.begin() );
	while( It != UnloadingResources_.end() )
	{
		CsResource* pResource = (*It);

		// Destroy resource.
		pResource->destroy();
		
		// Free resource.
		delete pResource;

		// Remove from list.
		It = UnloadingResources_.erase( It );
	}
}

//////////////////////////////////////////////////////////////////////////
// allocResource
CsResource* CsCore::allocResource( const std::string& Name, const std::string& Type, CsFile* pFile )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	TResourceFactoryInfoMapIterator Iter = ResourceFactoryInfoMap_.find( Type );
	CsResource* pResource = NULL;
	
	if( Iter != ResourceFactoryInfoMap_.end() )
	{
		pResource = (*Iter).second.allocFunc_( Name, pFile );
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
		delete pResource;
	}
}

//////////////////////////////////////////////////////////////////////////
// createFileReader
//virtual
CsFile* CsCore::createFileReader( const std::string& FileName )
{
	return new CsFileReader( FileName );
}

//////////////////////////////////////////////////////////////////////////
// createFileWriter
//virtual
CsFile* CsCore::createFileWriter( const std::string& FileName )
{
#ifdef PSY_SERVER
	return new CsFileWriter( FileName );
#else
	return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
// internalRegisterResource
void CsCore::internalRegisterResource( const std::string& Type, CsResourceAllocFunc allocFunc, CsResourceFreeFunc freeFunc )
{
	TResourceFactoryInfo FactoryInfo;
	
	FactoryInfo.allocFunc_ = allocFunc;
	FactoryInfo.freeFunc_ = freeFunc;
	
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	ResourceFactoryInfoMap_[ Type ] = FactoryInfo;
}

//////////////////////////////////////////////////////////////////////////
// internalCreateResource
BcBool CsCore::internalCreateResource( const std::string& Name, const std::string& Type, CsResourceRef<>& Handle )
{
	// Try to find resource, if we can't, allocate a new one and put into create list.
	//if( internalFindResource( Name, Type, Handle ) == BcFalse )
	{
		// Only request if we have a name.
		if( Name.length() > 0 )
		{		
			// Allocate resource.
			Handle = allocResource( Name, Type, NULL );
			
			// Put into create list.
			if( Handle.isValid() )
			{
				BcScopedLock< BcMutex > Lock( ContainerLock_ );

				CreateResources_.push_back( Handle );
			}
		}
	}
	
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalRequestResource
BcBool CsCore::internalRequestResource( const std::string& Name, const std::string& Type, CsResourceRef<>& Handle )
{
	// Try to find resource, if we can't, allocate a new one and put into create list.
	if( internalFindResource( Name, Type, Handle ) == BcFalse )
	{
		// Only request if we have a name.
		if( Name.length() > 0 )
		{
			// Create a file reader for resource (using full name!)
			CsFile* pFile = createFileReader( getResourceFullName( Name, Type ) );
			
			// Allocate resource.
			Handle = allocResource( Name, Type, pFile );
			
			if( Handle.isValid() )
			{
				// Call default initialiser.
				Handle->initialise();
				
				// Trigger a file load.
				if( pFile->load( CsFileReadyDelegate::bind< CsResource, &CsResource::delegateFileReady >( (CsResource*)Handle ),
								CsFileChunkDelegate::bind< CsResource, &CsResource::delegateFileChunkReady >( (CsResource*)Handle ) ) )
				{
					// Put into create list.
					if( Handle.isValid() )
					{
						BcScopedLock< BcMutex > Lock( ContainerLock_ );
						
						CreateResources_.push_back( Handle );
					}
				}
				else
				{
					BcPrintf( "CsCore::requestResource: Failed to load %s (%s).\n", Name.c_str(), pFile->getName().c_str() );
				}
			}
			else
			{
				BcPrintf( "CsCore::requestResource: Failed to create %s (%s).\n", Name.c_str(), pFile->getName().c_str() );
			}
		}
		else
		{
			BcPrintf( "CsCore::requestResource: Resource name invalid.\n" );
		}
	}
	
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalFindResource
BcBool CsCore::internalFindResource( const std::string& Name, const std::string& Type, CsResourceRef<>& Handle )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	// Make the handle null, this method must return a failure correctly.
	Handle = NULL;

	// Look in create, loading, and loaded lists.
	// Honestly, I don't like having multiple lists for everything as it makes this method a filthy
	// mess. BUT, it does mean I will have a much easier time debugging, and also process the minimum
	// amount of resources.
	for( TResourceHandleListIterator It( CreateResources_.begin() ); It != CreateResources_.end(); ++It )
	{
		if( (*It)->getName() == Name && (*It)->isTypeOf( Type ) )
		{
			Handle = (*It);
			return BcTrue;
		}
	}

	for( TResourceHandleListIterator It( LoadingResources_.begin() ); It != LoadingResources_.end(); ++It )
	{
		if( (*It)->getName() == Name && (*It)->isTypeOf( Type ) )
		{
			Handle = (*It);
			return BcTrue;
		}
	}

	for( TResourceListIterator It( LoadedResources_.begin() ); It != LoadedResources_.end(); ++It )
	{
		if( (*It)->getName() == Name && (*It)->isTypeOf( Type ) )
		{
			Handle = (*It);
			return BcTrue;
		}
	}

	return BcFalse;
}


