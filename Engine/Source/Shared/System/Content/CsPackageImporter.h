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
#include "System/Content/CsResourceImporter.h"
#include "System/Content/CsPackageDependencies.h"
#include "System/Content/CsPlatformParams.h"
#include "System/SysFence.h"

#include <json/json.h>

//////////////////////////////////////////////////////////////////////////
// Enable threaded importing.
#define THREADED_IMPORTING ( 0 )

//////////////////////////////////////////////////////////////////////////
// CsPackageImporter
class CsPackageImporter
{
private:
	CsPackageImporter( const CsPackageImporter& ) = delete;

public:
#if PSY_IMPORT_PIPELINE
	CsPackageImporter(
		const CsPlatformParams& Params,
		const BcName& Name,
		const BcPath& Filename );
	~CsPackageImporter();

	/**
	 * Import package.
	 */
	BcBool import();
	
	/**
	 * Save package out to a path.
	 */
	BcBool save( 
		const BcPath& Path );
	
	/**
	 * Load Json file into value.
	 */
	BcBool loadJsonFile( 
		const BcChar* pFileName, 
		Json::Value& Root );
	
	/**
	 * Import resource using importer.
	 * NOTE: Resource passed is is to be deprecated.
	 */
	BcBool importResource( 
		CsResourceImporterUPtr Importer, 
		const Json::Value& Resource );

	/**
	 * Get package name.
	 */
	BcName getName() const;
	
	/**
	 * Begin importing.
	 * Called before any addImport, addString, addChunk etc calls.
	 */
	void beginImport();

	/**
	 * End importing.
	 * Called after all addImport, addString, addChunk etc calls.
	 */
	void endImport();

	/**
	 * DEPRECATED: Add import.
	 */
	BcU32 addImport( 
		const Json::Value& Resource, 
		BcBool IsCrossRef = BcTrue );

	/**
	 * Add import.
	 */
	BcU32 addImport( 
		CsResourceImporterUPtr Importer, 
		const Json::Value& Resource = Json::nullValue, 
		BcBool IsCrossRef = BcTrue );
	
	/**
	 * Add string.
	 */
	BcU32 addString( 
		const BcChar* pString );
	
	/**
	 * Add package cross reference.
	 */
	BcU32 addPackageCrossRef( 
		const BcChar* pFullName );
	
	/**
	 * Add chunk.
	 */
	BcU32 addChunk( 
		BcU32 ID, 
		const void* pData, 
		BcSize Size, 
		BcSize RequiredAlignment = 16, 
		BcU32 Flags = csPCF_DEFAULT );
	
	/**
	 * Add dependency.
	 */
	void addDependency( 
		const BcChar* pFileName );
	
	/**
	 * Add dependency.
	 */
	void addDependency( 
		const ReClass* Class );

	/**
	 * Search through Json value hierarchy
	 * and add all package crossrefs.
	 */
	void addAllPackageCrossRefs( 
		Json::Value& Root );

	/**
	 * Get import params.
	 */
	const CsPlatformParams& getParams() const;


private:
	BcBool havePackageDependency( const BcName& PackageName );

#endif // PSY_IMPORT_PIPELINE

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

	struct TResourceImport
	{
		TResourceImport()
		{
		}

		TResourceImport( TResourceImport&& Other ):
			Importer_( std::move( Other.Importer_ ) ),
			Resource_( std::move( Other.Resource_ ) )
		{
		}

		TResourceImport& operator = ( TResourceImport&& Other )
		{
			Importer_ = std::move( Other.Importer_ );
			Resource_ = std::move( Other.Resource_ );
			return *this;
		}

		bool operator < ( const TResourceImport& Other ) const
		{
			BcAssert( Importer_ );
			BcAssert( Importer_->getImporterAttribute() );
			return Importer_->getImporterAttribute()->getPriority() <
				Other.Importer_->getImporterAttribute()->getPriority();
		}

		CsResourceImporterUPtr Importer_;
		Json::Value Resource_; // Temporary until we get rid of all importer Json deps.
	};
	
	const CsPlatformParams& Params_;
	BcName Name_;
	BcPath Filename_;

	typedef std::vector< TResourceImport > ResourceImportList;
	ResourceImportList Resources_;

	CsPackageResourceHeader CurrResourceHeader_;

	mutable std::recursive_mutex BuildingLock_;
	mutable SysFence BuildingFence_;
	std::atomic< BcU32 > BuildingBeginCount_;
	std::atomic< BcU32 > ImportErrorCount_;
	std::atomic< BcU32 > ResourceIds_;
	BcU32 DataPosition_;
	BcFile File_;
	CsPackageHeader Header_;
	TStringList StringList_;
	TPackageCrossRefList PackageCrossRefList_;
	TPackageDependencyDataList PackageDependencyDataList_;
	CsPackageResourceHeaderList ResourceHeaders_;
	CsPackageChunkHeaderList ChunkHeaders_;
	CsPackageChunkDataList ChunkDatas_;
	TPackageDependencyList PackageDependencyList_;

	// Building.
	CsPackageDependencies Dependencies_;
};

#endif

