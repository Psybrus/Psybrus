/**************************************************************************
*
* File:		FsCore.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FSCORE_H__
#define __FSCORE_H__

#include "System/File/FsTypes.h"
#include "System/File/FsEvents.h"
#include "Base/BcGlobal.h"
#include "Base/BcPath.h"
#include "System/SysSystem.h"

#include <functional>

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class OsBaseState;
class OsInputDevice;
class FsFileImpl;

//////////////////////////////////////////////////////////////////////////
/**	\class FsCore
*	\brief File System Core
*
*	Global accessor to FsCoreImpl.
*/
class FsCore:
	public BcGlobal< FsCore >,
	public SysSystem
{
public:
	static size_t JOB_QUEUE_ID;

public:
	/**
	 *	Open file implementation.
	 */
	virtual FsFileImpl* openFile( const BcChar* pFilename, eFsFileMode FileMode ) = 0;

	/**
	 *	Close file implementation.
	 */
	virtual void closeFile( FsFileImpl* pFileImpl ) = 0;
	
	/**
	 *	Does a file exist?
	 */
	virtual BcBool fileExists( const BcChar* pFilename ) = 0;
	
	/**
	 * Get file stats.
	 */
	virtual BcBool fileStats( const BcChar* pFilename, FsStats& Stats ) = 0;

	/**
	 * Find files.
	 */
	virtual void findFiles( BcPath StartPath, BcBool Recursive, BcBool AddFolders, std::list< BcPath >& OutputPaths ) = 0;
	
	/**
	 *	Add a read operation.
	 */	
	virtual void addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ) = 0;

	/**
	 *	Add a write operation.
	 */	
	virtual void addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ) = 0;

	/**
	 * Add file monitor.
	 */
	virtual void addFileMonitor( const BcChar* pFilename ) = 0;

	/**
	 * Remove file monitor.
	 */
	virtual void removeFileMonitor( const BcChar* pFilename ) = 0;

	
private:

};

#endif
