/**************************************************************************
*
* File:		CsCoreServer.cpp
* Author:	Neil Richardson 
* Ver/Date:	9/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Content/CsCoreServer.h"

#include "Base/BcHash.h"
#include "System/Remote/RmCore.h"
#include "System/File/FsCore.h"

SYS_CREATOR( CsCoreServer );

#define DbgTrace BcPrintf
//#define DbgTrace(...)

//////////////////////////////////////////////////////////////////////////
// Ctor
CsCoreServer::CsCoreServer()
{
	
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsCoreServer::~CsCoreServer()
{
	
}

//////////////////////////////////////////////////////////////////////////
// open
//virtual
void CsCoreServer::open()
{
	BcAssert( RmCore::pImpl() != NULL );
	
	// Register delegates.
	RmCore::pImpl()->registerDelegate( BcHash( "CsCore::delegateLoad" ),		RmDelegate::bind< CsCoreServer, &CsCoreServer::delegateLoad >( this ) );
	RmCore::pImpl()->registerDelegate( BcHash( "CsCore::delegateGetChunk" ),	RmDelegate::bind< CsCoreServer, &CsCoreServer::delegateGetChunk >( this ) );
	RmCore::pImpl()->registerDelegate( BcHash( "CsCore::delegateUnload" ),		RmDelegate::bind< CsCoreServer, &CsCoreServer::delegateUnload >( this ) );

	CsCore::open();
}

//////////////////////////////////////////////////////////////////////////
// update
//virtual
void CsCoreServer::update()
{
	BcAssert( RmCore::pImpl() != NULL );

	CsCore::update();
}

//////////////////////////////////////////////////////////////////////////
// close
//virtual
void CsCoreServer::close()
{
	BcAssert( RmCore::pImpl() != NULL );
	
	// Unregister delegates.
	RmCore::pImpl()->unregisterDelegate( BcHash( "CsCore::delegateLoad" ) );	
	RmCore::pImpl()->unregisterDelegate( BcHash( "CsCore::delegateGetChunk" ) );
	RmCore::pImpl()->unregisterDelegate( BcHash( "CsCore::delegateUnload" ) );

	CsCore::close();
}

//////////////////////////////////////////////////////////////////////////
// delegateLoad
void CsCoreServer::delegateLoad( void* pData, BcU32 Bytes )
{
	// Get params.
	RmParameters Params( RmParameters::TYPE_RECV, pData, Bytes );

	// Get file name.
	std::string Name;
	Params >> Name;
	
	// Create a file reader and put in the map.
	// NOTE: Perhaps this should actually trigger the resource to be created and
	//       and loaded to, so we can setup a link?
	CsFile* pFile = createFileReader( Name );
	FileMap_[ Name ] = pFile;
	
	// Add file for monitoring.
	FsCore::pImpl()->addFileMonitor( Name.c_str() );

	DbgTrace( "CsCoreServer::delegateLoad: %s\n", Name.c_str() );

	// Perform the load.
	pFile->load( CsFileReadyDelegate::bind< CsCoreServer, &CsCoreServer::delegateFileReady >( this ),
				 CsFileChunkDelegate::bind< CsCoreServer, &CsCoreServer::delegateFileChunkReady >( this ) );
}

//////////////////////////////////////////////////////////////////////////
// delegateGetChunk
void CsCoreServer::delegateGetChunk( void* pData, BcU32 Bytes )
{
	// Get params.
	RmParameters Params( RmParameters::TYPE_RECV, pData, Bytes );
	
	// Get file name.
	std::string Name;
	Params >> Name;
	
	// Lookup and bridge.
	TFileMapIterator Iter = FileMap_.find( Name );
	if( Iter != FileMap_.end() )
	{
		CsFile* pFile = (*Iter).second;
	
		// Get chunk.
		BcU32 ChunkIdx;
		Params >> ChunkIdx;
		
		DbgTrace( "CsCoreServer::delegateGetChunk: %s, %u\n", Name.c_str(), ChunkIdx );
		
		// Get chunk.
		pFile->getChunk( ChunkIdx, BcTrue );
	}
}

//////////////////////////////////////////////////////////////////////////
// delegateUnload
void CsCoreServer::delegateUnload( void* pData, BcU32 Bytes )
{
	// Get params.
	RmParameters Params( RmParameters::TYPE_RECV, pData, Bytes );
	
	// Get file name.
	std::string Name;
	Params >> Name;
	
	// Lookup and bridge.
	TFileMapIterator Iter = FileMap_.find( Name );
	if( Iter != FileMap_.end() )
	{
		CsFile* pFile = (*Iter).second;
		
		// Remove from maps.
		FileMap_.erase( Iter );
				
		// Delete file, no longer needed.
		delete pFile;

		DbgTrace( "CsCoreServer::delegateUnload: %s, %u\n", Name.c_str() );
	}
}

//////////////////////////////////////////////////////////////////////////
// delegateFileReady
void CsCoreServer::delegateFileReady( CsFile* pFile )
{
	// Setup parameters.
	RmParameters Params( RmParameters::TYPE_SEND );
	Params << pFile->getName();
	
	// Add header.
	CsFileHeader Header =
	{
		pFile->getID(),
		pFile->getNoofChunks(),
	};
	
	Params << Header;
	
	// Setup chunks.
	for( BcU32 iChunk = 0; iChunk < pFile->getNoofChunks(); ++iChunk )
	{
		const CsFileChunk* pChunk = pFile->getChunk( iChunk, BcFalse );
		Params << *pChunk;
	}
	
	// Do the RPC.
	RmCore::pImpl()->send( BcHash( "CsCore::delegateFileReady" ), Params );	
}

//////////////////////////////////////////////////////////////////////////
// delegateFileChunkReady
void CsCoreServer::delegateFileChunkReady( CsFile* pFile, BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	// Setup parameters.
	RmParameters Params( RmParameters::TYPE_SEND );
	Params << pFile->getName();
	
	// Add chunk index.
	Params << ChunkIdx;
	
	// Add data.
	Params.push( pData, pChunk->Size_ );
	
	// Do the RPC.	
	RmCore::pImpl()->send( BcHash( "CsCore::delegateFileChunkReady" ), Params );
}

