/**************************************************************************
*
* File:		ScnSound.cpp
* Author:	Neil Richardson 
* Ver/Date:	28/04/11
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnSound.h"

#include "CsCore.h"

#include "SsCore.h"

#ifdef PSY_SERVER
#include "BcFile.h"
#include "BcStream.h"
#include "json.h"
#include "Snd.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnSound::import( const Json::Value& Object, CsDependancyList& DependancyList )
{	
	const std::string& FileName = Object[ "source" ].asString();
	
	// Add root dependancy.
	DependancyList.push_back( FileName );
	
	// Load texture from file and create the data for export.
	SndSound* pSound = Snd::load( FileName.c_str() );
		
	if( pSound != NULL )
	{
		BcStream HeaderStream;
		BcStream SampleStream;
		
		// Setup header.
		THeader Header = 
		{
			pSound->getSampleRate(),
			pSound->getNumChannels(),
			BcFalse
		};
		
		HeaderStream << Header;
		
		// Setup sample stream.
		SampleStream.push( pSound->getData(), pSound->getDataSize() );
		
		// Add chunks.
		pFile_->addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		pFile_->addChunk( BcHash( "sample" ), SampleStream.pData(), SampleStream.dataSize() );
		
		//
		return BcTrue;
	}
	
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSound );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSound::initialise()
{
	pSample_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnSound::create()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnSound::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnSound::isReady()
{
	// TODO: LOCK!!
	return pSample_ != NULL;
}

//////////////////////////////////////////////////////////////////////////
// isReady
SsSample* ScnSound::getSample()
{
	return pSample_;
}

//////////////////////////////////////////////////////////////////////////
// setup
void ScnSound::setup()
{
	// Create a new sample.
	if( SsCore::pImpl() != NULL )
	{
		pSample_ = SsCore::pImpl()->createSample( pHeader_->SampleRate_, pHeader_->Channels_, pHeader_->Looping_, pSampleData_, SampleDataSize_ );
	}
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnSound::fileReady()
{
	// File is ready, get the header chunk.
	pFile_->getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnSound::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		pHeader_ = (THeader*)pData;
		
		pFile_->getChunk( ++ChunkIdx );
	}
	else if( pChunk->ID_ == BcHash( "sample" ) )
	{
		pSampleData_ = pData;
		SampleDataSize_ = pChunk->Size_;
		
		setup();
	}
}

