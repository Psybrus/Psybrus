/**************************************************************************
*
* File:		FsFile.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		A wrapper class that behaves like the old BcFile class.
*		
*
*
* 
**************************************************************************/

#ifndef __FSFILE_H__
#define __FSFILE_H__

#include "System/File/FsTypes.h"

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
	 * Get file name.
	 */
	virtual const BcChar* fileName() const = 0;

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
	virtual void readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ) = 0;
	
	/**
	*	Stream data from current file position to specified location.	
	*/
	virtual void writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ) = 0;
};

//////////////////////////////////////////////////////////////////////////
/** \class FsFile
*	\brief Wraps up FsFileImpl and interfaces with FsCoreImpl automatically.
*/
class FsFile
{
public:
	FsFile();
	~FsFile();
	
	/**
	*	Open file in specified mode.
	*	SYNC call.
	*/
	BcBool open( const BcChar* FileName, eFsFileMode FileMode );
	
	/**
	*	Close file.
	*	SYNC call.
	*/
	BcBool close();
	
	/**
	*	Get size of file in bytes.
	*	SYNC call. Don't use if using ASYNC.
	*/
	BcSize size() const;
	
	/**
	*	Get current position in file.
	*	SYNC call. Don't use if using ASYNC.
	*/
	BcSize tell() const;
	
	/**
	*	Seek to a position.
	*	SYNC call. Don't use if using ASYNC.
	*/
	void seek( BcSize Position );
	
	/**
	*	Stream data from current file position to specified location.	
	*	SYNC call.
	*/
	void read( BcSize Position, void* pData, BcSize Bytes );
	
	/**
	*	Stream data to current file position to specified location.	
	*	SYNC call.
	*/
	void write( BcSize Position, void* pData, BcSize Bytes );

	/**
	*	Stream data from current file position to specified location.	
	*	ASYNC call.
	*/
	void readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback );
	
	/**
	*	Stream data to current file position to specified location.	
	*	ASYNC call.
	*/
	void writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback );
	
	
private:
	class FsFileImpl* pImpl_;

};

#endif
