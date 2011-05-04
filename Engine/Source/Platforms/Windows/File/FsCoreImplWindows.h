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

#include "FsCore.h"

#include "BcCommandBuffer.h"
#include "BcScopedLock.h"

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
	
	void addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	void addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	
	void addFileMonitor( const BcChar* pFilename );
	void removeFileMonitor( const BcChar* pFilename );

	void updateFileMonitoring();
	
private:
	BcCommandBuffer CommandBuffer_;

	// File monitoring.
	typedef std::map< std::string, FsStats > TFileMonitorMap;
	typedef TFileMonitorMap::iterator TFileMonitorMapIterator;

	BcMutex FileMonitorLock_;
	TFileMonitorMap FileMonitorMap_;
	TFileMonitorMapIterator FileMonitorMapIterator_;
};

#endif

