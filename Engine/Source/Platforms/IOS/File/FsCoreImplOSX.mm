/**************************************************************************
*
* File:		FsCoreImplOSX.mm
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "FsCoreImplOSX.h"
#include "FsFileImplOSX.h"

#include "SysKernel.h"

//////////////////////////////////////////////////////////////////////////
// System Creator
extern "C"
{
	SYS_CREATOR( FsCoreImplOSX );
}

//////////////////////////////////////////////////////////////////////////
// Ctor
FsCoreImplOSX::FsCoreImplOSX()
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
FsCoreImplOSX::~FsCoreImplOSX()
{

}

//////////////////////////////////////////////////////////////////////////
// init
//virtual
void FsCoreImplOSX::open()
{
	int RetVal = 0;
	
	// Set the resource path.
	//[[NSFileManager defaultManager] changeCurrentDirectoryPath:@"/Users/neilo/Documents/Dev/Psybrus/Examples/TestBed"];
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void FsCoreImplOSX::update()
{
	// Execute the command buffer.
	CommandBuffer_.execute();
}

//////////////////////////////////////////////////////////////////////////
// shutdown
//virtual
void FsCoreImplOSX::close()
{
	
}

//////////////////////////////////////////////////////////////////////////
// newFileImpl
//virtual
FsFileImpl* FsCoreImplOSX::openFile( const BcChar* pFilename, eFsFileMode FileMode )
{
	FsFileImpl* pFileImpl = NULL;

	// PhysFS
	pFileImpl = new FsFileImplOSX();

	// Check for error or success.
	if( pFileImpl->open( pFilename, FileMode ) == BcFalse )
	{
		delete pFileImpl;
		pFileImpl = NULL;
	}
	
	return pFileImpl;
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void FsCoreImplOSX::closeFile( FsFileImpl* pFileImpl )
{
	delete pFileImpl;
}

//////////////////////////////////////////////////////////////////////////
// fileExists
//virtual
BcBool FsCoreImplOSX::fileExists( const BcChar* pFilename )
{
	FILE* pHandle = NULL;
	pHandle = fopen( pFilename, "rb" );
	if( pHandle != NULL )
	{
		fclose( pHandle );
	}
	return pHandle != NULL;
}

//////////////////////////////////////////////////////////////////////////
// addReadOp
void FsCoreImplOSX::addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	// Add a read op to the command buffer.
	class FileReadOp: public BcCommand
	{
	public:
		FileReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ):
			pImpl_( pImpl ),
			Position_( Position ),
			pData_( pData ),
			Bytes_( Bytes ),
			DoneCallback_( DoneCallback )
		{
			
		}
		
		virtual void execute()
		{
			pImpl_->seek( Position_ );
			pImpl_->read( pData_, Bytes_ );
			DoneCallback_( pData_, Bytes_ );
		}
		
	private:
		FsFileImpl* 	pImpl_;
		BcSize 			Position_;
		void* 			pData_;
		BcSize 			Bytes_;
		FsFileOpCallback DoneCallback_;
	};
	
	// Add command to the buffer.
	CommandBuffer_.push( new FileReadOp( pImpl, Position, pData, Bytes, DoneCallback ) );
}

//////////////////////////////////////////////////////////////////////////
// addWriteOp
void FsCoreImplOSX::addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback )
{
	// Add a write op to the command buffer.
	class FileWriteOp: public BcCommand
	{
	public:
		FileWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpCallback DoneCallback ):
			pImpl_( pImpl ),
			Position_( Position ),
			pData_( pData ),
			Bytes_( Bytes ),
			DoneCallback_( DoneCallback )
		{
			
		}
		
		virtual void execute()
		{
			pImpl_->seek( Position_ );
			pImpl_->write( pData_, Bytes_ );
			DoneCallback_( pData_, Bytes_ );
		}
		
	private:
		FsFileImpl* 	pImpl_;
		BcSize 			Position_;
		void* 			pData_;
		BcSize 			Bytes_;
		FsFileOpCallback DoneCallback_;
	};
	
	// Add command to the buffer.
	CommandBuffer_.push( new FileWriteOp( pImpl, Position, pData, Bytes, DoneCallback ) );
						
}
