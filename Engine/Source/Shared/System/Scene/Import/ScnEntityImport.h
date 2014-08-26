/**************************************************************************
*
* File:		ScnEntityImport.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnEntityImport_H__
#define __ScnEntityImport_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntityImport
class ScnEntityImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED( ScnEntityImport, CsResourceImporter );

public:
	ScnEntityImport();
	virtual ~ScnEntityImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	
};

#endif
#endif
