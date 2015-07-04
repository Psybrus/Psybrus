/**************************************************************************
*
* File:		FsCoreImplOSX.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsCoreImplOSX.h"
#include "System/File/FsFileImplOSX.h"

#include "System/SysKernel.h"

#include "Base/BcProfiler.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#if PSY_USE_PROFILER
#include <boost/format.hpp>
#endif

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( FsCoreImplOSX );

//////////////////////////////////////////////////////////////////////////
// Ctor
FsCoreImplOSX::FsCoreImplOSX()
{
	// Create our job queue.
	// 1 thread if we have 1 or more hardware thread.
	FsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 1 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsCoreImplOSX::~FsCoreImplOSX()
{

}

//////////////////////////////////////////////////////////////////////////
// init
//virtual
void FsCoreImplOSX::open()
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
void FsCoreImplOSX::update()
{
	// Update file monitoring.
	updateFileMonitoring();
}

//////////////////////////////////////////////////////////////////////////
// shutdown
//virtual
void FsCoreImplOSX::close()
{
	
}

//////////////////////////////////////////////////////////////////////////
// newFileImpl
//virtual
FsFileImpl* FsCoreImplOSX::openFile( const BcChar* pFilename, eFsFileMode FileMode )
{
	FsFileImpl* pFileImpl = NULL;

	// PhysFS
	pFileImpl = new FsFileImplOSX();

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
void FsCoreImplOSX::closeFile( FsFileImpl* pFileImpl )
{
	delete pFileImpl;
}

//////////////////////////////////////////////////////////////////////////
// fileExists
//virtual
BcBool FsCoreImplOSX::fileExists( const BcChar* pFilename )
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
BcBool FsCoreImplOSX::fileStats( const BcChar* pFilename, FsStats& Stats )
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
void FsCoreImplOSX::findFiles( BcPath Path, BcBool Recursive, BcBool AddFolders, std::list< BcPath >& OutputPaths )
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// addReadOp
class Job_ReadOp:
	public SysJob
{
public:
	Job_ReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ):
		pImpl_( pImpl ),
		Position_( Position ),
		pData_( pData ),
		Bytes_( Bytes ),
		DoneCallback_( DoneCallback )
	{
		PSY_PROFILER_START_ASYNC( "FsCoreImplOSX::addReadOp" );	
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->read( pData_, Bytes_ );
		PSY_PROFILER_FINISH_ASYNC( "FsCoreImplOSX::addReadOp" );	
		SysKernel::pImpl()->enqueueCallback( std::bind( DoneCallback_, pData_, Bytes_ ) );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpCallback DoneCallback_;
};

void FsCoreImplOSX::addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
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
	Job_WriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ):
		pImpl_( pImpl ),
		Position_( Position ),
		pData_( pData ),
		Bytes_( Bytes ),
		DoneCallback_( DoneCallback )
	{
		PSY_PROFILER_START_ASYNC( boost::str( boost::format( "FsCoreImplOSX::addWriteOp (%1%)" ) % pImpl_->fileName() ) );	
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->write( pData_, Bytes_ );
		PSY_PROFILER_FINISH_ASYNC( boost::str( boost::format( "FsCoreImplOSX::addWriteOp (%1%)" ) % pImpl_->fileName() ) );	
		SysKernel::pImpl()->enqueueCallback( std::bind( DoneCallback_, pData_, Bytes_ ) );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpCallback DoneCallback_;
};


void FsCoreImplOSX::addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	SysKernel::pImpl()->pushJob( FsCore::JOB_QUEUE_ID, new Job_WriteOp( pImpl, Position, pData, Bytes, DoneCallback ) );
}

//////////////////////////////////////////////////////////////////////////
// addFileMonitor
void FsCoreImplOSX::addFileMonitor( const BcChar* pFilename )
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
void FsCoreImplOSX::removeFileMonitor( const BcChar* pFilename )
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
void FsCoreImplOSX::updateFileMonitoring()
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
					PSY_LOG( "FsCoreImplOSX: File created: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_CREATED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
				else
				{
					PSY_LOG( "FsCoreImplOSX: File modified: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_MODIFIED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
			}
		}
		else
		{
			// Publish message that file has been deleted.
			if( OldFileStats.ModifiedTime_.isNull() == BcFalse )
			{
				PSY_LOG( "FsCoreImplOSX: File deleted: %s\n", FileName.c_str() );
				EvtPublisher::publish( fsEVT_MONITOR_DELETED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
			}
		}
		
		// Store new stats.
		OldFileStats = NewFileStats;
		
		// Advance to next file.		
		++FileMonitorMapIterator_;
	}
}

