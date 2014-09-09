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
#include "System/Sound/SsSource.h"

#ifdef PSY_SERVER
#include "System/Scene/Import/ScnSoundImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnSound );

void ScnSound::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pHeader_", &ScnSound::pHeader_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
		new ReField( "pSource_", &ScnSound::pSource_, bcRFF_TRANSIENT ),
		new ReField( "pSampleData_", &ScnSound::pSampleData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
		new ReField( "SampleDataSize_", &ScnSound::SampleDataSize_ ),
	};
		
	auto& Class = ReRegisterClass< ScnSound, Super >( Fields );
#ifdef PSY_SERVER
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnSoundImport::StaticGetClass(), 0 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnSound::initialise()
{
	pSource_ = NULL;
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnSound::create()
{	
	if( SsCore::pImpl() != nullptr )
	{
		// Create a new sample.
		pSource_ = SsCore::pImpl()->createSource( SsSourceParams() );
	}

	markReady();
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnSound::destroy()
{
	if( SsCore::pImpl() != nullptr )
	{
		if( pSource_ != nullptr )
		{
			SsCore::pImpl()->destroyResource( pSource_ );
			pSource_ = nullptr;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getSource
class SsSource* ScnSound::getSource()
{
	return pSource_;
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

