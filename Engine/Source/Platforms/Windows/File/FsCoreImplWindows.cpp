/**************************************************************************
*
* File:		FsCoreImplWindows.mm
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "FsCoreImplWindows.h"
#include "FsFileImplWindows.h"

#include "SysKernel.h"

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
	// Execute the command buffer.
	CommandBuffer_.execute();
	
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
	struct stat Attrib;
	/*
	if( stat( pFilename, &Attrib ) == 0 )
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
		
		return BcTrue;
	}
	*/
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// addReadOp
void FsCoreImplWindows::addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	// Add a read op to the command buffer.
	class FileReadOp: public BcCommand
	{
	public:
		FileReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ):
			pImpl_( pImpl ),
			Position_( Position ),
			pData_( pData ),
			Bytes_( Bytes ),
			DoneCallback_( DoneCallback )
		{
			
		}
		
		virtual void execute()
		{
			pImpl_->seek( Position_ );
			pImpl_->read( pData_, Bytes_ );
			DoneCallback_( pData_, Bytes_ );
		}
		
	private:
		FsFileImpl* 	pImpl_;
		BcSize 			Position_;
		void* 			pData_;
		BcSize 			Bytes_;
		FsFileOpDelegate DoneCallback_;
	};
	
	// Add command to the buffer.
	CommandBuffer_.push( new FileReadOp( pImpl, Position, pData, Bytes, DoneCallback ) );
}

//////////////////////////////////////////////////////////////////////////
// addWriteOp
void FsCoreImplWindows::addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	// Add a write op to the command buffer.
	class FileWriteOp: public BcCommand
	{
	public:
		FileWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ):
			pImpl_( pImpl ),
			Position_( Position ),
			pData_( pData ),
			Bytes_( Bytes ),
			DoneCallback_( DoneCallback )
		{
			
		}
		
		virtual void execute()
		{
			pImpl_->seek( Position_ );
			pImpl_->write( pData_, Bytes_ );
			DoneCallback_( pData_, Bytes_ );
		}
		
	private:
		FsFileImpl* 	pImpl_;
		BcSize 			Position_;
		void* 			pData_;
		BcSize 			Bytes_;
		FsFileOpDelegate DoneCallback_;
	};
	
	// Add command to the buffer.
	CommandBuffer_.push( new FileWriteOp( pImpl, Position, pData, Bytes, DoneCallback ) );
						
}

//////////////////////////////////////////////////////////////////////////
// addFileMonitor
void FsCoreImplWindows::addFileMonitor( const BcChar* pFilename )
{
	/*
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
	*/
}

//////////////////////////////////////////////////////////////////////////
// removeFileMonitor
void FsCoreImplWindows::removeFileMonitor( const BcChar* pFilename )
{
	/*
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
	*/
}

//////////////////////////////////////////////////////////////////////////
// updateFileMonitoring
void FsCoreImplWindows::updateFileMonitoring()
{
	/*
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
	*/
}

