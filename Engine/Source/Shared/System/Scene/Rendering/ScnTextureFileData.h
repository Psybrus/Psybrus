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

#include "System/Renderer/RsTypes.h"

//////////////////////////////////////////////////////////////////////////
// ScnTextureHeader
struct ScnTextureHeader
{
	BcS32 Width_;
	BcS32 Height_;
	BcU32 Depth_;
	BcU32 Levels_;
	RsTextureType Type_;
	RsResourceFormat Format_;
	BcBool Editable_;
	RsResourceBindFlags BindFlags_;
};

#endif
