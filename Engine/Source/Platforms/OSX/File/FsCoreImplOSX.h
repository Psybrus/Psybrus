/**************************************************************************
*
* File:		FsCoreImplOSX.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __FSCOREIMPLOSX_H__
#define __FSCOREIMPLOSX_H__

#include "FsCore.h"

#include "BcCommandBuffer.h"

#include <deque>

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class FsFileImpl;

//////////////////////////////////////////////////////////////////////////
// FsCoreImpl
class FsCoreImplOSX:
	public FsCore
{
public:
	FsCoreImplOSX();
	virtual ~FsCoreImplOSX();
	
	virtual void open();
	virtual void update();
	virtual void close();
	
	virtual FsFileImpl* openFile( const BcChar* pFilename, eFsFileMode FileMode );
	virtual void closeFile( FsFileImpl* pFileImpl );
	virtual BcBool fileExists( const BcChar* pFilename );
	
	void addReadOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	void addWriteOp( FsFileImpl* pImpl, BcSize Position, void* pData, BcSize Bytes, FsFileOpDelegate DoneCallback );
	
private:
	BcCommandBuffer CommandBuffer_;
};

#endif

