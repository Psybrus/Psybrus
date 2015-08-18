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

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntityImport
class ScnEntityImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnEntityImport, CsResourceImporter );

public:
	ScnEntityImport();
	ScnEntityImport( ReNoInit );
	virtual ~ScnEntityImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:

};

#endif
