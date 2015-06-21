/**************************************************************************
*
* File:		RsRenderNode.cpp
* Author: 	Neil Richardson 
* Ver/Date:	
* Description:
*		Renderable instance. Used to perform rendering.
*		
*
*
* 
**************************************************************************/

#include "System/Renderer/RsRenderNode.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsRenderNode::RsRenderNode( RsRenderSort InSort, Callback InCallback ):
	Sort_( InSort ),
	Callback_( InCallback )
{
}
