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

#include "Math/MaMat4d.h"

//////////////////////////////////////////////////////////////////////////
// ScnEntityImport
class ScnEntityImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnEntityImport, CsResourceImporter );

public:
	ScnEntityImport();
	ScnEntityImport( class ScnEntity* Entity );
	ScnEntityImport( ReNoInit );
	virtual ~ScnEntityImport();

	/**
	 * Import.
	 */
	BcBool import() override;

private:
	std::string Entity_;

	MaMat4d LocalTransform_;
	std::vector< class ScnComponent* > Components_;
};

#endif
