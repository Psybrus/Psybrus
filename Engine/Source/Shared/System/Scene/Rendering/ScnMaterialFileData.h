/**************************************************************************
*
* File:		Rendering/ScnMaterialFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNMATERIALFILEDATA__
#define __SCNMATERIALFILEDATA__

#include "Base/BcTypes.h"
#include "System/Renderer/RsSamplerState.h"

//////////////////////////////////////////////////////////////////////////
// ScnMaterialHeader
struct ScnMaterialHeader
{
	BcU32 ShaderRef_;
	BcU32 NoofTextures_;
	BcU32 NoofAutomaticUniformBlocks_;
};

//////////////////////////////////////////////////////////////////////////
// ScnMaterialTextureHeader
struct ScnMaterialTextureHeader
{
	BcName SamplerName_;
	RsSamplerStateDesc SamplerStateDesc_;
	BcU32 TextureRef_;
};

//////////////////////////////////////////////////////////////////////////
// ScnMaterialUniformBlockName
struct ScnMaterialUniformBlockName
{
	BcName Name_;
};

#endif
