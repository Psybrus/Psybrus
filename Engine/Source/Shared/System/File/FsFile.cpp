/**************************************************************************
*
* File:		FsFile.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		A wrapper class that behaves like the old BcFile class.
*		
*
*
* 
**************************************************************************/

#include "System/File/FsFile.h"
#include "System/File/FsCore.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
FsFile::FsFile():
	pImpl_( nullptr )
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
FsFile::~FsFile()
{
	// Close file.
	close();
}

//////////////////////////////////////////////////////////////////////////
// open
BcBool FsFile::open( const BcChar* FileName, eFsFileMode FileMode )
{
	// Grab a new implementation.
	BcAssertMsg( FsCore::pImpl() != nullptr, "FsFile: FsCore is NULL when trying to open a file!" );
	if( FsCore::pImpl() != nullptr )
	{
		pImpl_ = FsCore::pImpl()->openFile( FileName, FileMode );
	}
	
	return pImpl_ != nullptr;
}

//////////////////////////////////////////////////////////////////////////
// close
BcBool FsFile::close()
{
	BcBool RetVal = BcFalse;
	
	// If we have an implementation close and delete it.
	if( pImpl_ != nullptr )
	{
		RetVal = pImpl_->close();
		if( FsCore::pImpl() != nullptr )
		{
			FsCore::pImpl()->closeFile( pImpl_ );
			pImpl_ = nullptr;
		}
		else
		{
			PSY_LOG( "FsFile: FsCore is NULL when trying to close a file!\n" );
		}
	}
	
	return RetVal;	
}

//////////////////////////////////////////////////////////////////////////
// size
BcSize FsFile::size() const
{
	return pImpl_->size();
}

//////////////////////////////////////////////////////////////////////////
// tell
BcSize FsFile::tell() const
{
	return pImpl_->tell();
}

//////////////////////////////////////////////////////////////////////////
// seek
void FsFile::seek( BcSize Position )
{
	pImpl_->seek( Position );
}

//////////////////////////////////////////////////////////////////////////
// read
void FsFile::read( BcSize Position, void* pData, BcSize Bytes )
{
	pImpl_->seek( Position );
	pImpl_->read( pData, Bytes );
}

//////////////////////////////////////////////////////////////////////////
// write
void FsFile::write( BcSize Position, void* pData, BcSize Bytes )
{
	pImpl_->seek( Position );
	pImpl_->write( pData, Bytes );
}

//////////////////////////////////////////////////////////////////////////
// readAsync
void FsFile::readAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	if( Bytes > 0 )
	{
		pImpl_->readAsync( Position, pData, Bytes, DoneCallback );
	}
	else
	{
		DoneCallback( pData, Bytes );
	}
}

//////////////////////////////////////////////////////////////////////////
// writeAsync
void FsFile::writeAsync( BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	if( Bytes > 0 )
	{
		pImpl_->writeAsync( Position, pData, Bytes, DoneCallback );
	}
	else
	{
		DoneCallback( pData, Bytes );
	}
}
