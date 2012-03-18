/**************************************************************************
*
* File:		CsFileWriter.cpp
* Author:	Neil Richardson 
* Ver/Date:	12/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Content/CsFileWriter.h"

#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
CsFileWriter::CsFileWriter( const std::string& Name ):
	CsFile( Name )
{

}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsFileWriter::~CsFileWriter()
{
	CsFileChunkNativeListIterator Iter = Chunks_.begin();
	
	while( Iter != Chunks_.end() )
	{
		CsFileChunkNative Chunk = (*Iter);
		
		delete [] Chunk.pData_;
		
		// Next
		++Iter;
	}
}

//////////////////////////////////////////////////////////////////////////
// CsFileWriter
BcBool CsFileWriter::save()
{
	const BcChar* pType = BcStrStr( Name_.c_str(), "." );
	Header_.ID_ = BcHash( pType );
	
	if( File_.open( Name_.c_str(), bcFM_WRITE ) )
	{
		// Generate string table.
		BcStream StringTableStream;

		for( BcU32 Idx = 0; Idx < StringList_.size(); ++Idx )
		{
			const std::string& StringEntry( StringList_[ Idx ] );
			StringTableStream.push( StringEntry.c_str(), StringEntry.size() + 1 );
		}	

		// Write header.
		Header_.NoofChunks_ = Chunks_.size();
		Header_.StringTableSize_ = StringTableStream.dataSize();
		File_.write( &Header_, sizeof( Header_ ) );

		// Write string table.
		File_.write( StringTableStream.pData(), StringTableStream.dataSize() );
		
		// Write chunks.
		BcU32 Offset = sizeof( CsFileHeader ) + StringTableStream.dataSize() + ( sizeof( CsFileChunk ) * Chunks_.size() );
		{
			CsFileChunkNativeListIterator Iter = Chunks_.begin();
			
			while( Iter != Chunks_.end() )
			{
				CsFileChunkNative Chunk = (*Iter);
				CsFileChunk FileChunk;
				
				FileChunk.ID_ = Chunk.ID_;
				FileChunk.Offset_ = Offset;
				FileChunk.Size_ = Chunk.Size_;
				FileChunk.Hash_ = (BcU32)BcHash( Chunk.pData_, Chunk.Size_ );
				
				Offset += Chunk.Size_;
				
				File_.write( &FileChunk, sizeof( FileChunk ) );
				
				// Next
				++Iter;
			}
		}
		
		// Write data.
		{
			CsFileChunkNativeListIterator Iter = Chunks_.begin();
			
			while( Iter != Chunks_.end() )
			{
				CsFileChunkNative Chunk = (*Iter);
				
				File_.write( Chunk.pData_, Chunk.Size_ );
				
				// Next
				++Iter;
			}
		}
		
		// Done and done.
		File_.close();
		
		return BcTrue;
	}

	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// addString
BcU32 CsFileWriter::addString( const BcChar* pString )
{
	BcU32 CurrentOffset = 0;

	for( BcU32 Idx = 0; Idx < StringList_.size(); ++Idx )
	{
		const std::string& StringEntry( StringList_[ Idx ] );

		if( StringEntry == pString )
		{
			return CurrentOffset;
		}

		// String length with null terminator.
		CurrentOffset += StringEntry.length() + 1;
	}

	// Add string to list.
	StringList_.push_back( pString );

	// Return current offset.
	return CurrentOffset;
}

//////////////////////////////////////////////////////////////////////////
// addChunk
BcU32 CsFileWriter::addChunk( BcU32 ID, void* pData, BcU32 Size )
{
	BcAssert( Size > 0 );
	CsFileChunkNative Chunk = { ID, new BcU8[ Size ], Size };
	BcMemCopy( Chunk.pData_, pData, Size );
	Chunks_.push_back( Chunk );
	return Chunks_.size() - 1;
}
