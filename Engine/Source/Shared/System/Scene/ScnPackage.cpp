/**************************************************************************
*
* File:		ScnPackage.cpp
* Author:	Neil Richardson 
* Ver/Date:	5/03/11	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "ScnPackage.h"
#include "CsCore.h"

#ifdef PSY_SERVER
#include "BcStream.h"
#endif

#ifdef PSY_SERVER

//////////////////////////////////////////////////////////////////////////
// import
//virtual
BcBool ScnPackage::import( const Json::Value& Object, CsDependancyList& DependancyList )
{
	const Json::Value& ImportResources = Object[ "resources" ];
	
	std::vector< CsResourceRef<> > Resources;
	CsResourceRef<> ResourceRef;
	for( BcU32 Idx = 0; Idx < ImportResources.size(); ++Idx )
	{
		const Json::Value& Resource = ImportResources[ Idx ];
		
		if( CsCore::pImpl()->importObject( Resource, ResourceRef, DependancyList ) )
		{
			Resources.push_back( ResourceRef );
		}
	}
	return BcTrue;
}
#endif

//////////////////////////////////////////////////////////////////////////
// Define resource internals.
DEFINE_RESOURCE( ScnPackage );

//////////////////////////////////////////////////////////////////////////
// initialise
//virtual
void ScnPackage::initialise()
{
	
}

//////////////////////////////////////////////////////////////////////////
// create
//virtual
void ScnPackage::create()
{
	
}

//////////////////////////////////////////////////////////////////////////
// destroy
//virtual
void ScnPackage::destroy()
{
	
}

//////////////////////////////////////////////////////////////////////////
// isReady
//virtual
BcBool ScnPackage::isReady()
{
	return BcFalse;
}

//////////////////////////////////////////////////////////////////////////
// fileReady
void ScnPackage::fileReady()
{
	// File is ready, get the header chunk.
	pFile_->getChunk( 0 );
}

//////////////////////////////////////////////////////////////////////////
// fileChunkReady
void ScnPackage::fileChunkReady( const CsFileChunk* pChunk, void* pData )
{
	if( pChunk->ID_ == BcHash( "header" ) )
	{
		// TODO: Compile the script.
	}
}

