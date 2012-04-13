/**************************************************************************
*
* File:		CsPackageLoader.h
* Author:	Neil Richardson
* Ver/Date:	8/04/12
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __CSPACKAGELOADER_H__
#define __CSPACKAGELOADER_H__

#include "System/Content/CsPackageFileData.h"
#include "System/Content/CsResource.h"

//////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class CsPackageLoader;

//////////////////////////////////////////////////////////////////////////
// CsPackageLoader
class CsPackageLoader
{
private:
	CsPackageLoader( const CsPackageLoader& ){}

public:
	CsPackageLoader( CsPackage* pPackage, const BcPath& Path );
	~CsPackageLoader();

	BcBool							hasPendingCallback() const;
	BcBool							isDataReady() const;
	const BcChar*					getSourceFile() const;
	const BcChar*					getString( BcU32 Offset ) const;
	BcU32							getChunkSize( BcU32 ResourceIdx, BcU32 ResourceChunkIdx );
	BcU32							getNoofChunks( BcU32 ResourceIdx );
	BcBool							requestChunk( BcU32 ResourceIdx, BcU32 ResourceChunkIdx, void* pDataLocation = NULL );

private:
	void							onHeaderLoaded( void* pData, BcSize Size );
	void							onStringTableLoaded( void* pData, BcSize Size );
	void							onResourceHeadersLoaded( void* pData, BcSize Size );
	void							onChunkHeadersLoaded( void* pData, BcSize Size );
	void							onDataLoaded( void* pData, BcSize Size );

private:
	void							markupResources();
	void							initialiseResources();

	BcBool							findResourceChunk( void* pData, BcU32& ResourceIdx, BcU32& ChunkIdx );
	void							processResourceChunk( BcU32 ResourceIdx, BcU32 ChunkIdx );
	
private:
	CsPackage*						pPackage_;
	BcU32							DataPosition_;
	FsFile							File_;
	CsPackageHeader					Header_;
	void*							pPackageData_;
	BcChar*							pStringTable_;
	CsPackageResourceHeader*		pResourceHeaders_;
	CsPackageChunkHeader*			pChunkHeaders_;
	CsPackageChunkData*				pChunkData_;
	BcBool							IsStringTableReady_;
	BcBool							IsDataReady_;
	BcAtomicU32						PendingCallbackCount_;
	
	std::vector< CsResourceRef<> >	Resources_;
};

#endif