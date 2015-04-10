/**************************************************************************
*
* File:		FsCoreImplHTML5.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsCoreImplHTML5.h"
#include "System/File/FsFileImplHTML5.h"

#include "System/SysKernel.h"

#include "Base/BcProfiler.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////
// System Creator
SYS_CREATOR( FsCoreImplHTML5 );

//////////////////////////////////////////////////////////////////////////
// Ctor
FsCoreImplHTML5::FsCoreImplHTML5()
{
	// Create our job queue.
	// 1 thread if we have 1 or more hardware thread.
	FsCore::JOB_QUEUE_ID = SysKernel::pImpl()->createJobQueue( 1, 1 );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsCoreImplHTML5::~FsCoreImplHTML5()
{

}

//////////////////////////////////////////////////////////////////////////
// init
//virtual
void FsCoreImplHTML5::open()
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
void FsCoreImplHTML5::update()
{
	// Update file monitoring.
	updateFileMonitoring();
}

//////////////////////////////////////////////////////////////////////////
// shutdown
//virtual
void FsCoreImplHTML5::close()
{
	
}

//////////////////////////////////////////////////////////////////////////
// newFileImpl
//virtual
FsFileImpl* FsCoreImplHTML5::openFile( const BcChar* pFilename, eFsFileMode FileMode )
{
	FsFileImpl* pFileImpl = NULL;

	// PhysFS
	pFileImpl = new FsFileImplHTML5();

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
void FsCoreImplHTML5::closeFile( FsFileImpl* pFileImpl )
{
	delete pFileImpl;
}

//////////////////////////////////////////////////////////////////////////
// fileExists
//virtual
BcBool FsCoreImplHTML5::fileExists( const BcChar* pFilename )
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
BcBool FsCoreImplHTML5::fileStats( const BcChar* pFilename, FsStats& Stats )
{
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// findFiles
void FsCoreImplHTML5::findFiles( BcPath Path, BcBool Recursive, BcBool AddFolders, std::list< BcPath >& OutputPaths )
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
		PSY_PROFILER_START_ASYNC( "FsCoreImplHTML5::addReadOp" );	
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->read( pData_, Bytes_ );
		PSY_PROFILER_FINISH_ASYNC( "FsCoreImplHTML5::addReadOp" );	
		SysKernel::pImpl()->enqueueCallback( std::bind( DoneCallback_, pData_, Bytes_ ) );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpCallback DoneCallback_;
};

void FsCoreImplHTML5::addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
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
		PSY_PROFILER_START_ASYNC( boost::str( boost::format( "FsCoreImplLinux::addWriteOp (%1%)" ) % pImpl_->fileName() ) );	
	}
	
	void execute()
	{
		pImpl_->seek( Position_ );
		pImpl_->write( pData_, Bytes_ );
		PSY_PROFILER_FINISH_ASYNC( boost::str( boost::format( "FsCoreImplLinux::addWriteOp (%1%)" ) % pImpl_->fileName() ) );	
		SysKernel::pImpl()->enqueueCallback( std::bind( DoneCallback_, pData_, Bytes_ ) );
	}
	
private:
	FsFileImpl* pImpl_;
	BcSize Position_;
	void* pData_;
	BcSize Bytes_;
	FsFileOpCallback DoneCallback_;
};


void FsCoreImplHTML5::addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	SysKernel::pImpl()->pushJob( FsCore::JOB_QUEUE_ID, new Job_WriteOp( pImpl, Position, pData, Bytes, DoneCallback ) );
}

//////////////////////////////////////////////////////////////////////////
// addFileMonitor
void FsCoreImplHTML5::addFileMonitor( const BcChar* pFilename )
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
void FsCoreImplHTML5::removeFileMonitor( const BcChar* pFilename )
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
void FsCoreImplHTML5::updateFileMonitoring()
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
					PSY_LOG( "FsCoreImplHTML5: File created: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_CREATED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
				else
				{
					PSY_LOG( "FsCoreImplHTML5: File modified: %s\n", FileName.c_str() );
					EvtPublisher::publish( fsEVT_MONITOR_MODIFIED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
				}
			}
		}
		else
		{
			// Publish message that file has been deleted.
			if( OldFileStats.ModifiedTime_.isNull() == BcFalse )
			{
				PSY_LOG( "FsCoreImplHTML5: File deleted: %s\n", FileName.c_str() );
				EvtPublisher::publish( fsEVT_MONITOR_DELETED, FsEventMonitor( FileName.c_str(), OldFileStats, NewFileStats ) );
			}
		}
		
		// Store new stats.
		OldFileStats = NewFileStats;
		
		// Advance to next file.		
		++FileMonitorMapIterator_;
	}
}

