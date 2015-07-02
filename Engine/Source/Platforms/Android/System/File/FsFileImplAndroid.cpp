/**************************************************************************
*
* File:		FsFileImplAndroid.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsFileImplAndroid.h"

#include "System/File/FsCore.h"
#include "System/File/FsCoreImplAndroid.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
FsFileImplAndroid::FsFileImplAndroid():
	pFileHandle_( NULL ),
	FileSize_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsFileImplAndroid::~FsFileImplAndroid()
{
	BcAssert( pFileHandle_ == NULL );
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
BcBool FsFileImplAndroid::open( const BcChar* FileName, eFsFileMode FileMode )
{
	BcBool RetVal = BcFalse;

	if( pFileHandle_ == NULL )
	{
		// Read access.
		if( FileMode == fsFM_READ )
		{
			// TODO: Proper remapping.
			FileName_ = std::string( "/sdcard/" ) + FileName;

			pFileHandle_ = fopen( FileName_.c_str(), "rb" );
			if( pFileHandle_ != NULL )
			{
				fseek( pFileHandle_, 0, SEEK_END );
				FileSize_ = ftell( pFileHandle_ );
				fseek( pFileHandle_, 0, SEEK_SET );
			}
		}
		else if( FileMode == fsFM_WRITE )
		{
			// TODO: Proper remapping.
			FileName_ = std::string( "/sdcard/" ) + FileName;

			pFileHandle_ = fopen( FileName_.c_str(), "wb+" );
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
BcBool FsFileImplAndroid::close()
{
	BcBool RetVal = BcFalse;

	if( pFileHandle_ != NULL )
	{
		fclose( pFileHandle_ );
		pFileHandle_ = NULL;
		FileSize_ = 0;
		FileName_.clear();
		RetVal = BcTrue;
	}

	return RetVal;
}

//////////////////////////////////////////////////////////////////////////
// fileName
//virtual
const BcChar* FsFileImplAndroid::fileName() const
{
	return FileName_.c_str();
}

//////////////////////////////////////////////////////////////////////////
// size
//virtual
BcSize FsFileImplAndroid::size() const
{
	return (BcSize)FileSize_;
}

//////////////////////////////////////////////////////////////////////////
// tell
//virtual
BcSize FsFileImplAndroid::tell() const
{
	return (BcSize)ftell( pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// seek
//virtual
void FsFileImplAndroid::seek( BcSize Position )
{
	fseek( pFileHandle_, (long)Position, SEEK_SET );
}

//////////////////////////////////////////////////////////////////////////
// eof
//virtual
BcBool FsFileImplAndroid::eof() const
{
	return feof( pFileHandle_ ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// read
void FsFileImplAndroid::read( void* pDest, BcSize Bytes )
{
	fread( pDest, (long)Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// write
void FsFileImplAndroid::write( void* pSrc, BcSize Bytes )
{
	fwrite( pSrc, (long)Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// readAsync
//virtual
void FsFileImplAndroid::readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	FsCore::pImpl()->addReadOp( this, Position, pData, Bytes, DoneCallback );
}

//////////////////////////////////////////////////////////////////////////
// writeAsync
//virtual
void FsFileImplAndroid::writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	FsCore::pImpl()->addWriteOp( this, Position, pData, Bytes, DoneCallback );
}

