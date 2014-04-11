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

#include "Base/BcProfiler.h"

//////////////////////////////////////////////////////////////////////////
// Ctor
RsRenderNode::RsRenderNode()
{
	Sort_.Value_ = 0;

	PSY_PROFILER_START_ASYNC( BcStringPrintf( "RsRenderNode: 0x%p", this ).c_str() );
}

//////////////////////////////////////////////////////////////////////////
// Dtor
//virtual
RsRenderNode::~RsRenderNode()
{
	PSY_PROFILER_FINISH_ASYNC( BcStringPrintf( "RsRenderNode: 0x%p", this ).c_str()  );
}
