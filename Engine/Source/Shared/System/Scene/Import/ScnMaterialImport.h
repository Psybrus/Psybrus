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

#include "System/Content/CsCore.h"
#include "System/Content/CsResourceImporter.h"
#include "System/Renderer/RsTypes.h"
#include "System/Renderer/RsRenderState.h"

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
	RsRenderStateDesc RenderState_;
};

#endif
