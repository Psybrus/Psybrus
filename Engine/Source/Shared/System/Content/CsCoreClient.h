/**************************************************************************
*
* File:		CsCoreClient.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSCORECLIENT_H__
#define __CSCORECLIENT_H__

#include "System/Content/CsCore.h"

#include "System/Content/CsFileReader.h"
#include "System/Content/CsFileReaderRPC.h"

//////////////////////////////////////////////////////////////////////////
// CsCoreClient
class CsCoreClient:
	public CsCore
{
public:
	CsCoreClient();
	virtual ~CsCoreClient();
	
	virtual void						open();
	virtual void						update();
	virtual void						close();
	
protected:
	virtual CsFile*						createFileReader( const std::string& FileName );

public:
	void delegateLoad( CsFileReaderRPC* pFile, CsFileReadyDelegate ReadyDelegate, CsFileChunkDelegate ChunkDelegate );
	void delegateGetChunk( CsFileReaderRPC*, BcU32 Chunk );
	void delegateUnload( CsFileReaderRPC* pFile );
	
	void delegateFileReady( void* pData, BcU32 Bytes );
	void delegateFileChunkReady( void* pData, BcU32 Bytes );
	
private:
	typedef std::map< std::string, CsFileReaderRPC* > TFileMap;
	typedef TFileMap::iterator TFileMapIterator;
	
	TFileMap							FileMap_;
};


#endif
