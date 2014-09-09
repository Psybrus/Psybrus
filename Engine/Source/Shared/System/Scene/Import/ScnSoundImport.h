/**************************************************************************
*
* File:		ScnSoundImport.h
* Author:	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __ScnSoundImport_H__
#define __ScnSoundImport_H__

#ifdef PSY_SERVER
#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"
#include "System/Scene/Sound/ScnSoundFileData.h"

//////////////////////////////////////////////////////////////////////////
// ScnSoundImport
class ScnSoundImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnSoundImport, CsResourceImporter );

public:
	ScnSoundImport();
	ScnSoundImport( ReNoInit );
	virtual ~ScnSoundImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	ScnSoundHeader Header_;

};

#endif
#endif
