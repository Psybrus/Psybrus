/**************************************************************************
*
* File:		ScnPhysicsMeshImport.h
* Author:	Neil Richardson 
* Ver/Date: 
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNPHYSICSMESHIMPORT_H__
#define __SCNPHYSICSMESHIMPORT_H__

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsMeshImport
class ScnPhysicsMeshImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED( ScnPhysicsMeshImport, CsResourceImporter );

public:
	ScnPhysicsMeshImport();
	virtual ~ScnPhysicsMeshImport();

	/**
	 * Import.
	 */
	BcBool import( const Json::Value& );

private:
	std::string Source_;
	
};

#endif
