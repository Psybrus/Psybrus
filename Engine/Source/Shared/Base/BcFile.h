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

#include "Base/BcTypes.h"
#include <stdio.h>

#if COMPILER_MSVC

#include <memory>

#endif

const BcU32 MAX_LINE_LENGTH = 1024;

/////////////////////////////////////////////////
// BcFileSystemExists
bool BcFileSystemExists( const char* Path );

/////////////////////////////////////////////////
// BcFileSystemRemove
bool BcFileSystemRemove( const char* Path );

/////////////////////////////////////////////////
// BcFileSystemRename
bool BcFileSystemRename( const char* SrcPath, const char* DestPath );

/////////////////////////////////////////////////
// BcFileSystemCreateDirectories
bool BcFileSystemCreateDirectories( const char* Path );

/////////////////////////////////////////////////
// BcFileSystemChangeDirectory
bool BcFileSystemChangeDirectory( const char* Path );

/////////////////////////////////////////////////
// File mode.
enum eBcFileMode
{
	bcFM_READ = 0,
	bcFM_WRITE,
	bcFM_WRITE_TEXT,
};

/////////////////////////////////////////////////
// BcFile
class BcFile
{
public:
	BcFile();
	~BcFile();
	
	BcBool open( const BcChar*, eBcFileMode = bcFM_READ );
	void close();
		
	size_t size() const;
	BcBool isOpen() const;
	BcBool eof();

	void flush();
	
	// Sync
	size_t tell();
	void seek( size_t Position );
	void read( void* pDest, size_t nBytes );
	void readLine( BcChar* pBuffer, size_t Size );
	
	/**
	 * Allocate buffer and read all bytes. Use BcMemFree to free when done.
	 */
	std::unique_ptr< BcU8[] > readAllBytes();

	void write( const void* pSrc, size_t nBytes );
	void writeLine( const BcChar* pText );

private:
	void calcFileSize(void);

private:
	//
	FILE*			FileHandle_;
	int				FileDescriptor_;
	eBcFileMode		AccessMode_;
	size_t			FileSize_;
};

inline size_t BcFile::size() const
{
	return FileSize_;
}

inline BcBool BcFile::isOpen() const
{
	return FileHandle_ != NULL;
}

#endif
