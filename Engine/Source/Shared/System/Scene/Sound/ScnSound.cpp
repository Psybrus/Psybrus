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

#include "System/Scene/Sound/ScnSound.h"

#include "System/Content/CsCore.h"

#include "System/Sound/SsCore.h"

#ifdef PSY_SERVER
#include "Base/BcFile.h"
#include "Base/BcStream.h"
#include "Import/Snd/Snd.h"
#endif

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnSound::import( class CsPackageImporter& Importer, const Json::Value& Object )
{	
	const std::string& FileName = Object[ "source" ].asString();
	BcBool IsLooping = Object[ "looping" ].asInt() ? BcTrue : BcFalse;
	
	// Add root dependancy.
	Importer.addDependency( FileName.c_str() );
	
	// Load texture from file and create the data for export.
	SndSound* pSound = Snd::load( FileName.c_str() );
		
	if( pSound != NULL )
	{
		BcStream HeaderStream;
		BcStream SampleStream;
		
		// Setup header.
		ScnSoundHeader Header = 
		{
			pSound->getSampleRate(),
			pSound->getNumChannels(),
			IsLooping
		};
		
		HeaderStream << Header;
		
		// Setup sample stream.
		SampleStream.push( pSound->getData(), pSound->getDataSize() );
		
		// Add chunks.
		Importer.addChunk( BcHash( "header" ), HeaderStream.pData(), HeaderStream.dataSize() );
		Importer.addChunk( BcHash( "sample" ), SampleStream.pData(), SampleStream.dataSize() );
		
		//
		return BcTrue;
	}
	
	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSound );

void ScnSound::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pSample_",			&ScnSound::pSample_ ),
		new ReField( "pHeader_",			&ScnSound::pHeader_ ),
		new ReField( "SampleDataSize_",		&ScnSound::SampleDataSize_ ),
	};
		
	ReRegisterClass< ScnSound, Super >( Fields );
}

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
	// Create a new sample.
	if( SsCore::pImpl() != NULL )
	{
		pSample_ = SsCore::pImpl()->createSample( pHeader_->SampleRate_, pHeader_->Channels_, pHeader_->Looping_, pSampleData_, SampleDataSize_ );
	}

	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnSound::destroy()
{
	if( pSample_ != NULL )
	{
		SsCore::pImpl()->destroyResource( pSample_ );
		pSample_ = NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
// isReady
SsSample* ScnSound::getSample()
{
	return pSample_;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnSound::fileReady()
{
	// File is ready, get the header chunk.
	requestChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnSound::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	if( ChunkID == BcHash( "header" ) )
	{
		pHeader_ = (ScnSoundHeader*)pData;
		
		requestChunk( ++ChunkIdx );
	}
	else if( ChunkID == BcHash( "sample" ) )
	{
		pSampleData_ = pData;
		SampleDataSize_ = getChunkSize( ChunkIdx );
		
		markCreate();
	}
}

