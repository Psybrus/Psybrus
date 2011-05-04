/**************************************************************************
*
* File:		FsFileImplWindows.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FSFILEIMPLWindows_H__
#define __FSFILEIMPLWindows_H__

#include "FsCore.h"
#include "BcAtomic.h"

//////////////////////////////////////////////////////////////////////////
// FsFileImplWindows
class FsFileImplWindows:
	public FsFileImpl
{
public:
	FsFileImplWindows();
	virtual ~FsFileImplWindows();
	
	virtual BcBool		open( const BcChar* FileName, eFsFileMode FileMode );
	virtual BcBool		close();
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
	FILE*				pFileHandle_;
	BcU64				FileSize_;
};

#endif
