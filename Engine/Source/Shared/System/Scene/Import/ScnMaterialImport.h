/**************************************************************************
*
* File:		ScnMaterialImport.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMATERIALIMPORT_H__
#define __SCNMATERIALIMPORT_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

//////////////////////////////////////////////////////////////////////////
// ScnMaterialImport
class ScnMaterialImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED( ScnMaterialImport, CsResourceImporter );

public:
	ScnMaterialImport();
	virtual ~ScnMaterialImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	
};

#endif
#endif
