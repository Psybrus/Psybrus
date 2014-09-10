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
#include "System/Sound/SsSource.h"

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
	std::string Source_;
	BcBool IsStream_;
	SsSourceFileData FileData_;
};

#endif
#endif
