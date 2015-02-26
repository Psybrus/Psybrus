 /**************************************************************************
*
* File:		FsCoreImplWindows.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FSCOREIMPLWindows_H__
#define __FSCOREIMPLWindows_H__

#include "System/File/FsCore.h"


#include <mutex>

#include <deque>

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class FsFileImpl;

//////////////////////////////////////////////////////////////////////////
// FsCoreImpl
class FsCoreImplWindows:
	public FsCore
{
public:
	FsCoreImplWindows();
	virtual ~FsCoreImplWindows();
	
	virtual void open();
	virtual void update();
	virtual void close();
	
	virtual FsFileImpl* openFile( const BcChar* pFilename, eFsFileMode FileMode );
	virtual void closeFile( FsFileImpl* pFileImpl );
	virtual BcBool fileExists( const BcChar* pFilename );
	virtual BcBool fileStats( const BcChar* pFilename, FsStats& Stats );
	virtual void findFiles( BcPath StartPath, BcBool Recursive, BcBool AddFolders, std::list< BcPath >& OutputPaths );
	
	void addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback );
	void addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback );
	
	void addFileMonitor( const BcChar* pFilename );
	void removeFileMonitor( const BcChar* pFilename );

	void updateFileMonitoring();
	
private:
	BcBool getFiles( HANDLE& SearchHandle, WIN32_FIND_DATA& FileData, BcPath Path );


private:
	// File monitoring.
	typedef std::map< std::string, FsStats > TFileMonitorMap;
	typedef TFileMonitorMap::iterator TFileMonitorMapIterator;

	std::mutex FileMonitorLock_;
	TFileMonitorMap FileMonitorMap_;
	TFileMonitorMapIterator FileMonitorMapIterator_;
};

#endif

