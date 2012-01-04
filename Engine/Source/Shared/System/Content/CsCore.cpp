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

#ifdef PSY_SERVER
#define BUFFERSIZE ( 64 * 1024 )
#include <b64/encode.h>
#include <b64/decode.h>
#undef BUFFERSIZE
#endif

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
	
	BcAssertMsg( FsCore::pImpl() != NULL, "CsCore: FsCore is NULL when unsubscribing from events!" );
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
	// Verify we have no more resources to be created or loading.
	BcVerifyMsg( CreateResources_.size() == 0, "CsCore: Resources to be created, but system is closing!" );
	BcVerifyMsg( LoadingResources_.size() == 0, "CsCore: Resources currently loading, but system is closing!" );

	// Finish processing unloading resources.
	if( UnloadingResources_.size() > 0 )
	{
		processUnloadingResources();
	}

	// Verify we don't have any left floating loaded or unloading.
	BcVerifyMsg( LoadedResources_.size() == 0, "CsCore: Resources still loaded, but system is closing!" );
	BcVerifyMsg( UnloadingResources_.size() == 0, "CsCore: Resources still unloading, but system is closing!" );
	
#if PSY_SERVER
	BcAssertMsg( FsCore::pImpl() != NULL, "CsCore: FsCore is NULL when unsubscribing from events!" );
	FsCore::pImpl()->unsubscribe( fsEVT_MONITOR_MODIFIED, DelegateOnFileModified_ );
	FsCore::pImpl()->unsubscribe( fsEVT_MONITOR_CREATED, DelegateOnFileModified_ );
#endif
}

//////////////////////////////////////////////////////////////////////////
// getResourceFullName
std::string CsCore::getResourceFullName( const BcName& Name, const BcName& Type ) const
{
	return *Name + std::string( "." ) + *Type;
}

//////////////////////////////////////////////////////////////////////////
// isValidResource
BcBool CsCore::isValidResource( const BcPath& FileName ) const
{
	BcName ExtensionName = BcName( FileName.getExtension() );
	return ( ResourceFactoryInfoMap_.find( ExtensionName ) != ResourceFactoryInfoMap_.end() );
}

#ifdef PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// importResource
void CsCore::importResource( const BcPath& FileName, BcBool ForceImport )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	BcBool Import = shouldImportResource( FileName, ForceImport );

	// Only import if dependancies have changed, or there are none.
	if( Import == BcTrue )
	{
		ImportList_.push_back( *FileName );
	}
}

