/**************************************************************************
*
* File:		CsResource.cpp
* Author:	Neil Richardson 
* Ver/Date:	7/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "CsResource.h"
 
#include "CsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define CsResource
DEFINE_CSRESOURCE;

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResource::CsResource( const std::string& Name, CsFile* pFile ):
	pFile_( pFile ),
	Name_( Name ),
	Stage_( csRS_PENDING_CREATE )
{
	// TODO: Delegates should be bound to file here, and CsResource have
	//       virtuals to pass the data through :)
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsResource::~CsResource()
{
	delete pFile_;
}

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool CsResource::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	BcUnusedVar( Object );

	return BcFalse;
}
#endif

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void CsResource::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void CsResource::create()
{

}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void CsResource::destroy()
{

}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void CsResource::fileReady()
{
	
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
//virtual
void CsResource::fileChunkReady( BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	
}

//////////////////////////////////////////////////////////////////////////
// acquire
void CsResource::acquire()
{
	++RefCount_;
}

//////////////////////////////////////////////////////////////////////////
// release
void CsResource::release()
{
	if( ( --RefCount_ ) == 0 )
	{
		// Call into CsCore to destroy this resource.
		if( CsCore::pImpl() != NULL )
		{
			CsCore::pImpl()->destroyResource( this );
		}
		else
		{
			BcPrintf( "CsResource::release: %s.%s exists when CsCore is no longer around. Static or global reference held?\n", getName().c_str(), getTypeString().c_str() );
			
			// Only doing this so references held by this are cleaned up, and other resources are reported too.
			delete this;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getName
const std::string& CsResource::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// getChunk
void CsResource::getChunk( BcU32 Chunk, BcBool TriggerLoad )
{
	acquire(); // Prevent object being released until callback.b
	pFile_->getChunk( Chunk, TriggerLoad );
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsResource::getNoofChunks() const
{
	return pFile_->getNoofChunks();
}

//////////////////////////////////////////////////////////////////////////
// process
eCsResourceStage CsResource::process()
{
	// Call create/destroy depending on the stage we are in.
	switch( Stage_ )
	{
		case csRS_PENDING_CREATE:
			// Do nothing, this is up to parent resource.
			break;
		case csRS_CREATE:
			Stage_ = csRS_CREATE;
			create();
			break;
		case csRS_LOADING:
			if( isReady() == BcTrue )
			{
				Stage_ = csRS_LOADED;
			}
			break;
		case csRS_LOADED:
			// Shouldn't hit here.
			break;
		case csRS_DESTROY:
			Stage_ = csRS_KILL;
			destroy();
			break;
	}
	
	return Stage_;
}

//////////////////////////////////////////////////////////////////////////
// delegateFileReady
void CsResource::delegateFileReady( CsFile* pFile )
{
	BcScopedLock< BcAtomicMutex > Lock( Lock_ );
	fileReady();
	release();
}

//////////////////////////////////////////////////////////////////////////
// delegateFileChunkReady
void CsResource::delegateFileChunkReady( CsFile* pFile, BcU32 ChunkIdx, const CsFileChunk* pChunk, void* pData )
{
	BcScopedLock< BcAtomicMutex > Lock( Lock_ );
	fileChunkReady( ChunkIdx, pChunk, pData );
	release();
}
