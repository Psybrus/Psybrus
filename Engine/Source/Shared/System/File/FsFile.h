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

#include "System/File/FsCore.h"

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
	*	ASYNC call.
	*/
	void readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	
	/**
	*	Stream data to current file position to specified location.	
	*	ASYNC call.
	*/
	void writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	
	
private:
	FsFileImpl* pImpl_;

};

//////////////////////////////////////////////////////////////////////////
// Inlines
inline BcSize FsFile::size() const
{
	return pImpl_->size();
}

inline BcSize FsFile::tell() const
{
	return pImpl_->tell();
}

inline void FsFile::seek( BcSize Position )
{
	pImpl_->seek( Position );
}

inline void FsFile::readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	pImpl_->readAsync( Position, pData, Bytes, DoneCallback );
}

inline void FsFile::writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	pImpl_->writeAsync( Position, pData, Bytes, DoneCallback );
}
#endif
