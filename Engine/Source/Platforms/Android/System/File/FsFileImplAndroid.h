/**************************************************************************
*
* File:		FsFileImplAndroid.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FsFileImplAndroid_H__
#define __FsFileImplAndroid_H__

#include "System/File/FsCore.h"
#include "System/File/FsFile.h"
#include <atomic>

//////////////////////////////////////////////////////////////////////////
// FsFileImplAndroid
class FsFileImplAndroid:
	public FsFileImpl
{
public:
	FsFileImplAndroid();
	virtual ~FsFileImplAndroid();
	
	virtual BcBool		open( const BcChar* FileName, eFsFileMode FileMode );
	virtual BcBool		close();
	virtual const BcChar* fileName() const;
	virtual BcSize		size() const;
	virtual BcSize		tell() const;
	virtual void		seek( BcSize Position );
	virtual BcBool		eof() const;
	virtual void		read( void* pDest, BcSize Bytes );
	virtual void		write( void* pSrc, BcSize Bytes );
	
	//
	virtual void		readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback );
	virtual void		writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback );
	
private:	
	std::string			FileName_;
	FILE*				pFileHandle_;
	BcU64				FileSize_;
};

#endif
