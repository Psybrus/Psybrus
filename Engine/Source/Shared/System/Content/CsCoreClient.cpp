/**************************************************************************
*
* File:		CsCoreClient.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/


#include "CsCoreClient.h"

#include "BcHash.h"
#include "RmCore.h"

SYS_CREATOR( CsCoreClient );

#define DbgTrace BcPrintf
//#define DbgTrace(...)

//////////////////////////////////////////////////////////////////////////
// Ctor
CsCoreClient::CsCoreClient()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsCoreClient::~CsCoreClient()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void CsCoreClient::open()
{
	// Register delegates.
	if( RmCore::pImpl() != NULL )
	{
		RmCore::pImpl()->registerDelegate( BcHash( "CsCore::delegateFileReady" ),		RmDelegate::bind< CsCoreClient, &CsCoreClient::delegateFileReady >( this ) );
		RmCore::pImpl()->registerDelegate( BcHash( "CsCore::delegateFileChunkReady" ),	RmDelegate::bind< CsCoreClient, &CsCoreClient::delegateFileChunkReady >( this ) );
	}
	
	CsCore::open();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void CsCoreClient::update()
{
	CsCore::update();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void CsCoreClient::close()
{
	// Unregister delegates.
	if( RmCore::pImpl() != NULL )
	{
		RmCore::pImpl()->unregisterDelegate( BcHash( "CsCore::delegateFileReady" ) );	
		RmCore::pImpl()->unregisterDelegate( BcHash( "CsCore::delegateFileChunkReady" ) );
	}
	
	CsCore::close();
}

//////////////////////////////////////////////////////////////////////////
// delegateLoad
//virtual
CsFile* CsCoreClient::createFileReader( const std::string& FileName )
{
	if( RmCore::pImpl()->isConnected() )
	{
		return new CsFileReaderRPC( FileName );
	}
	return new CsFileReader( FileName );
}

//////////////////////////////////////////////////////////////////////////
// delegateLoad
void CsCoreClient::delegateLoad( CsFileReaderRPC* pFile, CsFileReadyDelegate ReadyDelegate, CsFileChunkDelegate ChunkDelegate )
{
	// Put into map.
	FileMap_[ pFile->getName() ] = pFile;

	// Setup parameters.
	RmParameters Params( RmParameters::TYPE_SEND );
	Params << pFile->getName();
		
	// Do RPC.
	RmCore::pImpl()->send( BcHash( "CsCore::delegateLoad" ), Params );	
}

//////////////////////////////////////////////////////////////////////////
// delegateGetChunk
void CsCoreClient::delegateGetChunk( CsFileReaderRPC* pFile, BcU32 Chunk )
{
	// Setup parameters.
	RmParameters Params( RmParameters::TYPE_SEND );
	Params << pFile->getName() << Chunk;
	
	// Do RPC.	
	RmCore::pImpl()->send( BcHash( "CsCore::delegateGetChunk" ), Params );	
}

//////////////////////////////////////////////////////////////////////////
// delegateUnload
void CsCoreClient::delegateUnload( CsFileReaderRPC* pFile )
{
	// Setup parameters.
	RmParameters Params( RmParameters::TYPE_SEND );
	Params << pFile->getName();

	// Do RPC.
	RmCore::pImpl()->send( BcHash( "CsCore::delegateUnload" ), NULL, 0 );	

	// Remove from map.
	TFileMapIterator Iter = FileMap_.find( pFile->getName() );
	if( Iter != FileMap_.end() )
	{
		FileMap_.erase( Iter );
	}
}

//////////////////////////////////////////////////////////////////////////
// delegateFileReady
void CsCoreClient::delegateFileReady( void* pData, BcU32 Bytes )
{
	// Get params.
	RmParameters Params( RmParameters::TYPE_RECV, pData, Bytes );
	
	// Parse in name.
	std::string Name;
	Params >> Name;
	
	// Lookup file and bridge.
	TFileMapIterator Iter = FileMap_.find( Name );
	if( Iter != FileMap_.end() )
	{
		CsFileReaderRPC* pFile = (*Iter).second;
	
		// Parse the header.
		CsFileHeader Header;
		Params >> Header;
		
		DbgTrace( "CsCoreClient::delegateFileReady: %s, ID %u, NoofChunks %u\n", Name.c_str(), Header.ID_, Header.NoofChunks_ );

		pFile->onHeaderLoaded( &Header, sizeof( Header ) );
	
		// Parse the chunks. (Rest of the parameter data will be all chunks)
		pFile->onChunksLoaded( Params.getDataFromCursor(), Params.getDataSizeFromCursor() );
	}
}

//////////////////////////////////////////////////////////////////////////
// delegateFileChunkReady
void CsCoreClient::delegateFileChunkReady( void* pData, BcU32 Bytes )
{
	// Get params.
	RmParameters Params( RmParameters::TYPE_RECV, pData, Bytes );
	
	// Parse in name.
	std::string Name;
	Params >> Name;
	
	// Lookup file and bridge.
	TFileMapIterator Iter = FileMap_.find( Name );
	if( Iter != FileMap_.end() )
	{
		CsFileReaderRPC* pFile = (*Iter).second;
		
		DbgTrace( "CsCoreClient::delegateFileChunkReady: %s, Bytes %u\n", Name.c_str(), Params.getDataSizeFromCursor() - sizeof( BcU32 ) );

		// Parse in the chunk data. (Rest of the parameter data will be the chunk idx + chunk data)
		pFile->onDataLoaded( Params.getDataFromCursor(), Params.getDataSizeFromCursor() );
	}
}

