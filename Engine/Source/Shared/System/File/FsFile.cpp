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

#include "FsFile.h"
#include "FsCore.h"

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
	pImpl_ = FsCore::pImpl()->openFile( FileName, FileMode );
	
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
		FsCore::pImpl()->closeFile( pImpl_ );
	}
	
	return RetVal;	
}
