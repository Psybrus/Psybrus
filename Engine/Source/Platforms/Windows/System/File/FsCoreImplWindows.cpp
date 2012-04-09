/**************************************************************************
*
* File:		FsCoreImplWindows.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsCoreImplWindows.h"
#include "System/File/FsFileImplWindows.h"

#include "System/SysKernel.h"

#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( FsCoreImplWindows );

//////////////////////////////////////////////////////////////////////////
// Ctor
FsCoreImplWindows::FsCoreImplWindows()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsCoreImplWindows::~FsCoreImplWindows()
{

}

//////////////////////////////////////////////////////////////////////////
// init
//virtual
void FsCoreImplWindows::open()
{
	int RetVal = 0;
	
	// Set the resource path.
	// NOTE: May need this again sometime, going to keep code here for ref.
	//[[NSFileManager defaultManager] changeCurrentDirectoryPath:@"/Users/neilo/Documents/Dev/Psybrus/Examples/TestBed"];
	
	// Setup file monitor iterator.
	FileMonitorMapIterator_ = FileMonitorMap_.begin();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void FsCoreImplWindows::update()
{
	// Update file monitoring.
	updateFileMonitoring();
}

//////////////////////////////////////////////////////////////////////////
// shutdown
//virtual
void FsCoreImplWindows::close()
{
	
}

//////////////////////////////////////////////////////////////////////////
// newFileImpl
//virtual
FsFileImpl* FsCoreImplWindows::openFile( const BcChar* pFilename, eFsFileMode FileMode )
{
	FsFileImpl* pFileImpl = NULL;

	// PhysFS
	pFileImpl = new FsFileImplWindows();

	// Check for error or success.
	if( pFileImpl->open( pFilename, FileMode ) == BcFalse )
	{
		delete pFileImpl;
		pFileImpl = NULL;
	}
	
	return pFileImpl;
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void FsCoreImplWindows::closeFile( FsFileImpl* pFileImpl )
{
	delete pFileImpl;
}

//////////////////////////////////////////////////////////////////////////
// fileExists
//virtual
BcBool FsCoreImplWindows::fileExists( const BcChar* pFilename )
{
	FILE* pHandle = NULL;
	pHandle = fopen( pFilename, "rb" );
	if( pHandle != NULL )
	{
		fclose( pHandle );
	}
	return pHandle != NULL;
}

//////////////////////////////////////////////////////////////////////////
// fileExists
//virtual
BcBool FsCoreImplWindows::fileStats( const BcChar* pFilename, FsStats& Stats )
{
#if COMPILER_MSVC
	struct _stat Attrib;

	int Descriptor = ::open(pFilename, _O_RDONLY | _O_BINARY);
	if( Descriptor != -1 )
	{
		if( _fstat( Descriptor, &Attrib ) == 0 )
		{
			struct tm* pCreatedTime;
			struct tm* pModifiedTime;
			pCreatedTime = gmtime( &(Attrib.st_ctime ) );
			pModifiedTime = gmtime( &(Attrib.st_mtime ) );
	
			Stats.CreatedTime_.Seconds_ = pCreatedTime->tm_sec;
			Stats.CreatedTime_.Minutes_ = pCreatedTime->tm_min;
			Stats.CreatedTime_.Hours_ = pCreatedTime->tm_hour;
			Stats.CreatedTime_.MonthDay_ = pCreatedTime->tm_mday;
			Stats.CreatedTime_.Month_ = pCreatedTime->tm_mon;
			Stats.CreatedTime_.Year_ = pCreatedTime->tm_year;
			Stats.CreatedTime_.WeekDay_ = pCreatedTime->tm_wday;
			Stats.CreatedTime_.YearDay_ = pCreatedTime->tm_yday;
			Stats.CreatedTime_.IsDST_ = pCreatedTime->tm_isdst;
	
			Stats.ModifiedTime_.Seconds_ = pModifiedTime->tm_sec;
			Stats.ModifiedTime_.Minutes_ = pModifiedTime->tm_min;
			Stats.ModifiedTime_.Hours_ = pModifiedTime->tm_hour;
			Stats.ModifiedTime_.MonthDay_ = pModifiedTime->tm_mday;
			Stats.ModifiedTime_.Month_ = pModifiedTime->tm_mon;
			Stats.ModifiedTime_.Year_ = pModifiedTime->tm_year;
			Stats.ModifiedTime_.WeekDay_ = pModifiedTime->tm_wday;
			Stats.ModifiedTime_.YearDay_ = pModifiedTime->tm_yday;
			Stats.ModifiedTime_.IsDST_ = pModifiedTime->tm_isdst;
		}
		::close( Descriptor );
		return BcTrue;
	}
#endif
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// findFiles
void FsCoreImplWindows::findFiles( BcPath Path, BcBool Recursive, BcBool AddFolders, std::list< BcPath >& OutputPaths )
{
	WIN32_FIND_DATA FileData;
	HANDLE SearchHandle = NULL;

	// If we want to add folders, do so.
	if( AddFolders == BcTrue )
	{
		BcPath NewPath( Path );
		NewPath.join( "/" );
		OutputPaths.push_back( NewPath );
	}

	//
	while( getFiles( SearchHandle, FileData, Path ) )
	{
		// Ignore hidden files
		if( ( FileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ) == 0 )
		{
			BcBool IsFolder = ( FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ? BcTrue : BcFalse;

			BcPath NewPath( Path );
			NewPath.join( BcPath( &FileData.cFileName[ 0 ] ) );

			// Add to list if it's a file.
			if( IsFolder == BcFalse )
			{
				OutputPaths.push_back( NewPath );
			}

			// Recurse folder
			if ( IsFolder == BcTrue && Recursive == BcTrue )
			{
				findFiles( NewPath, BcTrue, AddFolders, OutputPaths );
			}
		}
	}    
}

//////////////////////////////////////////////////////////////////////////
// getFiles
BcBool FsCoreImplWindows::getFiles( HANDLE& SearchHandle, WIN32_FIND_DATA& FileData, BcPath Path )
{
	BcBool bValid = BcFalse;

	if ( SearchHandle == NULL )
	{
		BcPath NewPath( Path );
		NewPath.join( "*.*" );
		SearchHandle = ::FindFirstFile( (*NewPath).c_str(), &FileData );
		bValid = ( SearchHandle == INVALID_HANDLE_VALUE ) ? BcFalse : BcTrue;
	}
	else
	{
		bValid = ::FindNextFile( SearchHandle, &FileData );
	}

	// Loop for em
	while( bValid )
	{	
		if ( strcmpi( FileData.cFileName, "." ) != 0 &&
		     strcmpi( FileData.cFileName, ".." ) != 0 )
		{
			return BcTrue;
		}
		
		bValid = ::FindNextFile( SearchHandle, &FileData );
	}

	::FindClose( SearchHandle );
	SearchHandle = NULL;

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// addReadOp
class Job_ReadOp:
	public SysJob
{
public:
	Job_ReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ):
		pImpl_( pImpl ),
		Position_( Position ),
		pData_( pData ),
		Bytes_( Bytes ),
		DoneCallback_( DoneCallback )
	{
		
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->read( pData_, Bytes_ );
		SysKernel::pImpl()->enqueueCallback( DoneCallback_, pData_, Bytes_ );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpDelegate DoneCallback_;
};

void FsCoreImplWindows::addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	SysKernel::pImpl()->enqueueJob( FsCore::WORKER_MASK, new Job_ReadOp( pImpl, Position, pData, Bytes, DoneCallback ) );
}

//////////////////////////////////////////////////////////////////////////
// addWriteOp
class Job_WriteOp:
	public SysJob
{
public:
	Job_WriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ):
		pImpl_( pImpl ),
		Position_( Position ),
		pData_( pData ),
		Bytes_( Bytes ),
		DoneCallback_( DoneCallback )
	{
		
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->write( pData_, Bytes_ );
		SysKernel::pImpl()->enqueueCallback( DoneCallback_, pData_, Bytes_ );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpDelegate DoneCallback_;
};


void FsCoreImplWindows::addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	SysKernel::pImpl()->enqueueJob( FsCore::WORKER_MASK, new Job_WriteOp( pImpl, Position, pData, Bytes, DoneCallback ) );
}

//////////////////////////////////////////////////////////////////////////
// addFileMonitor
void FsCoreImplWindows::addFileMonitor( const BcChar* pFilename )
{
	BcScopedLock< BcMutex > Lock( FileMonitorLock_ );
	
	// Attempt to find it.
	TFileMonitorMapIterator FoundIter = FileMonitorMap_.find( pFilename );
	
	if( FoundIter == FileMonitorMap_.end() )
	{
		FsStats FileStats;
		
		// Try to get stats, and add file.
		if( fileStats( pFilename, FileStats ) )
		{
			// Add to monitor list.
			FileMonitorMap_[ pFilename ] = FileStats;

			// Reset monitor iterator.
			FileMonitorMapIterator_ = FileMonitorMap_.begin();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// removeFileMonitor
void FsCoreImplWindows::removeFileMonitor( const BcChar* pFilename )
{
	BcScopedLock< BcMutex > Lock( FileMonitorLock_ );

	// Attempt to find it.
	TFileMonitorMapIterator FoundIter = FileMonitorMap_.find( pFilename );

	if( FoundIter != FileMonitorMap_.end() )
	{
		// Remove from monitor list.
		FileMonitorMap_.erase( FoundIter );

		// Reset monitor iterator.
		FileMonitorMapIterator_ = FileMonitorMap_.begin();
	}
}

//////////////////////////////////////////////////////////////////////////
// updateFileMonitoring
void FsCoreImplWindows::updateFileMonitoring()
{
	BcScopedLock< BcMutex > Lock( FileMonitorLock_ );

	// Check 1 file per update to prevent slowdown.
	if( FileMonitorMapIterator_ == FileMonitorMap_.end() )
	{
		FileMonitorMapIterator_ = FileMonitorMap_.begin();
	}
	else
	{
		// Grab file stats.
		const std::string& FileName = (*FileMonitorMapIterator_).first;
		FsStats& OldFileStats =  (*FileMonitorMapIterator_).second;
		FsStats NewFileStats;
		if( fileStats( FileName.c_str(), NewFileStats ) )
		{
			// Compare timestamps.
			if( NewFileStats.ModifiedTime_ != OldFileStats.ModifiedTime_ )
			{
				// Publish message that file has changed/been created.
				if( OldFileStats.ModifiedTime_.isNull() == BcTrue )
				{
					BcPrintf( "FsCoreImplWindows: File created: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_CREATED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
				else
				{
					BcPrintf( "FsCoreImplWindows: File modified: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_MODIFIED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
			}
		}
		else
		{
			// Publish message that file has been deleted.
			if( OldFileStats.ModifiedTime_.isNull() == BcFalse )
			{
				BcPrintf( "FsCoreImplWindows: File deleted: %s\n", FileName.c_str() );
				EvtPublisher::publish( fsEVT_MONITOR_DELETED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
			}
		}
		
		// Store new stats.
		OldFileStats = NewFileStats;
		
		// Advance to next file.		
		++FileMonitorMapIterator_;
	}
}

