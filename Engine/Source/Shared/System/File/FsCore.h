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

#include "FsTypes.h"
#include "FsEvents.h"
#include "BcGlobal.h"
#include "BcPath.h"
#include "SysSystem.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class OsBaseState;
class OsInputDevice;

//////////////////////////////////////////////////////////////////////////
// eFsFileMode
enum eFsFileMode
{
	fsFM_READ = 0,
	fsFM_WRITE,
};

//////////////////////////////////////////////////////////////////////////
// File operation delegate
typedef BcDelegate< void(*)( void*, BcSize ) > FsFileOpDelegate;

//////////////////////////////////////////////////////////////////////////
/** \class FsFileImpl
*	\brief File implementation.
*
*	Raw file implementation, wrapped by FsFile for more generalised use.
*/
class FsFileImpl
{
public:
	virtual ~FsFileImpl(){};

	/**
	*	Open file in specified mode.
	*/
	virtual BcBool open( const BcChar* FileName, eFsFileMode FileMode ) = 0;

	/**
	*	Close file.
	*/
	virtual BcBool close() = 0;

	/**
	*	Get size of file in bytes.
	*/
	virtual BcSize size() const = 0;

	/**
	*	Get current position in file.
	*/
	virtual BcSize tell() const = 0;
	
	/**
	*	Seek to a position.
	*/
	virtual void seek( BcSize Position ) = 0;
	
	/**
	*	Syncronously read data.
	*/
	virtual void read( void* pDest, BcSize Bytes ) = 0;

	/**
	*	Syncronously write data.
	*/
	virtual void write( void* pDest, BcSize Bytes ) = 0;

	/**
	*	Stream data from current file position to specified location.	
	*/
	virtual void readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ) = 0;
	
	/**
	*	Stream data from current file position to specified location.	
	*/
	virtual void writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ) = 0;
};

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
	static BcU32 WORKER_MASK;

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
	virtual void findFiles( BcPath StartPath, BcBool Recursive, BcBool AddThisPath, std::list< BcPath >& OutputPaths ) = 0;
	
	/**
	 *	Add a read operation.
	 */	
	virtual void addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ) = 0;

	/**
	 *	Add a write operation.
	 */	
	virtual void addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback ) = 0;

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
