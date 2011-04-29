/**************************************************************************
*
* File:		SndFileWAV.h
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SNDFILEWAV_H__
#define __SNDFILEWAV_H__

#include "BcFile.h"
#include "BcDebug.h"

#include "Snd.h"

//////////////////////////////////////////////////////////////////////////
// SndFileWAV
class SndFileWAV:
	public SndSound
{
private:
	struct Chunk
	{
		BcChar ChunkID_[4];
		BcS32 ChunkSize_;
	};
	
	struct HeaderChunk: Chunk
	{
		BcChar RIFFType_[4]; 
	};
	
	struct FmtChunk: Chunk
	{
		BcU16 FormatTag_;
		BcU16 Channels_;
		BcU32 SamplesPerSec_;
		BcU32 AvgBytesPerSec_;
		BcU16 BlockAlign_;
		BcU16 BitsPerSample_;
	};
	
	struct DataChunk: Chunk
	{

	};
	
	HeaderChunk*		Header_;
	FmtChunk*			Format_;
	DataChunk*			Data_;
	
	BcU8*				pFileData_;
	BcU32				FileDataSize_;
		
public:
	SndFileWAV();
	virtual ~SndFileWAV();
	
	BcU32				getSampleRate() const;
	BcU32				getNumChannels() const;
	void*				getData() const;
	BcU32				getDataSize() const;
	
	BcBool				load( const BcChar* pFileName );

private:
	Chunk*				findChunk( const BcChar* ChunkName );
	HeaderChunk*		findHeaderChunk( const BcChar* ChunkName, const BcChar* HeaderName );
	static BcBool		chunkNameTest( Chunk* ChunkData, const BcChar* ChunkName );
	static BcBool		chunkHeaderNameTest( HeaderChunk* ChunkData, const BcChar* ChunkName );
};

#endif
