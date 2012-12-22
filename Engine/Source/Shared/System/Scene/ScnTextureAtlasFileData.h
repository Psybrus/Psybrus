/**************************************************************************
*
* File:		ScnTextureAtlasFileData.h
* Author:	Neil Richardson 
* Ver/Date:	22/12/12
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __SCNTEXTUREATLASFILEDATA__
#define __SCNTEXTUREATLASFILEDATA__

#include "Base/BcTypes.h"
#include "System/Scene/ScnTypes.h"

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
