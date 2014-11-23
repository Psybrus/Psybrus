/**************************************************************************
*
* File:		FsFileImplHTML5.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FsFileImplHTML5_H__
#define __FsFileImplHTML5_H__

#include "System/File/FsCore.h"
#include <atomic>

//////////////////////////////////////////////////////////////////////////
// FsFileImplHTML5
class FsFileImplHTML5:
	public FsFileImpl
{
public:
	FsFileImplHTML5();
	virtual ~FsFileImplHTML5();
	
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
	virtual void		readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	virtual void		writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	
private:	
	std::string			FileName_;
	FILE*				pFileHandle_;
	BcU64				FileSize_;
};

#endif
