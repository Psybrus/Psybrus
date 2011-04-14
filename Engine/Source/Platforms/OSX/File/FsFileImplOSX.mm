/**************************************************************************
*
* File:		FsFileImplOSX.mm
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "FsFileImplOSX.h"

#include "FsCore.h"
#include "FsCoreImplOSX.h"

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
		RetVal = BcTrue;
	}

	return RetVal;
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
	fseek( pFileHandle_, Position, SEEK_SET );
}

//////////////////////////////////////////////////////////////////////////
// eof
//virtual
BcBool FsFileImplOSX::eof() const
{
	return ( feof( pFileHandle_ ) );
}

//////////////////////////////////////////////////////////////////////////
// read
void FsFileImplOSX::read( void* pDest, BcSize Bytes )
{
	fread( pDest, Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// write
void FsFileImplOSX::write( void* pSrc, BcSize Bytes )
{
	fwrite( pSrc, Bytes, 1, pFileHandle_ );
}

//////////////////////////////////////////////////////////////////////////
// readAsync
//virtual
void FsFileImplOSX::readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	FsCore::pImpl()->addReadOp( this, Position, pData, Bytes, DoneCallback );
}

//////////////////////////////////////////////////////////////////////////
// writeAsync
//virtual
void FsFileImplOSX::writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback )
{
	FsCore::pImpl()->addWriteOp( this, Position, pData, Bytes, DoneCallback );
}


