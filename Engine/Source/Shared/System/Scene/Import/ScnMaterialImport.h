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
#include "System/Renderer/RsSamplerState.h"

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
	BcBool import( const Json::Value& Object ) override;

	/**
	 * Add texture + sampler.
	 */
	void addTexture( const std::string& Name, CsCrossRefId Texture, RsSamplerStateDesc SamplerState );

private:
	CsCrossRefId Shader_;
	std::map< std::string, CsCrossRefId > Textures_;
	std::map< std::string, RsSamplerStateDesc > Samplers_;
	RsRenderStateDesc RenderState_;
	std::vector< BcName > AutomaticUniformBlocks_;
};

#endif
