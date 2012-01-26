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

#include "CsFileWriter.h"

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
		// Write header.
		Header_.NoofChunks_ = Chunks_.size();
		File_.write( &Header_, sizeof( Header_ ) );
		
		// Write chunks.
		BcU32 Offset = sizeof( CsFileHeader ) + ( sizeof( CsFileChunk ) * Chunks_.size() );
		
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
// CsFileWriter
BcU32 CsFileWriter::addChunk( BcU32 ID, void* pData, BcU32 Size )
{
	BcAssert( Size > 0 );
	CsFileChunkNative Chunk = { ID, new BcU8[ Size ], Size };
	BcMemCopy( Chunk.pData_, pData, Size );
	Chunks_.push_back( Chunk );
	return Chunks_.size() - 1;
}
