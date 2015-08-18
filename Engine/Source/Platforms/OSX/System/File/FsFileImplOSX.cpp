/**************************************************************************
*
* File:		FsFileImplOSX.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/File/FsFileImplOSX.h"

#include "System/File/FsCore.h"
#include "System/File/FsCoreImplOSX.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
FsFileImplOSX::FsFileImplOSX():
	pFileHandle_( NULL ),
	FileSize_( 0 )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsFileImplOSX::~FsFileImplOSX()
{
	BcAssert( pFileHandle_ == NULL );
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
BcBool FsFileImplOSX::open( const BcChar* FileName, eFsFileMode FileMode )
{
	BcBool RetVal = BcFalse;

	if( pFileHandle_ == NULL )
	{
		FileName_ = FileName;

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
BcBool FsFileImplOSX::close()
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
const BcChar* FsFileImplOSX::fileName() const
{
	return FileName_.c_str();
}

//////////////////////////////////////////////////////////////////////////
// size
//virtual
BcSize FsFileImplOSX::size() const
{
	return (BcSize)FileSize_;
}

//////////////////////////////////////////////////////////////////////////
// tell
//virtual
BcSize FsFileImplOSX::tell() const
{
	return (BcSize)ftell( pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// seek
//virtual
void FsFileImplOSX::seek( BcSize Position )
{
	fseek( pFileHandle_, (long)Position, SEEK_SET );
}

//////////////////////////////////////////////////////////////////////////
// eof
//virtual
BcBool FsFileImplOSX::eof() const
{
	return feof( pFileHandle_ ) != 0;
}

//////////////////////////////////////////////////////////////////////////
// read
void FsFileImplOSX::read( void* pDest, BcSize Bytes )
{
	fread( pDest, (long)Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// write
void FsFileImplOSX::write( void* pSrc, BcSize Bytes )
{
	fwrite( pSrc, (long)Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// readAsync
//virtual
void FsFileImplOSX::readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	FsCore::pImpl()->addReadOp( this, Position, pData, Bytes, DoneCallback );
}

//////////////////////////////////////////////////////////////////////////
// writeAsync
//virtual
void FsFileImplOSX::writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	FsCore::pImpl()->addWriteOp( this, Position, pData, Bytes, DoneCallback );
}


