/**************************************************************************
*
* File:		BcFile.h
* Author: 	Neil Richardson
* Ver/Date:	
* Description:
*		
*		
*		
*		
* 
**************************************************************************/
#ifndef __BCFILE_H__
#define __BCFILE_H__

#include "BcTypes.h"
#include <stdio.h>

const BcU32 MAX_LINE_LENGTH = 1024;

enum eBcFileMode
{
	bcFM_READ = 0,
	bcFM_WRITE,
};

/////////////////////////////////////////////////
// class
class BcFile
{
public:
	BcFile();
	~BcFile();
	
	BcBool open( const BcChar*, eBcFileMode = bcFM_READ );
	void close();
		
	BcU32 size() const;
	BcBool eof();

	void flush();

	// Sync
	BcU32 tell();
	void seek( BcU32 Position );
	void read( void* pDest, BcU32 nBytes );
	void readLine( BcChar* pBuffer, BcU32 Size );
	void write( const void* pSrc, BcU32 nBytes );
	void writeLine( const BcChar* pText );

private:
	void calcFileSize(void);

private:
	//
	FILE*			FileHandle_;
	int				FileDescriptor_;
	eBcFileMode		AccessMode_;
	BcU32			FileSize_;
};

inline BcU32 BcFile::size() const
{
	return FileSize_;
}

#endif
