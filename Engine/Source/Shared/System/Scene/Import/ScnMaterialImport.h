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
#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnMaterialImport
class ScnMaterialImport:
	public CsResourceImporter
{
public:
	REFLECTION_DECLARE_DERIVED_MANUAL_NOINIT( ScnMaterialImport, CsResourceImporter );

public:
	ScnMaterialImport();
	ScnMaterialImport( ReNoInit );
	virtual ~ScnMaterialImport();

	/**
	 * Import.
	 */
	BcBool import(
		const Json::Value& Object );

private:
	CsCrossRefId Shader_;
	std::map< std::string, CsCrossRefId > Textures_;
	std::map< RsRenderStateType, std::string > State_;

	std::array< BcU32, (BcU32)RsRenderStateType::MAX > DefaultState_;
};

#endif
#endif
