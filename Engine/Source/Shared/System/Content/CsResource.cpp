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
#include "System/Content/CsPackage.h"

//////////////////////////////////////////////////////////////////////////
// Define CsResource
DEFINE_CSRESOURCE;

//////////////////////////////////////////////////////////////////////////
// Ctor
CsResource::CsResource( const BcName& Name, BcU32 Index, CsPackage* pPackage ):
	Name_( Name ),
	Index_( Index ),
	pPackage_( pPackage )
{
	BcAssertMsg( Name != BcName::INVALID, "Resource can not have an invalid name." );
	BcAssertMsg( Name != BcName::NONE, "Resource can not have a none name." );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
CsResource::~CsResource()
{
	
}

#ifdef PSY_SERVER
//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool CsResource::import( class CsPackageImporter& Importer, const Json::Value& Object )
{
	BcUnusedVar( Importer );
	BcUnusedVar( Object );

	// TODO: Generic property save out?

	return BcTrue;
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
void CsResource::fileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	BcBreakpoint;
}

//////////////////////////////////////////////////////////////////////////
// acquire
void CsResource::acquire()
{
	++RefCount_;

	/*
	if(Name_ == BcName("LevelEntity"))
	{
		int a = 0; ++a;
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
// release
void CsResource::release()
{
	//BcPrintf("release: %s, %u\n", (*Name_).c_str(), RefCount_);
	if( ( --RefCount_ ) == 0 )
	{
		// Call into CsCore to destroy this resource.
		if( CsCore::pImpl() != NULL )
		{
			// Detach package.
			Index_ = BcErrorCode;
			pPackage_ = NULL;

			// Destroy.
			CsCore::pImpl()->destroyResource( this );
		}
		else
		{
			// Only doing this so references held by this are cleaned up, and other resources are reported too.
			delete this;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// refCount
BcU32 CsResource::refCount() const
{
	BcAssert( BcIsGameThread() );
	return RefCount_;
}

//////////////////////////////////////////////////////////////////////////
// getName
CsPackage* CsResource::getPackage()
{
	return pPackage_;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& CsResource::getPackageName() const
{
	return pPackage_ != NULL ? pPackage_->getName() : BcName::INVALID;
}

//////////////////////////////////////////////////////////////////////////
// getName
const BcName& CsResource::getName() const
{
	return Name_;
}

//////////////////////////////////////////////////////////////////////////
// getIndex
BcU32 CsResource::getIndex() const
{
	return Index_;
}

//////////////////////////////////////////////////////////////////////////
// getString
const BcChar* CsResource::getString( BcU32 Offset )
{
	return pPackage_->getString( Offset );
}

//////////////////////////////////////////////////////////////////////////
// getChunk
void CsResource::requestChunk( BcU32 Chunk, void* pDataLocation )
{
	acquire();
	if( !pPackage_->requestChunk( Index_, Chunk, pDataLocation ) )
	{
		// There will be no callback.
		release();
	}
}

//////////////////////////////////////////////////////////////////////////
// getChunkSize
BcU32 CsResource::getChunkSize( BcU32 Chunk )
{
	return pPackage_->getChunkSize( Index_, Chunk );
}

//////////////////////////////////////////////////////////////////////////
// getNoofChunks
BcU32 CsResource::getNoofChunks() const
{
	return pPackage_->getNoofChunks( Index_ );
}

//////////////////////////////////////////////////////////////////////////
// onFileReady
void CsResource::onFileReady()
{
	fileReady();
	release();
}

//////////////////////////////////////////////////////////////////////////
// onFileChunkReady
void CsResource::onFileChunkReady( BcU32 ChunkIdx, BcU32 ChunkID, void* pData )
{
	fileChunkReady( ChunkIdx, ChunkID, pData );
	release();
}
