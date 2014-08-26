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

	CsResourceImporterAttribute( 
		const ReClass* ImporterClass = nullptr );

	CsResourceImporterUPtr getImporter() const;

private:
	const ReClass* ImporterClass_;
};

//////////////////////////////////////////////////////////////////////////
// CsResourceImporter
class CsResourceImporter :
	public ReObject
{
public:
	REFLECTION_DECLARE_DERIVED( CsResourceImporter, ReObject );

public:
	CsResourceImporter();
	virtual ~CsResourceImporter();

	/**
	 * Initialise.
	 */
	virtual void initialise( 
		class CsPackageImporter* Importer );

	/**
	 * Import resource.
	 */
	virtual BcBool import( 
		const Json::Value& Object );


protected:
	/**
	 * Add import.
	 * Used to add a subresource if we define a new one.
	 */
	BcU32 addImport( 
		const Json::Value& Resource, 
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
	 * Add dependency.
	 */
	void addDependency( 
		const BcChar* pFileName );

	/**
	 * Get intermediate path.
	 */
	std::string getIntermediatePath();


private:
	class CsPackageImporter* Importer_;

};

#endif // __CSRESOURCEIMPORTER_H__
#include <json/json.h>