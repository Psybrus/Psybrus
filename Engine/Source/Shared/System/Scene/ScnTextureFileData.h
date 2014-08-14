/**************************************************************************
*
* File:		ScnTextureFileData.h
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
	BcU32							Width_;
	BcU32							Height_;
	BcU32							Depth_;
	BcU32							Levels_;
	RsTextureType					Type_;
	RsTextureFormat				Format_;
	BcBool							Editable_;
};

#endif
