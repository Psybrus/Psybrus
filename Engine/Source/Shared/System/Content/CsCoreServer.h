/**************************************************************************
*
* File:		CsCoreServer.h
* Author:	Neil Richardson 
* Ver/Date:	9/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSCORESERVER_H__
#define __CSCORESERVER_H__

#include "System/Content/CsCore.h"

//////////////////////////////////////////////////////////////////////////
// CsCoreServer
class CsCoreServer:
	public CsCore
{
public:
	CsCoreServer();
	virtual ~CsCoreServer();
	
	virtual void						open();
	virtual void						update();
	virtual void						close();

public:
	void delegateLoad( void* pData, BcU32 Bytes );
	void delegateGetChunk( void* pData, BcU32 Bytes );
	void delegateUnload( void* pData, BcU32 Bytes );
	
	void delegateFileReady( CsFile* pFile );
	void delegateFileChunkReady( CsFile* pFile, BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData );
	
private:
	typedef std::map< std::string, CsFile* > TFileMap;
	typedef TFileMap::iterator TFileMapIterator;
	
	TFileMap							FileMap_;
};


#endif
