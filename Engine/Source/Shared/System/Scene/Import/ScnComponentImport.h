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

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"

//////////////////////////////////////////////////////////////////////////
// ScnComponentImport
class ScnComponentImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnComponentImport, CsResourceImporter );

public:
	ScnComponentImport();
	ScnComponentImport( class ScnComponent* Component );
	ScnComponentImport( ReNoInit );
	virtual ~ScnComponentImport();

	/**
	 * Import.
	 */
	BcBool import() override;

private:
	class ScnComponent* Component_;
};

#endif
