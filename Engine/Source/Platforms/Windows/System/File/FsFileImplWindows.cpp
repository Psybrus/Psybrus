/**************************************************************************
*
* File:		FsFileImplWindows.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsFileImplWindows.h"

#include "System/File/FsCore.h"
#include "System/File/FsCoreImplWindows.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
FsFileImplWindows::FsFileImplWindows():
	pFileHandle_( NULL ),
	FileSize_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsFileImplWindows::~FsFileImplWindows()
{
	BcAssert( pFileHandle_ == NULL );
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
BcBool FsFileImplWindows::open( const BcChar* FileName, eFsFileMode FileMode )
{
	BcBool RetVal = BcFalse;

	if( pFileHandle_ == NULL )
	{
		// Read access.
		if( FileMode == fsFM_READ )
		{
			pFileHandle_ = fopen( FileName, "rb" );
			if( pFileHandle_ != NULL )
			{
				fseek( pFileHandle_, 0, SEEK_END );
				FileSize_ = ftell( pFileHandle_ );
				fseek( pFileHandle_, 0, SEEK_SET );
			}
		}
		else if( FileMode == fsFM_WRITE )
		{
			pFileHandle_ = fopen( FileName, "wb+" );
		}

		// Return value.
		if( pFileHandle_ != NULL )
		{
			RetVal = BcTrue;
		}
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
BcBool FsFileImplWindows::close()
{
	BcBool RetVal = BcFalse;

	if( pFileHandle_ != NULL )
	{
		fclose( pFileHandle_ );
		pFileHandle_ = NULL;
		FileSize_ = 0;
		RetVal = BcTrue;
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// size
//virtual
BcSize FsFileImplWindows::size() const
{
	return (BcSize)FileSize_;
}

//////////////////////////////////////////////////////////////////////////
// tell
//virtual
BcSize FsFileImplWindows::tell() const
{
	return (BcSize)ftell( pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// seek
//virtual
void FsFileImplWindows::seek( BcSize Position )
{
	fseek( pFileHandle_, Position, SEEK_SET );
}

//////////////////////////////////////////////////////////////////////////
// eof
//virtual
BcBool FsFileImplWindows::eof() const
{
	return feof( pFileHandle_ ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// read
void FsFileImplWindows::read( void* pDest, BcSize Bytes )
{
	fread( pDest, Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// write
void FsFileImplWindows::write( void* pSrc, BcSize Bytes )
{
	fwrite( pSrc, Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// readAsync
//virtual
void FsFileImplWindows::readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	FsCore::pImpl()->addReadOp( this, Position, pData, Bytes, DoneCallback );
}

//////////////////////////////////////////////////////////////////////////
// writeAsync
//virtual
void FsFileImplWindows::writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	FsCore::pImpl()->addWriteOp( this, Position, pData, Bytes, DoneCallback );
}


