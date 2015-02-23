/**************************************************************************
*
* File:		FsCoreImplLinux.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsCoreImplLinux.h"
#include "System/File/FsFileImplLinux.h"

#include "System/SysKernel.h"

#include "Base/BcProfiler.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( FsCoreImplLinux );

//////////////////////////////////////////////////////////////////////////
// Ctor
FsCoreImplLinux::FsCoreImplLinux()
{
	// Create our job queue.
	// 1 thread if we have 1 or more hardware thread.
	FsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 1 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsCoreImplLinux::~FsCoreImplLinux()
{

}

//////////////////////////////////////////////////////////////////////////
// init
//virtual
void FsCoreImplLinux::open()
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
void FsCoreImplLinux::update()
{
	// Update file monitoring.
	updateFileMonitoring();
}

//////////////////////////////////////////////////////////////////////////
// shutdown
//virtual
void FsCoreImplLinux::close()
{
	
}

//////////////////////////////////////////////////////////////////////////
// newFileImpl
//virtual
FsFileImpl* FsCoreImplLinux::openFile( const BcChar* pFilename, eFsFileMode FileMode )
{
	FsFileImpl* pFileImpl = NULL;

	// PhysFS
	pFileImpl = new FsFileImplLinux();

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
void FsCoreImplLinux::closeFile( FsFileImpl* pFileImpl )
{
	delete pFileImpl;
}

//////////////////////////////////////////////////////////////////////////
// fileExists
//virtual
BcBool FsCoreImplLinux::fileExists( const BcChar* pFilename )
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
BcBool FsCoreImplLinux::fileStats( const BcChar* pFilename, FsStats& Stats )
{
	struct stat Attrib;

	int Descriptor = ::open(pFilename, O_RDONLY);
	if( Descriptor != -1 )
	{
		if( fstat( Descriptor, &Attrib ) == 0 )
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
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// findFiles
void FsCoreImplLinux::findFiles( BcPath Path, BcBool Recursive, BcBool AddFolders, std::list< BcPath >& OutputPaths )
{
	BcBreakpoint;
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
		PSY_PROFILER_START_ASYNC( "FsCoreImplLinux::addReadOp" );	
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->read( pData_, Bytes_ );
		PSY_PROFILER_FINISH_ASYNC( "FsCoreImplLinux::addReadOp" );	
		SysKernel::pImpl()->enqueueCallback( DoneCallback_, pData_, Bytes_ );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpDelegate DoneCallback_;
};

void FsCoreImplLinux::addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	BcAssert( Bytes > 0 );
	SysKernel::pImpl()->pushJob( FsCore::JOB_QUEUE_ID, new Job_ReadOp( pImpl, Position, pData, Bytes, DoneCallback ) );
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
		PSY_PROFILER_START_ASYNC( boost::str( boost::format( "FsCoreImplLinux::addWriteOp (%1%)" ) % pImpl_->fileName() ) );	
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->write( pData_, Bytes_ );
		PSY_PROFILER_FINISH_ASYNC( boost::str( boost::format( "FsCoreImplLinux::addWriteOp (%1%)" ) % pImpl_->fileName() ) );	
		SysKernel::pImpl()->enqueueCallback( DoneCallback_, pData_, Bytes_ );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpDelegate DoneCallback_;
};


void FsCoreImplLinux::addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	SysKernel::pImpl()->pushJob( FsCore::JOB_QUEUE_ID, new Job_WriteOp( pImpl, Position, pData, Bytes, DoneCallback ) );
}

//////////////////////////////////////////////////////////////////////////
// addFileMonitor
void FsCoreImplLinux::addFileMonitor( const BcChar* pFilename )
{
	std::lock_guard< std::mutex > Lock( FileMonitorLock_ );
	
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
void FsCoreImplLinux::removeFileMonitor( const BcChar* pFilename )
{
	std::lock_guard< std::mutex > Lock( FileMonitorLock_ );

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
void FsCoreImplLinux::updateFileMonitoring()
{
	std::lock_guard< std::mutex > Lock( FileMonitorLock_ );

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
					PSY_LOG( "FsCoreImplLinux: File created: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_CREATED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
				else
				{
					PSY_LOG( "FsCoreImplLinux: File modified: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_MODIFIED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
			}
		}
		else
		{
			// Publish message that file has been deleted.
			if( OldFileStats.ModifiedTime_.isNull() == BcFalse )
			{
				PSY_LOG( "FsCoreImplLinux: File deleted: %s\n", FileName.c_str() );
				EvtPublisher::publish( fsEVT_MONITOR_DELETED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
			}
		}
		
		// Store new stats.
		OldFileStats = NewFileStats;
		
		// Advance to next file.		
		++FileMonitorMapIterator_;
	}
}

