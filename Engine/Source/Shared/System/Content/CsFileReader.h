/**************************************************************************
*
* File:		CsFile.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Resource file
*		
*
*
* 
**************************************************************************/

#ifndef __CSFILEREADER_H__
#define __CSFILEREADER_H__

#include "System/Content/CsFile.h"

//////////////////////////////////////////////////////////////////////////
// CsFileReader
class CsFileReader:
	public CsFile
{
public:
	CsFileReader( const std::string& Name );
	virtual ~CsFileReader();
	
	/**
	 * Load resource file.
 	 * @param ReadyDelegate Called when file is ready to be queried.
 	 * @param ChunkDelegate Called when data for a chunk is loaded.
	 */
	BcBool					load( CsFileReadyDelegate ReadyDelegate, CsFileChunkDelegate ChunkDelegate );

	/**
	 * Get string.
	 * @param Offset Offset of string in table.
	 * @return String, or NULL pointer if invalid offset.
	 */
	const BcChar*			getString( BcU32 Offset );
	
	/**
	 * Get chunk. Calls chunk delegate when it's loaded.
	 * @param Chunk Chunk index.
	 * @param TriggerLoad Do we want to trigger a load?
	 * @return File chunk requested.
	 */
	const CsFileChunk*		getChunk( BcU32 Chunk, BcBool TriggerLoad = BcTrue );
	
	/**
	 * Get header ID.
	 */
	BcU32					getID() const;

	/**
	 * Get number of chunks.
	 */
	BcU32					getNoofChunks() const;
		
private:
	void					onHeaderLoaded( void* pData, BcSize Size );
	void					onStringTableLoaded( void* pData, BcSize Size );
	void					onChunksLoaded( void* pData, BcSize Size );
	void					onDataLoaded( void* pData, BcSize Size );
	
private:
	FsFile					File_;
	CsFileHeader			Header_;
	BcChar*					pStringTable_;
	CsFileChunk*			pChunks_;
	CsFileChunkProps*		pChunkProps_;
	BcU8*					pData_;
	CsFileReadyDelegate		ReadyDelegate_;
	CsFileChunkDelegate		ChunkDelegate_;
};

#endif

