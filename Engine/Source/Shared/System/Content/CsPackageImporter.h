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

//////////////////////////////////////////////////////////////////////////
// Enable threaded importing.
#define THREADED_IMPORTING ( 0 )

//////////////////////////////////////////////////////////////////////////
// CsPackageDependencies
struct CsPackageDependencies
{
	REFLECTION_DECLARE_BASIC( CsPackageDependencies );

	CsPackageDependencies(){};

	typedef std::set< CsDependency > TDependencyList;
	
	TDependencyList Dependencies_;
};

//////////////////////////////////////////////////////////////////////////
// CsPackageImporter
class CsPackageImporter
{
private:
	CsPackageImporter( const CsPackageImporter& ){}

public:
	CsPackageImporter();
	~CsPackageImporter();

	BcBool import( const BcName& Name );
	BcBool save( const BcPath& Path );
	
	BcBool loadJsonFile( const BcChar* pFileName, Json::Value& Root );
	BcBool importResource( const Json::Value& Resource );
	
	void beginImport();
	void endImport();

	BcU32 addImport( const Json::Value& Resource, BcBool IsCrossRef = BcTrue );
	BcU32 addString( const BcChar* pString );
	BcU32 addPackageCrossRef( const BcChar* pFullName );
	BcU32 addChunk( BcU32 ID, const void* pData, BcSize Size, BcSize RequiredAlignment = 16, BcU32 Flags = csPCF_DEFAULT );
	void addDependency( const BcChar* pFileName );
	void addAllPackageCrossRefs( Json::Value& Root );

private:
	BcBool havePackageDependency( const BcName& PackageName );

private:
	BcBool importResource_worker( Json::Value ResourceObject );

private:
	typedef std::vector< std::string > TStringList;
	typedef TStringList::iterator TStringListIterator;

	typedef std::vector< CsPackageCrossRefData > TPackageCrossRefList;
	typedef TPackageCrossRefList::iterator TPackageCrossRefIterator;

	typedef std::vector< CsPackageDependencyData > TPackageDependencyDataList;
	typedef TPackageDependencyDataList::iterator TPackageDependencyDataIterator;
	
	typedef std::vector< CsPackageResourceHeader > CsPackageResourceHeaderList;
	typedef CsPackageResourceHeaderList::iterator CsPackageResourceHeaderIterator;

	typedef std::vector< CsPackageChunkHeader > CsPackageChunkHeaderList;
	typedef CsPackageChunkHeaderList::iterator CsPackageChunkHeaderIterator;

	typedef std::vector< CsPackageChunkData > CsPackageChunkDataList;
	typedef CsPackageChunkDataList::iterator CsPackageChunkDataIterator;
	
#if THREADED_IMPORTING 
	typedef std::vector< Json::Value > TJsonValueList;
	typedef TJsonValueList::iterator TJsonValueIterator;
#else
	typedef std::list< Json::Value > TJsonValueList;
	typedef TJsonValueList::iterator TJsonValueIterator;
#endif

	typedef std::vector< BcName > TPackageDependencyList;
	typedef TPackageDependencyList::iterator TPackageDependencyIterator;

	
	TJsonValueList					JsonResources_;

	CsPackageResourceHeader			CurrResourceHeader_;

	mutable std::recursive_mutex	BuildingLock_;
	mutable SysFence				BuildingFence_;
	std::atomic< BcU32 >			BuildingBeginCount_;
	std::atomic< BcU32 >			ImportErrorCount_;
	BcName							Name_;
	BcU32							DataPosition_;
	BcFile							File_;
	CsPackageHeader					Header_;
	TStringList						StringList_;
	TPackageCrossRefList			PackageCrossRefList_;
	TPackageDependencyDataList		PackageDependencyDataList_;
	CsPackageResourceHeaderList		ResourceHeaders_;
	CsPackageChunkHeaderList		ChunkHeaders_;
	CsPackageChunkDataList			ChunkDatas_;
	TPackageDependencyList			PackageDependencyList_;

	// Building.
	CsPackageDependencies			Dependencies_;
};

#endif

#endif
