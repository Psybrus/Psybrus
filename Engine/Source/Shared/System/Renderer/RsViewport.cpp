/**************************************************************************
*
* File:		RsViewport.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsViewport.h"

//////////////////////////////////////////////////////////////////////////
// Constructor
RsViewport::RsViewport()
{
	Top_ = BcErrorCode;
	Bottom_ = BcErrorCode;
	Left_ = BcErrorCode;
	Right_ = BcErrorCode;
	ZNear_ = 1.0f;
	ZFar_ = 1024.0f;
}
