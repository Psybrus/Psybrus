/**************************************************************************
*
* File:		CsResourceImporter.h
* Author:	Neil Richardson
* Ver/Date:	
* Description:
*
*
*
*
*
**************************************************************************/

#ifndef __CSRESOURCEIMPORTER_H__
#define __CSRESOURCEIMPORTER_H__

#include "Base/BcTypes.h"
#include "System/Content/CsTypes.h"
#include "System/Content/CsPackageFileData.h"
#include "Reflection/ReReflection.h"

#include <json/json.h>

#include <memory>

//////////////////////////////////////////////////////////////////////////
// Typedef
struct CsResourceImporterDeleter
{
	void operator() ( class CsResourceImporter* Importer );
};

typedef std::unique_ptr< class CsResourceImporter, CsResourceImporterDeleter > CsResourceImporterUPtr;

//////////////////////////////////////////////////////////////////////////
// CsResourceImporterAttribute
class CsResourceImporterAttribute :
	public ReAttribute
{
public:
	REFLECTION_DECLARE_DERIVED( CsResourceImporterAttribute, ReAttribute );

	CsResourceImporterAttribute();

	CsResourceImporterAttribute( 
		const ReClass* ImporterClass,
		BcU32 VersionId,
		BcU32 Priority = 0 );

	CsResourceImporterUPtr getImporter() const;
	BcU32 getVersionId() const;
	BcU32 getPriority() const;

private:
	const ReClass* ImporterClass_;
	BcU32 VersionId_;
	BcU32 Priority_;
};

//////////////////////////////////////////////////////////////////////////
// CsResourceImporter
class CsResourceImporter
{
public:
	REFLECTION_DECLARE_BASE( CsResourceImporter );

public:
	CsResourceImporter();
	CsResourceImporter(
		const std::string& Name,
		const std::string& Type );
	virtual ~CsResourceImporter();

	/**
	 * Initialise.
	 */
	virtual void initialise( 
		class CsPackageImporter* Importer,
		BcU32 ResourceId );

	/**
	 * Import resource.
	 */
	virtual BcBool import( 
		const Json::Value& Object );

	/**
	 * Get resource name.
	 */
	std::string getResourceName() const;

	/**
	 * Get resource type.
	 */
	std::string getResourceType() const;

	/**
	 * Get resource id.
	 */
	BcU32 getResourceId() const;

	/**
	 * Get importer attribute.
	 */
	const CsResourceImporterAttribute* getImporterAttribute() const;

protected:
	/**
	 * DEPRECATED: Add import.
	 * Used to add a subresource if we define a new one.
	 */
	BcU32 addImport_DEPRECATED( 
		const Json::Value& Resource, 
		BcBool IsCrossRef = BcTrue );
	
	/**
	 * Add import.
	 * Used to add a subresource if we define a new one.
	 */
	BcU32 addImport( 
		CsResourceImporterUPtr Importer, 
		BcBool IsCrossRef = BcTrue );

	/**
	 * Add string.
	 */
	BcU32 addString( 
		const BcChar* pString );

	/**
	 * Add package cross ref.
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
	 * Add file.
	 */
	CsFileHash addFile(
		std::string FileName );
	
	/**
	 * Add dependency.
	 */
	void addDependency( 
		const BcChar* pFileName );

	/**
	 * Get intermediate path.
	 */
	std::string getIntermediatePath();

protected:
	std::string Name_;
	std::string Type_;

private:
	class CsPackageImporter* Importer_;
	BcU32 ResourceId_;
};

#endif // __CSRESOURCEIMPORTER_H__
#include <json/json.h>