//////////////////////////////////////////////////////////////////////////
// getResourcePropertyTable
BcBool CsCore::getResourcePropertyTable( const BcName& Type, CsPropertyTable& PropertyTable )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	TResourceFactoryInfoMapIterator Iter = ResourceFactoryInfoMap_.find( Type );
	
	if( Iter != ResourceFactoryInfoMap_.end() )
	{
		(*Iter).second.propertyTableFunc_( PropertyTable );
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// internalImportResource
void CsCore::addImportOverlayPath( const BcPath& Path )
{
	ImportOverlayPaths_.push_front( Path );
}

//////////////////////////////////////////////////////////////////////////
// findImportPath
BcPath CsCore::findImportPath( const BcPath& InputPath )
{
	for( TOverlayListIterator It( ImportOverlayPaths_.begin() ); It != ImportOverlayPaths_.end(); ++It )
	{
		BcPath AppendedPath = (*It);
		AppendedPath.join( InputPath );

		if( FsCore::pImpl()->fileExists( (*AppendedPath).c_str() ) )
		{
			return AppendedPath;
		}
	}

	// Use path passed in on fail.
	return InputPath;
}

//////////////////////////////////////////////////////////////////////////
// internalImportResource
BcBool CsCore::internalImportResource( const BcPath& FileName, CsResourceRef<>& Handle, CsDependancyList* pDependancyList )
{
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	// Only import if we should. Otherwise just request.
	if( shouldImportResource( FileName, BcFalse ) == BcFalse )
	{
		return internalRequestResource( FileName.getFileNameNoExtension(), FileName.getExtension(), Handle );
	}
	
	// Parse Json file.
	Json::Value Object;
	if( parseJsonFile( (*findImportPath( FileName )).c_str(), Object ) )
	{
		BcBool Success = BcFalse;
		
		if( pDependancyList == NULL )
		{
			CsDependancyList DependancyList;
			Success = internalImportObject( Object, Handle, &DependancyList );
			
			for( CsDependancyListIterator Iter( DependancyList.begin() ); Iter != DependancyList.end(); ++Iter )
			{
				// Add file for monitoring.
				FsCore::pImpl()->addFileMonitor( (*((*Iter).getFileName())).c_str() );
			}
			
			// Store dependancy list in map for reimporting on file modification.
			DependancyMap_[ *FileName ] = DependancyList;
		}
		else
		{
			Success = internalImportObject( Object, Handle, pDependancyList );
		}

		// Add to import map (doesn't matter if reference is bad, it's just for debugging)
		ResourceImportMap_[ *FileName ] = Handle;

		// If we've successfully imported, save dependancies.
		if( Success == BcTrue )
		{
			saveDependancies( FileName );
		}

		// Return success.
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
BcBool CsCore::parseJsonFile( const BcChar* pFileName, Json::Value& Root )
{
	BcBool Success = BcFalse;
	BcFile File;
	if( File.open( pFileName ) )
	{
		char* pData = new char[ File.size() ];
		File.read( pData, File.size() );
		
		Json::Reader Reader;
		
		if( Reader.parse( pData, pData + File.size(), Root ) )
		{
			Success = BcTrue;

			// Add file for monitoring.
			FsCore::pImpl()->addFileMonitor( pFileName );
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
// shouldImportResource
BcBool CsCore::shouldImportResource( const BcPath& FileName, BcBool ForceImport )
{
	BcBool Import = BcFalse;

	// Only attempt import if the resource filename is valid.
	if( isValidResource( FileName ) )
	{
		// Only force import if the resource is actually valid.
		Import = ForceImport;

		// Check if the packed resource exists.
		BcPath PackedPath( "./PackedContent" );
		PackedPath.join( FileName.getFileName() );
		if( FsCore::pImpl()->fileExists( (*PackedPath).c_str() ) == BcFalse )
		{
			Import = BcTrue;
		}

		// Only do a dependancy check if we have a packed file.
		// Dependancies will be regenerated on import.
		if( Import == BcFalse )
		{
			// Load dependancies if we don't have any resident in memory.
			if( DependancyMap_.find( *FileName ) == DependancyMap_.end() )
			{
				loadDependancies( FileName );
			}
	
			// Grab dependancy list for file so we don't do more work than required.
			if( DependancyMap_.find( *FileName ) != DependancyMap_.end() )
			{
				CsDependancyList& DependancyList = DependancyMap_[ *FileName ];
		
				for( CsDependancyListIterator It( DependancyList.begin() ); It != DependancyList.end(); ++It )
				{
					if( (*It).hasChanged() == BcTrue )
					{
						// Update dependancy stats.
						(*It).updateStats();
		
						// Set import to true.
						Import = BcTrue;
					}
				}
			}
			else
			{
				// No dependancy list, do the import.
				Import = BcTrue;
			}
		}
	}

	return Import;	
}

//////////////////////////////////////////////////////////////////////////
// saveDependancy
void CsCore::saveDependancies( const BcPath& FileName )
{
	BcPath DependanciesFileName( FileName );
	
	// Append new extension.
	DependanciesFileName.append( ".dep" );
	
	// 
	Json::Value Object( Json::arrayValue );

	if( DependancyMap_.find( *FileName ) != DependancyMap_.end() )
	{
		CsDependancyList& DependancyList = DependancyMap_[ *FileName ];
	
		for( CsDependancyListIterator It( DependancyList.begin() ); It != DependancyList.end(); ++It )
		{
			Object.append( saveDependancy( (*It) ) );
		}
	}

	// Output using styled writer.
	Json::StyledWriter Writer;
	std::string JsonOutput = Writer.write( Object );

	BcFile OutFile;
	if( OutFile.open( (*DependanciesFileName).c_str(), bcFM_WRITE ) )
	{
		OutFile.write( JsonOutput.c_str(), JsonOutput.size() );
		OutFile.close();
	}
}

//////////////////////////////////////////////////////////////////////////
// loadDependancies
void CsCore::loadDependancies( const BcPath& FileName )
{
	BcPath DependanciesFileName( FileName );
	
	// Append new extension.
	DependanciesFileName.append( ".dep" );

	// Overwrite old dependancy list.
	DependancyMap_[ *FileName ] = CsDependancyList();


	// Open dependancy file.
	BcFile OutFile;
	if( OutFile.open( (*DependanciesFileName).c_str(), bcFM_READ ) )
	{
		BcU32 BufferSize = OutFile.size() + 1;
		BcChar* pJsonData = new BcChar[ BufferSize ];
		BcMemZero( pJsonData, BufferSize );
		OutFile.read( pJsonData, OutFile.size() );

		Json::Value Root;
		Json::Reader JsonReader;
		if( JsonReader.parse( pJsonData, pJsonData + OutFile.size(), Root ) )
		{
			// Create a new dependancy list.
			CsDependancyList DependancyList;

			// Iterate over all dependancies for file and parse.
			for( Json::Value::iterator It( Root.begin() ); It != Root.end(); ++It )
			{
				const Json::Value& Value = (*It);
				CsDependancy Dependancy = loadDependancy( Value );
				DependancyList.push_back( Dependancy );
			}

			// Assign.
			DependancyMap_[ *FileName ] = DependancyList;
		}

		OutFile.close();
	}
}

//////////////////////////////////////////////////////////////////////////
// saveDependancy
Json::Value CsCore::saveDependancy( const CsDependancy& Dependancy )
{
	// 
	Json::Value Object( Json::objectValue );

	Object[ "filename" ] = *Dependancy.getFileName();

	// Encode stats as binary.
	// NOTE: Endianess isn't taken into account.
	BcU32 BufferSize = sizeof( FsStats ) * 2;
	BcChar* pStatsBuffer = new BcChar[ BufferSize ];
	base64::base64_encodestate EncodeState;
	BcMemZero( pStatsBuffer, BufferSize );
	BcMemZero( &EncodeState, sizeof( EncodeState ) );
	base64::base64_encode_block( reinterpret_cast< const char* >( &Dependancy.getStats() ), sizeof( FsStats ), reinterpret_cast< char* >( pStatsBuffer ), &EncodeState );
	Object[ "stats" ] = pStatsBuffer;
	delete [] pStatsBuffer;

	return Object;
}

//////////////////////////////////////////////////////////////////////////
// loadDependancy
CsDependancy CsCore::loadDependancy( const Json::Value& Object )
{
	// Grab props..
	BcPath FileName( Object[ "filename" ].asCString() );
	std::string StatsB64( Object[ "stats" ].asString() );

	// Encode stats as binary.
	// NOTE: Endianess isn't taken into account.
	FsStats Stats;
	base64::base64_decodestate DecodeState;
	BcMemZero( &DecodeState, sizeof( DecodeState ) );
	base64::base64_decode_block( reinterpret_cast< const char* >( StatsB64.c_str() ), StatsB64.size(), reinterpret_cast< char* >( &Stats ), &DecodeState );

	return CsDependancy( FileName, Stats );
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
				if( BcStrCompare( (*(*DepIter).getFileName()).c_str(), Event.FileName_ ) )
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
			BcPrintf( "%s (%s)\n", (*pResource->getName()).c_str(), (*pResource->getType()).c_str() );
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
CsResource* CsCore::allocResource( const BcName& Name, const BcName& Type, CsFile* pFile )
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
void CsCore::internalRegisterResource( const BcName& Type, CsResourceAllocFunc allocFunc, CsResourceFreeFunc freeFunc, CsResourcePropertyTableFunc propertyTableFunc )
{
	TResourceFactoryInfo FactoryInfo;
	
	FactoryInfo.allocFunc_ = allocFunc;
	FactoryInfo.freeFunc_ = freeFunc;
	FactoryInfo.propertyTableFunc_ = propertyTableFunc;
	
	BcScopedLock< BcMutex > Lock( ContainerLock_ );

	ResourceFactoryInfoMap_[ Type ] = FactoryInfo;
}

//////////////////////////////////////////////////////////////////////////
// internalUnRegisterResource
void CsCore::internalUnRegisterResource( const BcName& Type )
{
	TResourceFactoryInfoMapIterator It = ResourceFactoryInfoMap_.find( Type );

	if( It != ResourceFactoryInfoMap_.end() )
	{
		ResourceFactoryInfoMap_.erase( It );
	}
}

//////////////////////////////////////////////////////////////////////////
// internalCreateResource
BcBool CsCore::internalCreateResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle )
{
	// Generate a unique name for the resource.
	BcName UniqueName = Name.isValid() ? Name.getUnique() : Type.getUnique();

	// Allocate resource with a unique name.
	Handle = allocResource( UniqueName, Type, NULL );
	
	// Put into create list.
	if( Handle.isValid() )
	{
		BcScopedLock< BcMutex > Lock( ContainerLock_ );

		CreateResources_.push_back( Handle );
	}
	
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalRequestResource
BcBool CsCore::internalRequestResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle )
{
	// Try to find resource, if we can't, allocate a new one and put into create list.
	if( internalFindResource( Name, Type, Handle ) == BcFalse )
	{
		// Only request if we have a name.
		if( Name.isValid() )
		{
			// Create a file reader for resource (using full name!)
			CsFile* pFile = createFileReader( getResourceFullName( Name, Type ) );
			
			// Allocate resource.
			Handle = allocResource( Name, Type, pFile );
			
			if( Handle.isValid() )
			{
				// Call default initialiser.
				Handle->initialise();
				
				// Acquire (callback from load will release).
				Handle->acquire();
				
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
					BcPrintf( "CsCore::requestResource: Failed to load %s (%s).\n", (*Name).c_str(), pFile->getName().c_str() );
					
					// Release (callback from load won't happen on failure).
					Handle->release();
					Handle = NULL;
				}
			}
			else
			{
				BcPrintf( "CsCore::requestResource: Failed to create %s (%s).\n", (*Name).c_str(), pFile->getName().c_str() );
				Handle = NULL;
			}
		}
		else
		{
			BcPrintf( "CsCore::requestResource: Resource name invalid.\n" );
			Handle = NULL;
		}
	}
	
	return Handle.isValid();
}

//////////////////////////////////////////////////////////////////////////
// internalFindResource
BcBool CsCore::internalFindResource( const BcName& Name, const BcName& Type, CsResourceRef<>& Handle )
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


