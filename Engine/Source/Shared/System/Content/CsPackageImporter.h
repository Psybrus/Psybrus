/**************************************************************************
*
* File:		CsPackageImporter.h
* Author:	Neil Richardson
* Ver/Date:	8/04/12
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __CSPACKAGEIMPORTER_H__
#define __CSPACKAGEIMPORTER_H__

#include "Base/BcFile.h"

#include "System/Content/CsPackageFileData.h"
#include "System/Content/CsResource.h"

#ifdef PSY_SERVER
#include <json/json.h>
#endif

//////////////////////////////////////////////////////////////////////////
// CsPackageImporter
class CsPackageImporter
{
private:
	CsPackageImporter( const CsPackageImporter& ){}

public:
	CsPackageImporter();
	~CsPackageImporter();

	BcBool							import( const BcName& Name );
	BcBool							save( const BcPath& Path );
	
	BcBool							loadJsonFile( const BcChar* pFileName, Json::Value& Root );
	BcBool							importResource( const Json::Value& Resource );
	
	void							addImport( const Json::Value& Resource );
	BcU32							addString( const BcChar* pString );
	BcU32							addChunk( BcU32 ID, void* pData, BcU32 Size, BcU32 RequiredAlignment = 16, BcU32 Flags = csPCF_DEFAULT );
	void							addDependency( const BcChar* pFileName );	
	
private:
	typedef std::vector< std::string > TStringList;
	typedef TStringList::iterator TStringListIterator;

	typedef std::vector< CsPackageResourceHeader > CsPackageResourceHeaderList;
	typedef CsPackageResourceHeaderList::iterator CsPackageResourceHeaderIterator;

	typedef std::vector< CsPackageChunkHeader > CsPackageChunkHeaderList;
	typedef CsPackageChunkHeaderList::iterator CsPackageChunkHeaderIterator;

	typedef std::vector< CsPackageChunkData > CsPackageChunkDataList;
	typedef CsPackageChunkDataList::iterator CsPackageChunkDataIterator;
	
	typedef std::list< Json::Value > TJsonValueList;
	typedef TJsonValueList::iterator TJsonValueIterator;

	TJsonValueList					JsonResources_;

	CsPackageResourceHeader			CurrResourceHeader_;

	BcU32							DataPosition_;
	BcFile							File_;
	CsPackageHeader					Header_;
	TStringList						StringList_;
	CsPackageResourceHeaderList		ResourceHeaders_;
	CsPackageChunkHeaderList		ChunkHeaders_;
	CsPackageChunkDataList			ChunkDatas_;
};

#endif
