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

#include "System/Content/CsResource.h"
 
#include "System/Content/CsCore.h"

//////////////////////////////////////////////////////////////////////////
// Define CsResource
DEFINE_CSRESOURCE;

//////////////////////////////////////////////////////////////////////////
// StaticPropertyTable
void CsResource::StaticPropertyTable( CsPropertyTable& PropertyTable )
{
	PropertyTable.beginCatagory( "CsResource" )
		.field( "name",					csPVT_STRING,		csPCT_VALUE )
	.endCatagory();
}

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResource::CsResource( const BcName& Name, CsFile* pFile ):
	pFile_( pFile ),
	Name_( Name )
{

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
BcBool CsResource::isReady()
{
	return BcTrue;
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
			BcPrintf( "CsResource::release: %s.%s exists when CsCore is no longer around. Static or global reference held?\n", (*getName()).c_str(), (*getType()).c_str() );
			
			// Only doing this so references held by this are cleaned up, and other resources are reported too.
			delete this;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& CsResource::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// getString
const BcChar* CsResource::getString( BcU32 Offset )
{
	return pFile_->getString( Offset );
}

//////////////////////////////////////////////////////////////////////////
// getChunk
void CsResource::getChunk( BcU32 Chunk, BcBool TriggerLoad )
{
	if( TriggerLoad == BcTrue )
	{
		acquire(); // Prevent object being released until callback.
	}

	pFile_->getChunk( Chunk, TriggerLoad );
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsResource::getNoofChunks() const
{
	return pFile_->getNoofChunks();
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
