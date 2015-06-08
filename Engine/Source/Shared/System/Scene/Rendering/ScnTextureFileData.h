/**************************************************************************
*
* File:		Rendering/ScnTextureFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNTEXTUREFILEDATA__
#define __SCNTEXTUREFILEDATA__

#include "System/Scene/ScnTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureHeader
struct ScnTextureHeader
{
	BcS32 Width_;
	BcS32 Height_;
	BcU32 Depth_;
	BcU32 Levels_;
	RsTextureType Type_;
	RsTextureFormat Format_;
	BcBool Editable_;
	BcBool RenderTarget_;
	BcBool DepthStencilTarget_;
};

//////////////////////////////////////////////////////////////////////////
// ScnTextureAtlasHeader
struct ScnTextureAtlasHeader
{
	BcU32 NoofTextures_;
};

//////////////////////////////////////////////////////////////////////////
// ScnTextureAtlasRect
struct ScnTextureAtlasRect
{
	ScnRect	Rect_;
};

#endif
