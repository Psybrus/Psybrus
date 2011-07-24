// ============================================================================
//
// Copyright © 2010 Alice Blunt & Neil Richardson.
//
// This file is part of ExcaliburEx.
//
// ExcaliburEx is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ExcaliburEx is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with ExcaliburEx.  If not, see <http://www.gnu.org/licenses/>.
//
// ============================================================================

#ifndef __EX_SSFILEWAV_H__
#define __EX_SSFILEWAV_H__

#include "exSsFile.h"

// ============================================================================
// exSsFileWAV
class exSsFileWAV: public exSsFile
{
private:
	struct Chunk
	{
		exChar ChunkID_[4];
		exS32 ChunkSize_;
	};
	
	struct HeaderChunk: Chunk
	{
		exChar RIFFType_[4]; 
	};
	
	struct FmtChunk: Chunk
	{
		exU16 FormatTag_;
		exU16 Channels_;
		exU32 SamplesPerSec_;
		exU32 AvgBytesPerSec_;
		exU16 BlockAlign_;
		exU16 BitsPerSample_;
	};
	
	struct DataChunk: Chunk
	{

	};
	
	HeaderChunk*		Header_;
	FmtChunk*			Format_;
	DataChunk*			Data_;
	
	exFileTicket		FileTicket_;
	exSsFileDelegate	FileDelegate_;
	volatile exBool		IsLoaded_;
	volatile exBool		LoadCallbackDone_;
	
public:
	exSsFileWAV( exFileTicket FileTicket, exSsFileDelegate FileDelegate );
	virtual ~exSsFileWAV();
	
	exBool				IsLoaded() const;
	exSsDataFormat		DataFormat() const;
	exU32				NumChannels() const;
	exU32				SampleRate() const;
	exBool				DecodeNextBuffer( void*& Data, exU32& DataSize );
	
private:
	void				OnLoadCallback( exFileTicket FileTicket, void* UserData );

	Chunk*				FindChunk( exConstChar* ChunkName );
	HeaderChunk*		FindHeaderChunk( exConstChar* ChunkName, exConstChar* HeaderName );
	static exBool		ChunkNameTest( Chunk* ChunkData, exConstChar* ChunkName );
	static exBool		ChunkHeaderNameTest( HeaderChunk* ChunkData, exConstChar* ChunkName );

};


#endif
