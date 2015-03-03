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

#ifdef PSY_IMPORT_PIPELINE
#include "System/Scene/Import/ScnSoundImport.h"
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
REFLECTION_DEFINE_DERIVED( ScnSound );

void ScnSound::StaticRegisterClass()
{
	ReField* Fields[] = 
	{
		new ReField( "pFileData_", &ScnSound::pFileData_, bcRFF_SHALLOW_COPY | bcRFF_CHUNK_DATA ),
		new ReField( "pSource_", &ScnSound::pSource_, bcRFF_TRANSIENT ),
	};
		
	auto& Class = ReRegisterClass< ScnSound, Super >( Fields );
	BcUnusedVar( Class );

#ifdef PSY_IMPORT_PIPELINE
	// Add importer attribute to class for resource system to use.
	Class.addAttribute( new CsResourceImporterAttribute( 
		ScnSoundImport::StaticGetClass(), 1 ) );
#endif
}

//////////////////////////////////////////////////////////////////////////
// Ctor
ScnSound::ScnSound():
	pSource_( nullptr )
{
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
ScnSound::~ScnSound()
{
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnSound::create()
{	
	if( SsCore::pImpl() != nullptr )
	{
		// Create a new sample.
		pSource_ = SsCore::pImpl()->createSource( SsSourceParams(), pFileData_ );
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
	if( ChunkID == BcHash( "filedata" ) )
	{
		pFileData_ = (SsSourceFileData*)pData;
		markCreate();
	}
}

