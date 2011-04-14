/**************************************************************************
*
* File:		CsFile.h
* Author:	Neil Richardson 
* Ver/Date:	14/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __CSFILE_H__
#define __CSFILE_H__

#include "CsTypes.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations
class CsFile;

//////////////////////////////////////////////////////////////////////////
// CsFileHeader
struct CsFileHeader
{
	BcU32					ID_;
	BcU32					NoofChunks_;
};

//////////////////////////////////////////////////////////////////////////
// CsFileChunk
struct CsFileChunk
{
	BcU32					ID_;				///!< What type of chunk it is.
	BcU32					Offset_;			///!< Offset of chunk, 0 for none.
	BcU32					Size_;				///!< Size of chunk, 0 for none.
};

//////////////////////////////////////////////////////////////////////////
// CsFileChunkNative
struct CsFileChunkNative
{
	BcU32					ID_;				///!< What type of chunk it is.
	BcU8*					pData_;				///!< Pointer to data.
	BcU32					Size_;				///!< Size of chunk, 0 for none.
};

//////////////////////////////////////////////////////////////////////////
// CsFileChunkProps
struct CsFileChunkProps
{
	enum Status
	{
		STATUS_NOT_LOADED = 0,
		STATUS_LOADING,
		STATUS_LOADED
	};
	
	BcAtomic< Status >		Status_;			///!< Status of chunk.
};

//////////////////////////////////////////////////////////////////////////
// Typedefs
typedef BcDelegate< void(*)( CsFile* ) > CsFileReadyDelegate;
typedef BcDelegate< void(*)( CsFile*, const CsFileChunk*, void* ) > CsFileChunkDelegate;

//////////////////////////////////////////////////////////////////////////
// CsFile
class CsFile
{
public:
	CsFile( const std::string& Name );
	virtual ~CsFile();
	
	/**
	 * Load file.
	 */
	virtual BcBool					load( CsFileReadyDelegate ReadyDelegate, CsFileChunkDelegate ChunkDelegate );
	
	/**
	 * Save file.
	 */
	virtual BcBool					save();
	
	/**
	 * Get name.
	 */
	virtual const std::string&		getName() const;

	/**
	 * Get chunk. Calls chunk delegate when it's loaded.
	 * @param Chunk Chunk index.
	 * @param TriggerLoad Do we want to trigger a load?
	 * @return File chunk requested.
	 */
	virtual const CsFileChunk*		getChunk( BcU32 Chunk, BcBool TriggerLoad = BcTrue );
		
	/**
	 * Get header ID.
	 */
	virtual BcU32					getID() const;
	
	/**
	 * Get number of chunks.
	 */
	virtual BcU32					getNoofChunks() const;
	
	/**
	 * Add chunk.
	 */
	virtual BcU32					addChunk( BcU32 ID, void* pData, BcU32 Size );

protected:
	std::string						Name_;

};

#endif
