/**************************************************************************
*
* File:		ScnComponentImport.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnComponentImport_H__
#define __ScnComponentImport_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

//////////////////////////////////////////////////////////////////////////
// ScnComponentImport
class ScnComponentImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED( ScnComponentImport, CsResourceImporter );

public:
	ScnComponentImport();
	virtual ~ScnComponentImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	
};

#endif
#endif
