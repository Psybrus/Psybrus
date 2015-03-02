/**************************************************************************
*
* File:		ScnPhysicsImport.h
* Author:	Neil Richardson 
* Ver/Date: 25/02/13
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNPHYSICSIMPORT_H__
#define __SCNPHYSICSIMPORT_H__

#ifdef PSY_IMPORT_PIPELINE
#include "System/Content/CsCore.h"

#include "Base/BcStream.h"

//////////////////////////////////////////////////////////////////////////
// ScnPhysicsImport
class ScnPhysicsImport
{
public:
	ScnPhysicsImport();

	/**
	 * Import.
	 */
	BcBool import( class CsPackageImporter& Importer, const Json::Value& Object );

private:

private:
	CsPackageImporter* pImporter_;
};

#endif

#endif
