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
	pImpl_( NULL )
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
	BcAssertMsg( FsCore::pImpl() != NULL, "FsFile: FsCore is NULL when trying to open a file!" );
	if( FsCore::pImpl() != NULL )
	{
		pImpl_ = FsCore::pImpl()->openFile( FileName, FileMode );
	}
	
	return pImpl_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// close
BcBool FsFile::close()
{
	BcBool RetVal = BcFalse;
	
	// If we have an implementation close and delete it.
	if( pImpl_ != NULL )
	{
		RetVal = pImpl_->close();
		if( FsCore::pImpl() != NULL )
		{
			FsCore::pImpl()->closeFile( pImpl_ );
		}
		else
		{
			BcPrintf( "FsFile: FsCore is NULL when trying to close a file!\n" );
		}
	}
	
	return RetVal;	
}
